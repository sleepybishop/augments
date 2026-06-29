#!/usr/bin/env perl
use strict;
use warnings;
use Test::More tests => 4;
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

my $res1 = run_queries(
    "LCP ADD apple",
    "LCP ADD application",
    "LCP QUERY"
);
is($res1->[0], 4, "LCP of 'apple' and 'application' is 4 ('appl')");

my $res2 = run_queries(
    "LCP ADD apple",
    "LCP ADD application",
    "LCP ADD app",
    "LCP QUERY"
);
is($res2->[0], 3, "LCP drops to 3 ('app') when inserting 'app'");

my $res3 = run_queries(
    "LCP ADD apple",
    "LCP ADD application",
    "LCP ADD app",
    "LCP ADD banana",
    "LCP QUERY"
);
is($res3->[0], 0, "LCP drops to 0 when inserting 'banana'");

my $res4 = run_queries(
    "LCP ADD apple",
    "LCP ADD application",
    "LCP ADD app",
    "LCP ADD banana",
    "LCP REMOVE banana",
    "LCP QUERY"
);
is($res4->[0], 3, "LCP reverts to 3 after removing 'banana'");
