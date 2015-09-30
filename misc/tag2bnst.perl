#!/usr/bin/env perl

use strict;

my $flag = 0;
my $bnst_line;
my $bnst_end_flag = 0;
my $bnst_count = 0;
my $phrase_count = 0;
my @phrase_table;
my @mrphs;
my @data;

while (<>) {
    if (/\# S-ID/) {
	print;
	$flag = 1;
	next;
    }

    if ($flag) {
	if (/^[\+\*]/) {
	    if ($bnst_end_flag) {
		# $bnst_line =~ s/^(\+ -?\d+)[Rr](.+)/$1D$2/;
		push(@data, {bnst_line => $bnst_line, mrphs => [@mrphs]});
		# print &make_bnst_line($bnst_line);
		# print @mrphs;
		$bnst_end_flag = 0;
		@mrphs = ();
	    }

	    $phrase_table[$phrase_count++] = $bnst_count;

	    unless (/<複合名詞前部>/ or /<文節内>/) {
		$bnst_end_flag = 1;
		$bnst_line = $_;
		$bnst_count++;
	    }
	}
	elsif (/^EOS/) {
	    push(@data, {bnst_line => $bnst_line, mrphs => [@mrphs]});
	    # print &make_bnst_line($bnst_line);
	    # print @mrphs;
	    for my $dat (@data) {
		print &make_bnst_line($dat->{bnst_line});
		print @{$dat->{mrphs}};
	    }
	    print;

	    @data = ();
	    $bnst_end_flag = 0;
	    @mrphs = ();
	    $flag = 0;
	    $bnst_count = 0;
	    $phrase_count = 0;
	    @phrase_table = ();
	}
	else {
	    push(@mrphs, $_);
	}
    }
}


sub make_bnst_line {
    my ($bnst_line) = @_;

   $bnst_line =~ /^[\+\*] (-?\d+)(\w)(.*)/;
    my ($phrase_head, $dpnd_type, $f) = ($1, $2, $3);
    my $bnst_head;
    if ($phrase_head == -1) {
	$bnst_head = -1;
    }
    else {
	$bnst_head = $phrase_table[$phrase_head];
    }
    $dpnd_type =~ s/^[Rr]/D/;

    return "\* $bnst_head$dpnd_type$f\n";
}
