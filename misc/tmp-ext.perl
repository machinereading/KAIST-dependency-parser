#!/usr/bin/env perl

my $flag = 0;
while (<>) {
    if (/\# S-ID/) {
#	my ($com, $line) = (/^([^+]+)(\+.+)$/);
#	print $com, "\n";
#	$line =~ s/^(\+ -?\d+)[Rr](.+)/$1D$2/;
#	print $line, "\n";
	print;
	$flag = 1;
	next;
    }

    if ($flag) {
	s/^(\+ -?\d+)[Rr](.+)/$1D$2/;
	print;
    }

    if (/^EOS/) {
	$flag = 0;
    }
}
