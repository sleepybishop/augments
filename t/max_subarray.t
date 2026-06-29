use strict;
use warnings;
use Test::More tests => 5;

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
MAXSUB QUERY 1|10
EOF
    my $res = run_commands($cmds);
    # Empty query returns -INFINITY (represented as -inf or similar? In C, we returned -INFINITY)
    # Wait, sscanf/printf of -INFINITY prints "-inf" or "-nan". Let's check what it prints.
    # Usually, printf("-inf") on Linux prints "-inf".
    # Wait, in max_subarray.c, sub_tree_query returns -INFINITY.
    # Let's check what C printf("%.2f") does for -INFINITY. It prints "-inf".
    is(lc($res->[0]), '-inf', "Query on empty tree returns -inf");
}

# Test 2: Add and Query Max Subarray Sums
{
    my $cmds = <<'EOF';
MAXSUB ADD 1|2.00
MAXSUB ADD 2|-3.00
MAXSUB ADD 3|4.00
MAXSUB ADD 4|-1.00
MAXSUB ADD 5|2.00
MAXSUB ADD 6|1.00
MAXSUB ADD 7|-5.00
MAXSUB ADD 8|4.00
MAXSUB QUERY 1|8
MAXSUB QUERY 3|6
MAXSUB QUERY 1|2
MAXSUB QUERY 2|2
MAXSUB QUERY 7|8
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['6.00', '6.00', '2.00', '-3.00', '4.00'], "Query returns correct max subarray sums for various ranges");
}

# Test 3: Update and Query
{
    my $cmds = <<'EOF';
MAXSUB ADD 1|2.00
MAXSUB ADD 2|-3.00
MAXSUB ADD 3|4.00
MAXSUB ADD 4|-1.00
MAXSUB ADD 5|2.00
MAXSUB ADD 6|1.00
MAXSUB ADD 7|-5.00
MAXSUB ADD 8|4.00
MAXSUB UPDATE 4|5.00
MAXSUB QUERY 3|6
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['12.00'], "Update propagates and query returns correct new max subarray sum (12.00)");
}

# Test 4: Remove and Query
{
    my $cmds = <<'EOF';
MAXSUB ADD 1|2.00
MAXSUB ADD 2|-3.00
MAXSUB ADD 3|4.00
MAXSUB ADD 4|5.00
MAXSUB ADD 5|2.00
MAXSUB ADD 6|1.00
MAXSUB ADD 7|-5.00
MAXSUB ADD 8|4.00
MAXSUB REMOVE 4
MAXSUB QUERY 3|6
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['7.00'], "Remove propagates and query returns correct new max subarray sum (7.00)");
}

# Test 5: Graphviz visualization
{
    my $cmds = <<'EOF';
MAXSUB ADD 1|2.00
MAXSUB GRAPH
EOF
    my $res = run_commands($cmds);
    my $has_digraph = grep { $_ =~ /digraph/ } @$res;
    ok($has_digraph, "MAXSUB GRAPH outputs a valid Graphviz digraph");
}
