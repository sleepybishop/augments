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
    "EULER LINK 0 1",
    "EULER LINK 1 2",
    "EULER LINK 3 4",
    "EULER CONNECTED 0 2", # Should be 1
    "EULER CONNECTED 0 3", # Should be 0
    "EULER LINK 2 4",
    "EULER CONNECTED 0 3", # Should now be 1
    "EULER CUT 1 2",
    "EULER CONNECTED 0 4", # Should be 0
    "EULER CONNECTED 3 4", # Should be 1
    "EULER CONNECTED 0 1"  # Should be 1
);

is($res->[0], "1", "0 and 2 are connected");
is($res->[1], "0", "0 and 3 are disconnected");
is($res->[2], "1", "0 and 3 are connected after linking 2 and 4");
is($res->[3], "0", "0 and 4 disconnected after cutting 1-2");
is($res->[4], "1", "3 and 4 remain connected");
is($res->[5], "1", "0 and 1 remain connected");
is(scalar(@$res), 6, "Correct number of outputs");
