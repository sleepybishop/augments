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
    "ROPE ADD 0|a",
    "ROPE ADD 1|b",
    "ROPE ADD 2|d",
    "ROPE ADD 2|c",  # Insert 'c' at index 2, implicitly shifting 'd' to index 3
    "ROPE QUERY 0",
    "ROPE QUERY 1",
    "ROPE QUERY 2",
    "ROPE QUERY 3"
);

is($res->[0], "a", "Index 0 is 'a'");
is($res->[1], "b", "Index 1 is 'b'");
is($res->[2], "c", "Index 2 is 'c'");
is($res->[3], "d", "Index 3 is 'd'");

my $res2 = run_queries(
    "ROPE ADD 0|a",
    "ROPE ADD 1|b",
    "ROPE ADD 2|c",
    "ROPE ADD 3|d",
    "ROPE REMOVE 1", # Remove 'b', implicitly shifting 'c' to 1 and 'd' to 2
    "ROPE QUERY 1",
    "ROPE QUERY 2",
    "ROPE QUERY 3"
);

is($res2->[0], "c", "After removing index 1 ('b'), index 1 is now 'c'");
is($res2->[1], "d", "After removing index 1, index 2 is now 'd'");
is($res2->[2], "", "After removing index 1, index 3 is out of bounds");
