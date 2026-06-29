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
MINTREE QUERY 10|20
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['INFINITY'], "Query on empty tree returns INFINITY");
}

# Test 2: Add and Query
{
    my $cmds = <<'EOF';
MINTREE ADD 10|5.50
MINTREE ADD 20|1.50
MINTREE ADD 30|3.50
MINTREE QUERY 10|30
MINTREE QUERY 10|15
MINTREE QUERY 25|35
MINTREE QUERY 5|9
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['1.50', '5.50', '3.50', 'INFINITY'], "Query returns correct minimum for various ranges");
}

# Test 3: Update and Query
{
    my $cmds = <<'EOF';
MINTREE ADD 10|5.50
MINTREE ADD 20|1.50
MINTREE ADD 30|3.50
MINTREE UPDATE 20|8.50
MINTREE QUERY 10|30
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['3.50'], "Update propagates and query returns correct new minimum (3.50)");
}

# Test 4: Remove and Query
{
    my $cmds = <<'EOF';
MINTREE ADD 10|5.50
MINTREE ADD 20|8.50
MINTREE ADD 30|3.50
MINTREE REMOVE 30
MINTREE QUERY 10|30
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['5.50'], "Remove propagates and query returns correct new minimum (5.50)");
}

# Test 5: Graphviz visualization
{
    my $cmds = <<'EOF';
MINTREE ADD 10|5.50
MINTREE GRAPH
EOF
    my $res = run_commands($cmds);
    my $has_digraph = grep { $_ =~ /digraph/ } @$res;
    ok($has_digraph, "MINTREE GRAPH outputs a valid Graphviz digraph");
}
