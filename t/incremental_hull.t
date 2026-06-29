#!/usr/bin/env perl
use strict;
use warnings;
use Test::More tests => 7;
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
    "INCHULL ADD 0 0",
    "INCHULL ADD 10 0",
    "INCHULL ADD 5 5",
    "INCHULL ADD 5 2", # Should be ignored (swallowed)
    "INCHULL QUERY"    # Dump points
);

is(scalar(@$res), 3, "There should be exactly 3 points on the hull");
is($res->[0], "0.0 0.0", "First point is (0,0)");
is($res->[1], "5.0 5.0", "Second point is (5,5)");
is($res->[2], "10.0 0.0", "Third point is (10,0)");

my $res2 = run_queries(
    "INCHULL ADD 0 0",
    "INCHULL ADD 10 0",
    "INCHULL ADD 5 5",
    "INCHULL ADD 5 10", # This higher point should destroy the (5,5) point
    "INCHULL QUERY"
);

is(scalar(@$res2), 3, "Still exactly 3 points on the hull");
is($res2->[1], "5.0 10.0", "(5,5) has been replaced by (5,10)");

my $res3 = run_queries(
    "INCHULL ADD 0 0",
    "INCHULL ADD 10 0",
    "INCHULL ADD 5 5",
    "INCHULL ADD 2 8",
    "INCHULL ADD 8 8", # These two new points should swallow (5,5) entirely
    "INCHULL QUERY"
);

my $swallowed = 1;
for my $line (@$res3) {
    if ($line eq "5.0 5.0") {
        $swallowed = 0;
    }
}
is($swallowed, 1, "(5,5) was successfully destroyed by higher arching neighbors");
