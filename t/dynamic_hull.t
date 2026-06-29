#!/usr/bin/env perl
use strict;
use warnings;
use Test::More tests => 9;
use IPC::Open2;

my $cmd = "valgrind --leak-check=full --error-exitcode=1 ./t/util/test_helper";

sub run_queries {
    my (@queries) = @_;
    my ($chld_out, $chld_in);
    my $pid = open2($chld_out, $chld_in, $cmd);
    
    for my $q (@queries) {
        print $chld_in "$q\n";
    }
    close($chld_in);
    
    my @results;
    while(<$chld_out>) {
        chomp;
        push @results, $_;
    }
    waitpid($pid, 0);
    return \@results;
}

my $res = run_queries(
    "DYNHULL ADD 0 0",
    "DYNHULL ADD 10 0",
    "DYNHULL ADD 5 5",
    "DYNHULL ADD 5 2", # Should be ignored (swallowed)
    "DYNHULL QUERY"    # Dump points
);

is(scalar(@$res), 3, "There should be exactly 3 points on the hull");
is($res->[0], "0.0 0.0", "First point is (0,0)");
is($res->[1], "5.0 5.0", "Second point is (5,5)");
is($res->[2], "10.0 0.0", "Third point is (10,0)");

my $res2 = run_queries(
    "DYNHULL ADD 0 0",
    "DYNHULL ADD 10 0",
    "DYNHULL ADD 5 5",
    "DYNHULL ADD 5 10", # This higher point should swallow the (5,5) point
    "DYNHULL QUERY"
);

is(scalar(@$res2), 3, "Still exactly 3 points on the hull");
is($res2->[1], "5.0 10.0", "(5,5) has been swallowed and replaced by (5,10) conceptually");

my $res3 = run_queries(
    "DYNHULL ADD 0 0",
    "DYNHULL ADD 10 0",
    "DYNHULL ADD 5 5",
    "DYNHULL REMOVE 5 5", # Dynamic delete!
    "DYNHULL ADD 5 2",
    "DYNHULL QUERY"
);

is(scalar(@$res3), 3, "Exactly 3 points on the hull after dynamic deletion");
is($res3->[1], "5.0 2.0", "(5,2) was resurrected after (5,5) was deleted!");
my $swallowed = 1;
for my $line (@$res3) {
    if ($line eq "5.0 5.0") {
        $swallowed = 0;
    }
}
is($swallowed, 1, "(5,5) was successfully deleted dynamically");
