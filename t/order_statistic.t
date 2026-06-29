use strict;
use warnings;
use Test::More tests => 7;

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

# Test 1: Empty Tree Select
{
    my $cmds = <<'EOF';
OSTREE SELECT 0
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['NULL'], "Select on empty tree returns NULL");
}

# Test 2: Rank on Empty Tree
{
    my $cmds = <<'EOF';
OSTREE RANK 10
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['NOT_FOUND'], "Rank on empty tree returns NOT_FOUND");
}

# Test 3: Insert and Select
{
    my $cmds = <<'EOF';
OSTREE ADD 30
OSTREE ADD 10
OSTREE ADD 50
OSTREE ADD 20
OSTREE ADD 40
OSTREE SELECT 0
OSTREE SELECT 1
OSTREE SELECT 2
OSTREE SELECT 3
OSTREE SELECT 4
OSTREE SELECT 5
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['10', '20', '30', '40', '50', 'NULL'], "Select returns elements in sorted order");
}

# Test 4: Rank of existing keys
{
    my $cmds = <<'EOF';
OSTREE ADD 30
OSTREE ADD 10
OSTREE ADD 50
OSTREE ADD 20
OSTREE ADD 40
OSTREE RANK 10
OSTREE RANK 20
OSTREE RANK 30
OSTREE RANK 40
OSTREE RANK 50
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['0', '1', '2', '3', '4'], "Rank returns correct 0-based index for all keys");
}

# Test 5: Rank of non-existing key
{
    my $cmds = <<'EOF';
OSTREE ADD 30
OSTREE ADD 10
OSTREE ADD 50
OSTREE RANK 25
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['NOT_FOUND'], "Rank of non-existing key returns NOT_FOUND");
}

# Test 6: Remove and verify Rank and Select updates
{
    my $cmds = <<'EOF';
OSTREE ADD 30
OSTREE ADD 10
OSTREE ADD 50
OSTREE ADD 20
OSTREE ADD 40
OSTREE REMOVE 30
OSTREE SELECT 2
OSTREE RANK 40
OSTREE RANK 50
EOF
    my $res = run_commands($cmds);
    is_deeply($res, ['40', '2', '3'], "After removing 30, rank and select are correctly updated");
}

# Test 7: Graphviz visualization
{
    my $cmds = <<'EOF';
OSTREE ADD 10
OSTREE GRAPH
EOF
    my $res = run_commands($cmds);
    my $has_digraph = grep { $_ =~ /digraph/ } @$res;
    ok($has_digraph, "OSTREE GRAPH outputs a valid Graphviz digraph");
}
