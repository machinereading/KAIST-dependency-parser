#!/usr/bin/env perl

# convert KMA output to parser input
# - same POS sequences -> one POS
# - mark predicate and arguments

use strict;
use Getopt::Long;

our (%opt);
&GetOptions(\%opt, 'pos', 'punctuation');

our $PUNCTUATION_POS = 'SF';

my $count = 0;
my $prev_start;
while (<STDIN>) {
    if (/^$/) {
	next;
    }
    elsif (/^EOS/) {
	s/\r$//;
	print;
	$count = 0;
	$prev_start = undef;
    }
    else {
	chomp;
	my @line = split(' ', $_);
	next unless @line == 4;
	if (defined($prev_start) and $prev_start != $line[0]) {
	    $count++;
	}

	if ($opt{pos}) {
	    printf "%d %d ", $count, $count + 1;
	}
	else {
	    printf "%d %d ", $line[0], $line[1];
	}

	my @str_list = split('\+', $line[2]);
	my @pos_list = split('\+', $line[3]);
	my $smoothed_pos_sequence = &smooth_pos_sequence(\@pos_list);

	my @features;
	# push(@features, sprintf("BP_TYPE:%s", $smoothed_pos_sequence));
	push(@features, &get_features(\@str_list, \@pos_list));

	printf "%s %s", $line[2], $smoothed_pos_sequence;
	print ' ', join(',', @features) if @features;
	print "\n";
	$prev_start = $line[0];
    }
}

sub smooth_pos_sequence {
    my ($pos_list_ar) = @_;

    my (@result, $prev_pos);
    for my $pos (@$pos_list_ar) {
	next if $prev_pos eq $pos;
	push(@result, $pos);
	$prev_pos = $pos;
    }

    pop(@result) if $opt{punctuation} and @result > 1 and $result[$#result] eq $PUNCTUATION_POS;

    return join('+', @result);
}

sub get_features {
    my ($str_list_ar, $pos_list_ar) = @_;

    my (@features);
    for my $i (0 .. $#{$str_list_ar}) {
	my $str = $str_list_ar->[$i];
	my $pos = $pos_list_ar->[$i];

	if ($pos eq 'VV') {
	    push(@features, "PRED:$str");
	}

	# arguments
	if ($pos eq 'JKS' and 
	    ($str eq '이' or $str eq '가')) { # nominative
	    push(@features, 'ARG-이');
	}
	elsif ($pos eq 'JKO' and 
	       ($str eq '을' or $str eq '를')) { # accusative
	    push(@features, 'ARG-을');
	}
	# elsif ($str eq '은' or $str eq '는') { # topic marker
	#     push(@features, 'ARG-이', 'ARG-을');
	# }
    }

    return @features;
}
