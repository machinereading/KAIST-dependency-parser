#!/usr/bin/env perl

# check whether input morphemes are covered in the dependency rules
# $0 input.soted < rules.txt > not_found.list

# to produce input.sorted:
# 1. perl script/conv-input.perl < output_MorphAnalyzerToJuman.txt > kma_out_conv.txt
# 2. perl script/delete-sentence-end-morpheme.perl < kma_out_conv.txt | LANG=C sort | LANG=C uniq -c | LANG=C sort -nr > input.sorted

# to produce rules.txt:
# perl script/extract-modifier-rules.perl < rule/dependency.rule > rules.txt


my %rule;
while (<STDIN>) {
    chomp;
    $rule{$_}++;
}

open(IN, $ARGV[0]) or die;
while (<IN>) {
    if (/^\s*(\d+) (.+)/) {
	my ($freq, $pos) = ($1, $2);
	unless (exists($rule{$pos})) {
	    print;
	}
    }
}
close(IN);
