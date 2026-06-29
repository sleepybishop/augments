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
SUMTREE QUERY 10|20
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['0.00'], "Query on empty tree returns 0.00");
}

# Test 2: Add and Query
{
    my $cmds = <<'EOF';
SUMTREE ADD 10|1.50
SUMTREE ADD 20|2.50
SUMTREE ADD 30|3.50
SUMTREE QUERY 10|30
SUMTREE QUERY 15|25
SUMTREE QUERY 5|15
SUMTREE QUERY 5|9
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['7.50', '2.50', '1.50', '0.00'], "Query returns correct sum for various ranges");
}

# Test 3: Update and Query
{
    my $cmds = <<'EOF';
SUMTREE ADD 10|1.50
SUMTREE ADD 20|2.50
SUMTREE ADD 30|3.50
SUMTREE UPDATE 20|5.00
SUMTREE QUERY 10|30
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['10.00'], "Update propagates and query returns correct new sum");
}

# Test 4: Remove and Query
{
    my $cmds = <<'EOF';
SUMTREE ADD 10|1.50
SUMTREE ADD 20|2.50
SUMTREE ADD 30|3.50
SUMTREE REMOVE 20
SUMTREE QUERY 10|30
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['5.00'], "Remove propagates and query returns correct new sum");
}

# Test 5: Graphviz visualization
{
    my $cmds = <<'EOF';
SUMTREE ADD 10|1.50
SUMTREE GRAPH
EOF
    my $res = run_commands($cmds);
    my $has_digraph = grep { $_ =~ /digraph/ } @$res;
    ok($has_digraph, "SUMTREE GRAPH outputs a valid Graphviz digraph");
}
