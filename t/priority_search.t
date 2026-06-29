use strict;
use warnings;
use Test::More tests => 6;

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

# Test 1: Empty Tree Query
{
    my $cmds = <<'EOF';
PSTREE QUERY 1.00|4.00|1.00
EOF
    my $res = run_commands($cmds);
    is_deeply($res, [], "Query on empty tree returns nothing");
}

# Test 2: Add and Query
{
    my $cmds = <<'EOF';
PSTREE ADD 1.00|5.00
PSTREE ADD 2.00|10.00
PSTREE ADD 3.00|2.00
PSTREE ADD 4.00|8.00
PSTREE QUERY 1.50|4.50|4.00
EOF
    my $res = run_commands($cmds);
    is_deeply([sort @$res], ['2.00|10.00', '4.00|8.00'], "3-sided query returns matching points");
}

# Test 3: Query with high y_min (no matches)
{
    my $cmds = <<'EOF';
PSTREE ADD 1.00|5.00
PSTREE ADD 2.00|10.00
PSTREE ADD 3.00|2.00
PSTREE ADD 4.00|8.00
PSTREE QUERY 1.00|4.00|12.00
EOF
    my $res = run_commands($cmds);
    is_deeply($res, [], "3-sided query with high y_min returns nothing");
}

# Test 4: Query all points
{
    my $cmds = <<'EOF';
PSTREE ADD 1.00|5.00
PSTREE ADD 2.00|10.00
PSTREE ADD 3.00|2.00
PSTREE ADD 4.00|8.00
PSTREE QUERY 1.00|4.00|1.00
EOF
    my $res = run_commands($cmds);
    is_deeply([sort @$res], ['1.00|5.00', '2.00|10.00', '3.00|2.00', '4.00|8.00'], "3-sided query covering all returns all points");
}

# Test 5: Remove and Query
{
    my $cmds = <<'EOF';
PSTREE ADD 1.00|5.00
PSTREE ADD 2.00|10.00
PSTREE ADD 3.00|2.00
PSTREE ADD 4.00|8.00
PSTREE REMOVE 2.00|10.00
PSTREE QUERY 1.00|4.00|1.00
EOF
    my $res = run_commands($cmds);
    is_deeply([sort @$res], ['1.00|5.00', '3.00|2.00', '4.00|8.00'], "Removing a point removes it from subsequent queries");
}

# Test 6: Graphviz visualization
{
    my $cmds = <<'EOF';
PSTREE ADD 1.00|5.00
PSTREE GRAPH
EOF
    my $res = run_commands($cmds);
    my $has_digraph = grep { $_ =~ /digraph/ } @$res;
    ok($has_digraph, "PSTREE GRAPH outputs a valid Graphviz digraph");
}
