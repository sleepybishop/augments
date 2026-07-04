use strict;
use warnings;
use Test::More tests => 14;

# Check that test_helper exists
ok(-f 't/util/test_helper', 'test_helper compiled');
ok(-x 't/util/test_helper', 'test_helper is executable');

sub run_commands {
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

# Test 1: Simple Addition and Overlap Search
{
    my $cmds = <<'EOF';
ITREE ADD 1|10|20
ITREE ADD 2|20|30
ITREE FIND 15|25|1
EOF
    my $res = run_commands($cmds);
    is_deeply([sort @$res], ['1|10|20', '2|20|30'], "Find overlapping intervals with [15, 25]")
        or diag explain $res;
}

# Test 2: Containment Queries (overlaps = 0)
{
    my $cmds = <<'EOF';
ITREE ADD 1|10|20
ITREE ADD 2|20|30
ITREE FIND 15|25|0
EOF
    my $res = run_commands($cmds);
    is_deeply($res, [], "Containment query for [15, 25] returns nothing")
        or diag explain $res;
}

# Test 3: Valid Containment Query
{
    my $cmds = <<'EOF';
ITREE ADD 3|5|15
ITREE FIND 6|12|0
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['3|5|15'], "Containment query for [6, 12] inside [5, 15] returns interval 3")
        or diag explain $res;
}

# Test 4: Deletion of specific interval
{
    my $cmds = <<'EOF';
ITREE ADD 4|40|50
ITREE ADD 5|45|55
ITREE REMOVE 40|50|4
ITREE FIND 40|60|1
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['5|45|55'], "Specific interval is successfully removed")
        or diag explain $res;
}

# Test 5: Deletion of all matching intervals (using id = -1 / 18446744073709551615)
{
    my $cmds = <<'EOF';
ITREE ADD 4|40|50
ITREE ADD 5|40|50
ITREE REMOVE 40|50|18446744073709551615
ITREE FIND 40|60|1
EOF
    my $res = run_commands($cmds);
    is_deeply($res, [], "All matching intervals are successfully removed using wildcard ID")
        or diag explain $res;
}

# Test 6: Complex tree structure deletion and search
{
    my $cmds = <<'EOF';
ITREE ADD 20|20|20
ITREE ADD 10|10|10
ITREE ADD 30|30|30
ITREE ADD 25|25|25
ITREE ADD 35|35|35
ITREE ADD 27|27|27
ITREE REMOVE 20|20|20
ITREE REMOVE 25|25|25
ITREE REMOVE 27|27|27
ITREE REMOVE 30|30|30
ITREE FIND 0|100|1
EOF
    my $res = run_commands($cmds);
    is_deeply([sort @$res], ['10|10|10', '35|35|35'], "Complex delete removes matches and leaves [10, 35]")
        or diag explain $res;
}

# Test 7: Duplicate intervals
{
    my $cmds = <<'EOF';
ITREE ADD 1|10|20
ITREE ADD 2|10|20
ITREE FIND 10|20|1
EOF
    my $res = run_commands($cmds);
    is_deeply([sort @$res], ['1|10|20', '2|10|20'], "Duplicate intervals with different IDs are stored")
        or diag explain $res;
}

# Test 8: Multiple duplicates with different low or high
{
    my $cmds = <<'EOF';
ITREE ADD 1|10|20
ITREE ADD 2|10|25
ITREE FIND 10|30|1
EOF
    my $res = run_commands($cmds);
    is_deeply([sort @$res], ['1|10|20', '2|10|25'], "Different intervals with same low but different high are both added")
        or diag explain $res;
}

# Test 9: Empty tree searches
{
    my $cmds = <<'EOF';
ITREE FIND 10|20|1
EOF
    my $res = run_commands($cmds);
    is_deeply($res, [], "Searching empty tree returns nothing")
        or diag explain $res;
}

# Test 10: Destroy empty tree
{
    my $cmds = <<'EOF';
EOF
    my $res = run_commands($cmds);
    is_deeply($res, [], "Destroying empty tree succeeds without error")
        or diag explain $res;
}

# Test 11: Single element tree find and remove
{
    my $cmds = <<'EOF';
ITREE ADD 1|5|5
ITREE FIND 5|5|1
ITREE REMOVE 5|5|1
ITREE FIND 5|5|1
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['1|5|5'], "Single element tree: find, remove, and verify empty")
        or diag explain $res;
}

# Test 12: Graphviz visualization
{
    my $cmds = <<'EOF';
ITREE ADD 1|10|20
ITREE GRAPH
EOF
    my $res = run_commands($cmds);
    my $has_digraph = grep { $_ =~ /digraph/ } @$res;
    ok($has_digraph, "ITREE GRAPH outputs a valid Graphviz digraph");
}
