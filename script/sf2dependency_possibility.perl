#!/usr/bin/env perl

# extract depnedency rules and possibilities from a treebank

# Usage:
# perl -I perl script/sf2dependency_possibility.perl --outdir test_rule.korean somewhere/treebank/*.xml 2> case.prob

use XML::LibXML;
use StandardFormat;
use Getopt::Long;
use strict;
use utf8;

binmode(STDERR, ':encoding(utf8)');
binmode(STDOUT, ':encoding(utf8)');

our $PUNCTUATION_POS_REGEXP = '特殊';
our $PUNCTUATION_POS_SEXP = '特殊';
our $PHRASE_TYPE_STR = 'BP_TYPE';
our $POS_DELIMITER = '+';

our $DEPENDENCY_RULE_FILENAME = 'dependency.rule';
our $PHRASE_RULE_FILENAME = 'phrase_basic.rule';

our %opt;
&GetOptions(\%opt, 'debug', 'exact', 'punctuation=s', 'outdir=s', 'list', 'phrase_rule', 'prob_threshold=f', 'dpnd_num_threshold=i', 'head_num_threshold=i');
if ($opt{punctuation}) {
    my (@punc);
    push(@punc, split(',', $opt{punctuation}));
    $PUNCTUATION_POS_SEXP = '(' . join(' ', @punc) . ')';
    my $regexp_str = join('|', @punc);
    $PUNCTUATION_POS_REGEXP = qr/(?:$regexp_str)/;
}
$opt{outdir} = '.' if !$opt{outdir};
mkdir($opt{outdir}, 0755) if ! -d $opt{outdir};

our $PROB_THRESHOLD = exists($opt{prob_threshold}) ? $opt{prob_threshold} : 0.001;
our $DPND_NUM_THRESHOLD = exists($opt{dpnd_num_threshold}) ? $opt{dpnd_num_threshold} : 10;
our $HEAD_NUM_THRESHOLD = exists($opt{head_num_threshold}) ? $opt{head_num_threshold} : 2;

our $parser = new XML::LibXML;
our $sf = new StandardFormat;

my (%dpnd, %phrase, %rdpnd);
for my $file (@ARGV) {
    my $xmldat;

    open(XML, '< :utf8', $file) or die;
    while (<XML>) {
	$xmldat .= $_;
    }
    close(XML);

    &process_one_file($parser, $xmldat);
}

# open an output dependency rule file
open(DEP_RULE, '>:encoding(utf8)', "$opt{outdir}/$DEPENDENCY_RULE_FILENAME") or die;

# write dependency rules
for my $pos (sort {$dpnd{$b}{ALL} <=> $dpnd{$a}{ALL}} keys %dpnd) {
    last if $DPND_NUM_THRESHOLD and $dpnd{$pos}{ALL} < $DPND_NUM_THRESHOLD;
    my $buf = "( (($PHRASE_TYPE_STR:$pos))\n\t(";
    my $first;
    for my $head_pos (sort {$dpnd{$pos}{POS}{$b}{ALL} <=> $dpnd{$pos}{POS}{$a}{ALL}} keys %{$dpnd{$pos}{POS}}) {
	for my $direction (keys %{$dpnd{$pos}{POS}{$head_pos}{DIRECTION}}) {
	    last if $HEAD_NUM_THRESHOLD and $dpnd{$pos}{POS}{$head_pos}{DIRECTION}{$direction} < $HEAD_NUM_THRESHOLD;
	    my $prob = $dpnd{$pos}{POS}{$head_pos}{DIRECTION}{$direction} / $dpnd{$pos}{ALL};
	    next if $prob < $PROB_THRESHOLD;
	    printf STDERR "%s %s %s %.5f (%d / %d)\n", $pos, $head_pos, $direction, $prob, 
		$dpnd{$pos}{POS}{$head_pos}{DIRECTION}{$direction}, $dpnd{$pos}{ALL} if $opt{debug};

	    if ($first++) {
		$buf .= "\n\t ";
	    }
	    $buf .= "[(($PHRASE_TYPE_STR:$head_pos)) $direction]";
	}

	for my $dist (keys %{$rdpnd{$head_pos}{POS}{$pos}}) {
	    printf STDERR "%s,%d|G:%s %.5f\n", $pos, $dist, $head_pos, $rdpnd{$head_pos}{POS}{$pos}{$dist} / $rdpnd{$head_pos}{ALL}, $rdpnd{$head_pos}{POS}{$pos}{$dist}, $rdpnd{$head_pos}{ALL};
	}
	$phrase{$head_pos} = 1 unless exists($phrase{$head_pos});
    }
    next unless $first; # no head candidate
    $buf .= ")\n\t() 1 )";
    print DEP_RULE $buf, "\n";
    $phrase{$pos} = 1 unless exists($phrase{$pos});
}

