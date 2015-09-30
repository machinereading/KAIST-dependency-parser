#!/usr/bin/env perl

# output POS of morphemes except sentence-end
# Usage: $0 < output_MorphAnalyzerToJuman.txt > kma_out_conv.txt

use strict;

my @buf;
while (<STDIN>) {
    if (/^EOS/) {
	pop(@buf); # delete the sentence-end morpheme
	for my $mrph (@buf) {
	    my $pos = (split(' ', $mrph))[3];
	    print $pos, "\n";
	}
	@buf = ();
    }
    else {
	push(@buf, $_);
    }
}
