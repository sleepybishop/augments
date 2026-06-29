use strict;
use warnings;
use Test::More tests => 12;
use List::Util qw(min max);

# Helper to run batch commands
sub run_batch {
    my ($cmds) = @_;
    open my $fh, "| valgrind --leak-check=full --error-exitcode=1 ./t/util/test_helper > test_out.txt" or die "Could not run test_helper: $!";
    print $fh $cmds;
    close $fh;
    
    open my $rfh, "<", "test_out.txt" or die "Could not read test_out.txt: $!";
    my @lines = <$rfh>;
    close $rfh;
    unlink "test_out.txt";
    
    chomp @lines;
    return \@lines;
}

sub ccw {
    my ($a, $b, $c) = @_;
    return ($b->[0] - $a->[0]) * ($c->[1] - $a->[1]) - ($b->[1] - $a->[1]) * ($c->[0] - $a->[0]);
}

# Seed random number generator for reproducibility
srand(12345);

# --- 1. ITREE ---
{
    my @intervals;
    my @added_list;
    my $cmds = "";
    my @expected;
    my $find_idx = 0;
    
    for my $i (1 .. 100) {
        my $low = int(rand(1000));
        my $high = $low + int(rand(100)) + 1;
        my $id = $i;
        push @intervals, { id => $id, low => $low, high => $high };
        push @added_list, { id => $id, low => $low, high => $high };
        $cmds .= "ITREE ADD $id|$low|$high\n";
    }
    
    for my $step (1 .. 100) {
        my $op = rand();
        my $low = int(rand(1000));
        my $high = $low + int(rand(100));
        my $overlaps = (rand() < 0.8) ? 1 : 0;
        
        if ($op < 0.5) {
            my @res;
            for my $int (@intervals) {
                my $match = 0;
                if ($overlaps > 0) {
                    if ($low <= $int->{high} && $high >= $int->{low}) { $match = 1; }
                } else {
                    if ($low >= $int->{low} && $high <= $int->{high}) { $match = 1; }
                }
                if ($match) { push @res, "$int->{id}|$int->{low}|$int->{high}"; }
            }
            my $marker = "ITREE_MARKER_$find_idx";
            $cmds .= "ECHO $marker\nITREE FIND $low|$high|$overlaps\n";
            push @expected, { marker => $marker, ans => [sort @res] };
            $find_idx++;
        } else {
            if (@added_list) {
                my $idx = int(rand(@added_list));
                my $int = splice(@added_list, $idx, 1);
                @intervals = grep { !($_->{low} == $int->{low} && $_->{high} == $int->{high} && $_->{id} == $int->{id}) } @intervals;
                $cmds .= "ITREE REMOVE $int->{low}|$int->{high}|$int->{id}\n";
            }
        }
    }
    
    my $output = run_batch($cmds);
    my %results;
    my $curr = undef;
    for my $line (@$output) {
        if ($line =~ /^ITREE_MARKER_/) { $curr = $line; $results{$curr} = []; }
        elsif (defined $curr) { push @{$results{$curr}}, $line; }
    }
    my $ok = 1;
    for my $exp (@expected) {
        my $got = [ sort @{$results{$exp->{marker}} // []} ];
        my $exp_sorted = [ sort @{$exp->{ans}} ];
        if (join(",", @$got) ne join(",", @$exp_sorted)) { $ok = 0; }
    }
    ok($ok, "ITREE fuzzing matches oracle");
}

# --- 2. OSTREE ---
{
    my @oracle_arr;
    my $cmds = "";
    my @expected;
    my $find_idx = 0;
    for my $i (1 .. 150) {
        my $op = rand();
        if ($op < 0.5) {
            my $key = int(rand(1000)) + 1;
            if (!grep { $_ == $key } @oracle_arr) {
                push @oracle_arr, $key;
                @oracle_arr = sort { $a <=> $b } @oracle_arr;
                $cmds .= "OSTREE ADD $key\n";
            }
        } elsif ($op < 0.7) {
            if (@oracle_arr) {
                my $idx = int(rand(@oracle_arr));
                my $key = splice(@oracle_arr, $idx, 1);
                $cmds .= "OSTREE REMOVE $key\n";
            }
        } else {
            my $marker = "OSTREE_MARKER_$find_idx";
            $cmds .= "ECHO $marker\n";
            if (rand() < 0.5) {
                my $rank = int(rand(max(1, scalar(@oracle_arr) + 5)));
                $cmds .= "OSTREE SELECT $rank\n";
                my $ans = ($rank < @oracle_arr) ? $oracle_arr[$rank] : "NULL";
                push @expected, { marker => $marker, ans => [$ans] };
            } else {
                my $key = (rand() < 0.5 && @oracle_arr) ? $oracle_arr[int(rand(@oracle_arr))] : int(rand(1000)) + 1;
                $cmds .= "OSTREE RANK $key\n";
                my ($idx) = grep { $oracle_arr[$_] == $key } 0 .. $#oracle_arr;
                my $ans = defined($idx) ? $idx : "NOT_FOUND";
                push @expected, { marker => $marker, ans => [$ans] };
            }
            $find_idx++;
        }
    }
    my $output = run_batch($cmds);
    my %results;
    my $curr = undef;
    for my $line (@$output) {
        if ($line =~ /^OSTREE_MARKER_/) { $curr = $line; $results{$curr} = []; }
        elsif (defined $curr) { push @{$results{$curr}}, $line; }
    }
    my $ok = 1;
    for my $exp (@expected) {
        my $got = [ sort @{$results{$exp->{marker}} // []} ];
        my $exp_sorted = [ sort @{$exp->{ans}} ];
        if (join(",", @$got) ne join(",", @$exp_sorted)) { $ok = 0; }
    }
    ok($ok, "OSTREE fuzzing matches oracle");
}

# --- 3. SUMTREE ---
{
    my %oracle_sum;
    my $cmds = "";
    my @expected;
    my $find_idx = 0;
    for my $i (1 .. 150) {
        my $op = rand();
        if ($op < 0.4) {
            my $key = int(rand(1000)) + 1;
            my $val = sprintf("%.2f", rand(100) - 50);
            if (!exists $oracle_sum{$key}) {
                $oracle_sum{$key} = $val;
                $cmds .= "SUMTREE ADD $key|$val\n";
            }
        } elsif ($op < 0.6) {
            if (keys %oracle_sum) {
                my $key = (keys %oracle_sum)[rand(keys %oracle_sum)];
                my $val = sprintf("%.2f", rand(100) - 50);
                $oracle_sum{$key} = $val;
                $cmds .= "SUMTREE UPDATE $key|$val\n";
            }
        } elsif ($op < 0.8) {
            if (keys %oracle_sum) {
                my $key = (keys %oracle_sum)[rand(keys %oracle_sum)];
                delete $oracle_sum{$key};
                $cmds .= "SUMTREE REMOVE $key\n";
            }
        } else {
            my $low = int(rand(500)) + 1;
            my $high = $low + int(rand(500));
            my $marker = "SUMTREE_MARKER_$find_idx";
            $cmds .= "ECHO $marker\nSUMTREE QUERY $low|$high\n";
            my $sum = 0;
            for my $k (keys %oracle_sum) {
                if ($k >= $low && $k <= $high) { $sum += $oracle_sum{$k}; }
            }
            push @expected, { marker => $marker, ans => [sprintf("%.2f", $sum)] };
            $find_idx++;
        }
    }
    my $output = run_batch($cmds);
    my %results;
    my $curr = undef;
    for my $line (@$output) {
        if ($line =~ /^SUMTREE_MARKER_/) { $curr = $line; $results{$curr} = []; }
        elsif (defined $curr) { push @{$results{$curr}}, $line; }
    }
    my $ok = 1;
    for my $exp (@expected) {
        my $got = [ sort @{$results{$exp->{marker}} // []} ];
        my $exp_sorted = [ sort @{$exp->{ans}} ];
        if (join(",", @$got) ne join(",", @$exp_sorted)) { $ok = 0; }
    }
    ok($ok, "SUMTREE fuzzing matches oracle");
}

# --- 4. MINTREE ---
{
    my %oracle_min;
    my $cmds = "";
    my @expected;
    my $find_idx = 0;
    for my $i (1 .. 150) {
        my $op = rand();
        if ($op < 0.4) {
            my $key = int(rand(1000)) + 1;
            my $val = sprintf("%.2f", rand(100) - 50);
            if (!exists $oracle_min{$key}) {
                $oracle_min{$key} = $val;
                $cmds .= "MINTREE ADD $key|$val\n";
            }
        } elsif ($op < 0.6) {
            if (keys %oracle_min) {
                my $key = (keys %oracle_min)[rand(keys %oracle_min)];
                my $val = sprintf("%.2f", rand(100) - 50);
                $oracle_min{$key} = $val;
                $cmds .= "MINTREE UPDATE $key|$val\n";
            }
        } elsif ($op < 0.8) {
            if (keys %oracle_min) {
                my $key = (keys %oracle_min)[rand(keys %oracle_min)];
                delete $oracle_min{$key};
                $cmds .= "MINTREE REMOVE $key\n";
            }
        } else {
            my $low = int(rand(500)) + 1;
            my $high = $low + int(rand(500));
            my $marker = "MINTREE_MARKER_$find_idx";
            $cmds .= "ECHO $marker\nMINTREE QUERY $low|$high\n";
            my @vals;
            for my $k (keys %oracle_min) {
                if ($k >= $low && $k <= $high) { push @vals, $oracle_min{$k}; }
            }
            my $ans = @vals ? sprintf("%.2f", min(@vals)) : "INFINITY";
            push @expected, { marker => $marker, ans => [$ans] };
            $find_idx++;
        }
    }
    my $output = run_batch($cmds);
    my %results;
    my $curr = undef;
    for my $line (@$output) {
        if ($line =~ /^MINTREE_MARKER_/) { $curr = $line; $results{$curr} = []; }
        elsif (defined $curr) { push @{$results{$curr}}, $line; }
    }
    my $ok = 1;
    for my $exp (@expected) {
        my $got = [ sort @{$results{$exp->{marker}} // []} ];
        my $exp_sorted = [ sort @{$exp->{ans}} ];
        if (join(",", @$got) ne join(",", @$exp_sorted)) { $ok = 0; }
    }
    ok($ok, "MINTREE fuzzing matches oracle");
}

# --- 5. PSTREE ---
{
    my %oracle_ps;
    my $cmds = "";
    my @expected;
    my $find_idx = 0;
    for my $i (1 .. 150) {
        my $op = rand();
        if ($op < 0.5) {
            my $x = int(rand(100));
            my $y = int(rand(100));
            if (!exists $oracle_ps{$x}) {
                $oracle_ps{$x} = $y;
                $cmds .= "PSTREE ADD $x|$y\n";
            }
        } elsif ($op < 0.7) {
            if (keys %oracle_ps) {
                my $x = (keys %oracle_ps)[rand(keys %oracle_ps)];
                my $y = $oracle_ps{$x};
                delete $oracle_ps{$x};
                $cmds .= "PSTREE REMOVE $x|$y\n";
            }
        } else {
            my $x_min = int(rand(50));
            my $x_max = $x_min + int(rand(50));
            my $y_min = int(rand(50));
            my $marker = "PSTREE_MARKER_$find_idx";
            $cmds .= "ECHO $marker\nPSTREE QUERY $x_min|$x_max|$y_min\n";
            my @ans;
            for my $x (sort { $a <=> $b } keys %oracle_ps) {
                if ($x >= $x_min && $x <= $x_max && $oracle_ps{$x} >= $y_min) {
                    push @ans, sprintf("%.2f|%.2f", $x, $oracle_ps{$x});
                }
            }
            push @expected, { marker => $marker, ans => \@ans };
            $find_idx++;
        }
    }
    my $output = run_batch($cmds);
    my %results;
    my $curr = undef;
    for my $line (@$output) {
        if ($line =~ /^PSTREE_MARKER_/) { $curr = $line; $results{$curr} = []; }
        elsif (defined $curr) { push @{$results{$curr}}, $line; }
    }
    my $ok = 1;
    for my $exp (@expected) {
        my $got = [ sort @{$results{$exp->{marker}} // []} ];
        my $exp_sorted = [ sort @{$exp->{ans}} ];
        if (join(",", @$got) ne join(",", @$exp_sorted)) {
            $ok = 0;
            diag "PSTREE mismatch at $exp->{marker}:";
            diag "  Got: " . join(", ", @$got);
            diag "  Exp: " . join(", ", @$exp_sorted);
        }
    }
    ok($ok, "PSTREE fuzzing matches oracle");
}

# --- 6. MAXSUB ---
{
    my %oracle_sub;
    my $cmds = "";
    my @expected;
    my $find_idx = 0;
    for my $i (1 .. 150) {
        my $op = rand();
        if ($op < 0.4) {
            my $key = int(rand(1000)) + 1;
            my $val = int(rand(100)) - 50;
            if (!exists $oracle_sub{$key}) {
                $oracle_sub{$key} = $val;
                $cmds .= "MAXSUB ADD $key|$val\n";
            }
        } elsif ($op < 0.6) {
            if (keys %oracle_sub) {
                my $key = (keys %oracle_sub)[rand(keys %oracle_sub)];
                my $val = int(rand(100)) - 50;
                $oracle_sub{$key} = $val;
                $cmds .= "MAXSUB UPDATE $key|$val\n";
            }
        } elsif ($op < 0.8) {
            if (keys %oracle_sub) {
                my $key = (keys %oracle_sub)[rand(keys %oracle_sub)];
                delete $oracle_sub{$key};
                $cmds .= "MAXSUB REMOVE $key\n";
            }
        } else {
            my $low = int(rand(500)) + 1;
            my $high = $low + int(rand(500));
            my $marker = "MAXSUB_MARKER_$find_idx";
            $cmds .= "ECHO $marker\nMAXSUB QUERY $low|$high\n";
            my @elements;
            for my $k (sort { $a <=> $b } keys %oracle_sub) {
                if ($k >= $low && $k <= $high) { push @elements, $oracle_sub{$k}; }
            }
            my $ans;
            if (!@elements) {
                $ans = "-inf";
            } else {
                my $max_so_far = -999999999;
                my $curr_max = -999999999;
                for my $x (@elements) {
                    $curr_max = ($curr_max < 0) ? $x : $curr_max + $x;
                    $max_so_far = max($max_so_far, $curr_max);
                }
                $ans = sprintf("%.2f", $max_so_far);
            }
            push @expected, { marker => $marker, ans => [$ans] };
            $find_idx++;
        }
    }
    my $output = run_batch($cmds);
    my %results;
    my $curr = undef;
    for my $line (@$output) {
        if ($line =~ /^MAXSUB_MARKER_/) { $curr = $line; $results{$curr} = []; }
        elsif (defined $curr) { push @{$results{$curr}}, $line; }
    }
    my $ok = 1;
    for my $exp (@expected) {
        my $got = [ sort @{$results{$exp->{marker}} // []} ];
        my $exp_sorted = [ sort @{$exp->{ans}} ];
        if (join(",", @$got) ne join(",", @$exp_sorted)) {
            $ok = 0;
            diag "MAXSUB mismatch at $exp->{marker}:";
            diag "  Got: " . join(", ", @$got);
            diag "  Exp: " . join(", ", @$exp_sorted);
        }
    }
    ok($ok, "MAXSUB fuzzing matches oracle");
}

# --- 7. HASHTREE ---
{
    my %oracle_ht;
    my $cmds = "";
    my @expected;
    my $find_idx = 0;
    for my $i (1 .. 150) {
        my $op = rand();
        if ($op < 0.6) {
            my $key = int(rand(1000)) + 1;
            my $val = (qw(a b c d e f g h i j))[int(rand(10))];
            if (!exists $oracle_ht{$key}) {
                $oracle_ht{$key} = $val;
                $cmds .= "HASHTREE ADD $key|$val\n";
            }
        } elsif ($op < 0.8) {
            if (keys %oracle_ht) {
                my $key = (keys %oracle_ht)[rand(keys %oracle_ht)];
                delete $oracle_ht{$key};
                $cmds .= "HASHTREE REMOVE $key\n";
            }
        } else {
            my $marker = "HASHTREE_MARKER_$find_idx";
            $cmds .= "ECHO $marker\nHASHTREE HASH\n";
            my $h = 0;
            for my $k (sort { $a <=> $b } keys %oracle_ht) {
                $h = ($h * 313 + ord($oracle_ht{$k})) % 1000000007;
            }
            push @expected, { marker => $marker, ans => [$h] };
            $find_idx++;
        }
    }
    my $output = run_batch($cmds);
    my %results;
    my $curr = undef;
    for my $line (@$output) {
        if ($line =~ /^HASHTREE_MARKER_/) { $curr = $line; $results{$curr} = []; }
        elsif (defined $curr) { push @{$results{$curr}}, $line; }
    }
    my $ok = 1;
    for my $exp (@expected) {
        my $got = [ sort @{$results{$exp->{marker}} // []} ];
        my $exp_sorted = [ sort @{$exp->{ans}} ];
        if (join(",", @$got) ne join(",", @$exp_sorted)) { $ok = 0; }
    }
    ok($ok, "HASHTREE fuzzing matches oracle");
}

# --- 8. LCP ---
{
    my %oracle_lcp;
    my $cmds = "";
    my @expected;
    my $find_idx = 0;
    my @words = qw(apple application banana band bandit cat dog elephant fox giraffe hello help world word);
    for my $i (1 .. 150) {
        my $op = rand();
        if ($op < 0.6) {
            my $word = $words[int(rand(@words))] . int(rand(100));
            if (!exists $oracle_lcp{$word}) {
                $oracle_lcp{$word} = 1;
                $cmds .= "LCP ADD $word\n";
            }
        } elsif ($op < 0.8) {
            if (keys %oracle_lcp) {
                my $word = (keys %oracle_lcp)[rand(keys %oracle_lcp)];
                delete $oracle_lcp{$word};
                $cmds .= "LCP REMOVE $word\n";
            }
        } else {
            my $marker = "LCP_MARKER_$find_idx";
            $cmds .= "ECHO $marker\nLCP QUERY\n";
            my $ans = 0;
            if (keys %oracle_lcp) {
                my @sorted = sort keys %oracle_lcp;
                my $min_s = $sorted[0];
                my $max_s = $sorted[-1];
                my $idx = 0;
                while ($idx < length($min_s) && $idx < length($max_s) && substr($min_s, $idx, 1) eq substr($max_s, $idx, 1)) {
                    $idx++;
                }
                $ans = $idx;
            }
            push @expected, { marker => $marker, ans => [$ans] };
            $find_idx++;
        }
    }
    my $output = run_batch($cmds);
    my %results;
    my $curr = undef;
    for my $line (@$output) {
        if ($line =~ /^LCP_MARKER_/) { $curr = $line; $results{$curr} = []; }
        elsif (defined $curr) { push @{$results{$curr}}, $line; }
    }
    my $ok = 1;
    for my $exp (@expected) {
        my $got = [ sort @{$results{$exp->{marker}} // []} ];
        my $exp_sorted = [ sort @{$exp->{ans}} ];
        if (join(",", @$got) ne join(",", @$exp_sorted)) { $ok = 0; }
    }
    ok($ok, "LCP fuzzing matches oracle");
}

# --- 9. ROPE ---
{
    my $oracle_rope = "";
    my $cmds = "";
    my @expected;
    my $find_idx = 0;
    for my $i (1 .. 150) {
        my $op = rand();
        my $len = length($oracle_rope);
        if ($op < 0.5) {
            my $idx = int(rand($len + 1));
            my $val = (qw(a b c d e f g))[int(rand(7))];
            substr($oracle_rope, $idx, 0, $val);
            $cmds .= "ROPE ADD $idx|$val\n";
        } elsif ($op < 0.7) {
            if ($len > 0) {
                my $idx = int(rand($len));
                substr($oracle_rope, $idx, 1, "");
                $cmds .= "ROPE REMOVE $idx\n";
            }
        } else {
            if ($len > 0) {
                my $idx = int(rand($len));
                my $marker = "ROPE_MARKER_$find_idx";
                $cmds .= "ECHO $marker\nROPE QUERY $idx\n";
                push @expected, { marker => $marker, ans => [substr($oracle_rope, $idx, 1)] };
                $find_idx++;
            }
        }
    }
    my $output = run_batch($cmds);
    my %results;
    my $curr = undef;
    for my $line (@$output) {
        if ($line =~ /^ROPE_MARKER_/) { $curr = $line; $results{$curr} = []; }
        elsif (defined $curr) { push @{$results{$curr}}, $line; }
    }
    my $ok = 1;
    for my $exp (@expected) {
        my $got = [ sort @{$results{$exp->{marker}} // []} ];
        my $exp_sorted = [ sort @{$exp->{ans}} ];
        if (join(",", @$got) ne join(",", @$exp_sorted)) { $ok = 0; }
    }
    ok($ok, "ROPE fuzzing matches oracle");
}

# --- 10. EULER ---
{
    my %adj;
    my $cmds = "";
    my @expected;
    my $find_idx = 0;
    for my $i (1 .. 150) {
        my $op = rand();
        if ($op < 0.5) {
            my $u = int(rand(100));
            my $v = int(rand(100));
            if ($u != $v) {
                my $connected = 0;
                my %visited;
                my @queue = ($u);
                $visited{$u} = 1;
                while (@queue) {
                    my $curr = shift @queue;
                    if ($curr == $v) { $connected = 1; last; }
                    for my $neighbor (@{$adj{$curr} // []}) {
                        if (!$visited{$neighbor}) {
                            $visited{$neighbor} = 1;
                            push @queue, $neighbor;
                        }
                    }
                }
                if (!$connected) {
                    push @{$adj{$u}}, $v;
                    push @{$adj{$v}}, $u;
                    $cmds .= "EULER LINK $u $v\n";
                }
            }
        } elsif ($op < 0.7) {
            my @edges;
            for my $u (keys %adj) {
                for my $v (@{$adj{$u}}) {
                    if ($u < $v) { push @edges, [$u, $v]; }
                }
            }
            if (@edges) {
                my $edge = $edges[int(rand(@edges))];
                my ($u, $v) = @$edge;
                @{$adj{$u}} = grep { $_ != $v } @{$adj{$u}};
                @{$adj{$v}} = grep { $_ != $u } @{$adj{$v}};
                $cmds .= "EULER CUT $u $v\n";
            }
        } else {
            my $u = int(rand(100));
            my $v = int(rand(100));
            my $marker = "EULER_MARKER_$find_idx";
            $cmds .= "ECHO $marker\nEULER CONNECTED $u $v\n";
            my $connected = 0;
            if ($u == $v) {
                $connected = 1;
            } else {
                my %visited;
                my @queue = ($u);
                $visited{$u} = 1;
                while (@queue) {
                    my $curr = shift @queue;
                    if ($curr == $v) { $connected = 1; last; }
                    for my $neighbor (@{$adj{$curr} // []}) {
                        if (!$visited{$neighbor}) {
                            $visited{$neighbor} = 1;
                            push @queue, $neighbor;
                        }
                    }
                }
            }
            push @expected, { marker => $marker, ans => [$connected] };
            $find_idx++;
        }
    }
    my $output = run_batch($cmds);
    my %results;
    my $curr = undef;
    for my $line (@$output) {
        if ($line =~ /^EULER_MARKER_/) { $curr = $line; $results{$curr} = []; }
        elsif (defined $curr) { push @{$results{$curr}}, $line; }
    }
    my $ok = 1;
    for my $exp (@expected) {
        my $got = [ sort @{$results{$exp->{marker}} // []} ];
        my $exp_sorted = [ sort @{$exp->{ans}} ];
        if (join(",", @$got) ne join(",", @$exp_sorted)) { $ok = 0; }
    }
    ok($ok, "EULER fuzzing matches oracle");
}

# --- 11. INCHULL ---
{
    my @pts;
    my $cmds = "";
    my @expected;
    my $find_idx = 0;
    for my $i (1 .. 150) {
        my $op = rand();
        if ($op < 0.8) {
            my $x = int(rand(100));
            my $y = int(rand(100));
            push @pts, [$x, $y];
            $cmds .= "INCHULL ADD $x $y\n";
        } else {
            my $marker = "INCHULL_MARKER_$find_idx";
            $cmds .= "ECHO $marker\nINCHULL QUERY\n";
            my %max_y;
            for my $p (@pts) {
                if (!exists $max_y{$p->[0]} || $p->[1] > $max_y{$p->[0]}) { $max_y{$p->[0]} = $p->[1]; }
            }
            my @sorted;
            for my $x (sort { $a <=> $b } keys %max_y) { push @sorted, [$x, $max_y{$x}]; }
            my @upper;
            for my $p (@sorted) {
                while (@upper >= 2) {
                    if (ccw($upper[-2], $upper[-1], $p) >= 0) { pop @upper; }
                    else { last; }
                }
                push @upper, $p;
            }
            my @ans;
            for my $p (@upper) { push @ans, sprintf("%.1f %.1f", $p->[0], $p->[1]); }
            push @expected, { marker => $marker, ans => \@ans };
            $find_idx++;
        }
    }
    my $output = run_batch($cmds);
    my %results;
    my $curr = undef;
    for my $line (@$output) {
        if ($line =~ /^INCHULL_MARKER_/) { $curr = $line; $results{$curr} = []; }
        elsif (defined $curr) { push @{$results{$curr}}, $line; }
    }
    my $ok = 1;
    for my $exp (@expected) {
        my $got = [ sort @{$results{$exp->{marker}} // []} ];
        my $exp_sorted = [ sort @{$exp->{ans}} ];
        if (join(",", @$got) ne join(",", @$exp_sorted)) {
            $ok = 0;
            diag "INCHULL mismatch at $exp->{marker}:";
            diag "  Got: " . join(", ", @$got);
            diag "  Exp: " . join(", ", @$exp_sorted);
        }
    }
    ok($ok, "INCHULL fuzzing matches oracle");
}

# --- 12. DYNHULL ---
{
    my %pts_hash;
    my $cmds = "";
    my @expected;
    my $find_idx = 0;
    for my $i (1 .. 150) {
        my $op = rand();
        if ($op < 0.5) {
            my $x = int(rand(100));
            my $y = int(rand(100));
            my $attempts = 0;
            while ($attempts < 100 && grep { $_->[0] == $x } values %pts_hash) {
                $x = int(rand(100));
                $attempts++;
            }
            if ($attempts < 100 && !exists $pts_hash{"$x,$y"}) {
                $pts_hash{"$x,$y"} = [$x, $y];
                $cmds .= "DYNHULL ADD $x $y\n";
            }
        } elsif ($op < 0.7) {
            if (keys %pts_hash) {
                my $k = (keys %pts_hash)[rand(keys %pts_hash)];
                my $p = $pts_hash{$k};
                delete $pts_hash{$k};
                $cmds .= "DYNHULL REMOVE $p->[0] $p->[1]\n";
            }
        } else {
            my $marker = "DYNHULL_MARKER_$find_idx";
            $cmds .= "ECHO $marker\nDYNHULL QUERY\n";
            my %max_y;
            for my $p (values %pts_hash) {
                if (!exists $max_y{$p->[0]} || $p->[1] > $max_y{$p->[0]}) { $max_y{$p->[0]} = $p->[1]; }
            }
            my @sorted;
            for my $x (sort { $a <=> $b } keys %max_y) { push @sorted, [$x, $max_y{$x}]; }
            my @upper;
            for my $p (@sorted) {
                while (@upper >= 2) {
                    if (ccw($upper[-2], $upper[-1], $p) >= 0) { pop @upper; }
                    else { last; }
                }
                push @upper, $p;
            }
            my @ans;
            for my $p (@upper) { push @ans, sprintf("%.1f %.1f", $p->[0], $p->[1]); }
            push @expected, { marker => $marker, ans => \@ans };
            $find_idx++;
        }
    }
    my $output = run_batch($cmds);
    my %results;
    my $curr = undef;
    for my $line (@$output) {
        if ($line =~ /^DYNHULL_MARKER_/) { $curr = $line; $results{$curr} = []; }
        elsif (defined $curr) { push @{$results{$curr}}, $line; }
    }
    my $ok = 1;
    for my $exp (@expected) {
        my $got = [ sort @{$results{$exp->{marker}} // []} ];
        my $exp_sorted = [ sort @{$exp->{ans}} ];
        if (join(",", @$got) ne join(",", @$exp_sorted)) {
            $ok = 0;
            diag "DYNHULL mismatch at $exp->{marker}:";
            diag "  Got: " . join(", ", @$got);
            diag "  Exp: " . join(", ", @$exp_sorted);
        }
    }
    ok($ok, "DYNHULL fuzzing matches oracle");
}