close(DEP_RULE);

# open an output phrase rule file
open(PHRASE_RULE, '>:encoding(utf8)', "$opt{outdir}/$PHRASE_RULE_FILENAME") or die;

# write phrase rules
if ($opt{list}) { # output as a list
    for my $pos_sequence (keys %phrase) {
	print PHRASE_RULE "$pos_sequence\n";
    }
}
else {
    print PHRASE_RULE "( ( ) ( ? ) ( ?* ) BOS )\n\n"; # a rule for beginning of sentence

    if ($opt{phrase_rule}) {
	for my $pos_sequence (keys %phrase) {
	    print PHRASE_RULE "( ( ?* ) ( < (\[$PUNCTUATION_POS_SEXP\]*";
	    for my $pos (split('-', $pos_sequence)) {
		print PHRASE_RULE " \[$pos\]";
		print PHRASE_RULE " \[$pos\]*" if !$opt{exact};
	    }
	    print PHRASE_RULE " \[$PUNCTUATION_POS_SEXP\]* ) > ) ( ?* ) $PHRASE_TYPE_STR:$pos_sequence )\n";
	}
    }
}

close(PHRASE_RULE);


# process sentences in a file
sub process_one_file {
    my ($parser, $xmldat) = @_;

    my $doc = $parser->parse_string($xmldat);

    for my $sentence_node ($doc->getElementsByTagName('S')) { # sentence loop
	my $sentence_id = $sentence_node->getAttribute('Id');
	&process_one_sentence($sentence_node, $sentence_id);
    }
}

# process a sentence
sub process_one_sentence {
    my ($sentence_node, $sentence_id) = @_;

    for my $annotation_node ($sentence_node->getElementsByTagName('Annotation')) { # parse
	$sf->read_annotation_from_node($annotation_node);

	# dependency
	for my $id (sort {$sf->{phrases}{$a} <=> $sf->{phrases}{$b}} keys %{$sf->{phrases}}) {
	    next if !$sf->{phrases}{$id}{head_ids}; # skip roots (e.g., English)
	    for my $head_id (@{$sf->{phrases}{$id}{head_ids}}) {
		next if $head_id == -1; # skip roots of Japanese (-1)
		my $dpnd_phrase_pos = &get_pos_sequence_from_phrase($sf->{phrases}{$id});
		my $head_phrase_pos = &get_pos_sequence_from_phrase($sf->{phrases}{$head_id});
		if ($dpnd_phrase_pos and $head_phrase_pos) {
		    my $direction = ($head_id > $id) ? 'R' : 'L'; # dependency direction
		    # for each modifier POS
		    $dpnd{$dpnd_phrase_pos}{POS}{$head_phrase_pos}{DIRECTION}{$direction}++;
		    $dpnd{$dpnd_phrase_pos}{POS}{$head_phrase_pos}{ALL}++;
		    $dpnd{$dpnd_phrase_pos}{ALL}++;
		    # for each head POS
		    $rdpnd{$head_phrase_pos}{POS}{$dpnd_phrase_pos}{&get_dist($sf->{phrases}, $id, $head_id)}++;
		    $rdpnd{$head_phrase_pos}{ALL}++;
		}
	    }
	}
    }
}

sub get_pos_sequence_from_phrase {
    my ($phrase) = @_;

    my (@pos_array, $prev_pos);
    for my $word (@{$phrase->{words}}) {
	next if $word->{pos} =~ /^$PUNCTUATION_POS_REGEXP/; # skip fullstop and comma
	my $pos;
	if ($word->{pos} =~ /^([^:]+):([^:]+)$/) { # course-grained pos and fine-grained pos
	    $pos = $1;
	}
	else {
	    $pos = $word->{pos};
	}
	next if !$opt{exact} and $prev_pos eq $pos;
	push(@pos_array, $pos);
	$prev_pos = $pos;
    }

    return join($POS_DELIMITER, @pos_array);
}

# get a distance between a modifier and its head (difference of phrase id)
sub get_dist {
    my ($phrases, $modifier_id, $head_id) = @_;

    my $dist = abs($head_id - $modifier_id);
    # my $dist = $phrases->{$head_id}{position} - $phrases->{$modifier_id}{position};

    if ($dist == 1) {
	return $dist;
    }
    elsif ($dist < 6) {
	return 2;
    }
    else {
	return 6;
    }
}
