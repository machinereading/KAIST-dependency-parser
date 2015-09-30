#!/usr/bin/env perl

use encoding 'euc-jp', STDOUT => 'utf8';
use strict;

while (<STDIN>) {
    if (/^(\d+) (\S+) (\d+) (\d+) (.*)/) {
	my ($id, $connect_ids, $n1, $n2, $str) = ($1, $2, $3, $4, $5);
	$n1 = $n1 / 2 * 3;
	$n2 = $n2 / 2 * 3;
	print "$id $connect_ids $n1 $n2 $str\n";
    }
    else {
	print;
    }
}
