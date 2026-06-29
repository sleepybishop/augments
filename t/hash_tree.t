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

# The expected hash for "ab" is 30459
# 97 * 313^1 + 98 * 313^0 = 30361 + 98 = 30459
my $expected_hash = 30459;

my $res1 = run_queries(
    "HASHTREE ADD 1|a",
    "HASHTREE ADD 2|b",
    "HASHTREE HASH"
);
is($res1->[0], $expected_hash, "Hash is correct when inserting 'a' then 'b'");

my $res2 = run_queries(
    "HASHTREE ADD 2|b",
    "HASHTREE ADD 1|a",
    "HASHTREE HASH"
);
is($res2->[0], $expected_hash, "Hash is perfectly invariant when inserting 'b' then 'a' (forcing tree rotations)");

my $res3 = run_queries(
    "HASHTREE ADD 1|a",
    "HASHTREE ADD 3|c",
    "HASHTREE ADD 2|b",
    "HASHTREE HASH",
    "HASHTREE REMOVE 3",
    "HASHTREE HASH"
);

# "abc" hash:
# 97 * 313^2 + 98 * 313 + 99 = 97 * 97969 + 30674 + 99 = 9502993 + 30674 + 99 = 9533766
my $expected_abc = 9533766;
is($res3->[0], $expected_abc, "Hash is correct for 'abc' after middle insertion");
is($res3->[1], $expected_hash, "Hash reverts perfectly to 'ab' after removing 'c'");
