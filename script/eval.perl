#!/usr/bin/env perl

# calculate unlabeled attachment score
# format: standard format (XML)

# Usage:
# perl -I perl script/eval.perl gold_file auto_file
# --ignore_root: ignore the root
# --each: output accuracy for each sentence

use XML::LibXML;
use StandardFormat;
use Getopt::Long;
use strict;

binmode(STDERR, ':encoding(utf8)');
binmode(STDOUT, ':encoding(utf8)');

sub usage {
    print STDERR "Usage: $0 [--each] [--ignore_root] gold_file auto_file\n";
    exit 1;
}

our %opt;
&GetOptions(\%opt, 'debug', 'ignore_root', 'each');
&usage unless $ARGV[0] && $ARGV[1] && -f $ARGV[0] && -f $ARGV[1];

our $parser = new XML::LibXML;

our $ALL_PARSE_COUNT = 0;
our $SUCCESS_PARSE_COUNT = 0;
our $OK_PARSE_COUNT = 0;
our $OK_DEP_COUNT = 0;
our $OK_PHRASE_COUNT = 0;
our $ALL_PHRASE_COUNT = 0; # count gold phrases for only parse successful sentences
our $ALL_PHRASE_COUNT_GOLD = 0;
our $ALL_PHRASE_COUNT_AUTO = 0;

my @gold_sentence_nodes = &process_one_file($parser, $ARGV[0]); # gold parse
my @auto_sentence_nodes = &process_one_file($parser, $ARGV[1]); # auto parse
die "Mismatch: # of sentences\n" if @gold_sentence_nodes != @auto_sentence_nodes;

for my $i (0 .. @gold_sentence_nodes - 1) { # sentence loop
    &process_one_sentence($gold_sentence_nodes[$i], $auto_sentence_nodes[$i], $i);
}

if ($ALL_PARSE_COUNT) {
    printf "Parse success = %.5f (%d / %d)\n", $SUCCESS_PARSE_COUNT / $ALL_PARSE_COUNT, $SUCCESS_PARSE_COUNT, $ALL_PARSE_COUNT;
    if ($SUCCESS_PARSE_COUNT) {
	printf "Sentence accuracy = %.5f (%d / %d)\n", $OK_PARSE_COUNT / $SUCCESS_PARSE_COUNT, $OK_PARSE_COUNT, $SUCCESS_PARSE_COUNT;
    }
    if ($ALL_PHRASE_COUNT) {
	printf "Phrase accuracy = %.5f (%d / %d)\n", $OK_PHRASE_COUNT / $ALL_PHRASE_COUNT, $OK_PHRASE_COUNT, $ALL_PHRASE_COUNT;
	if ($OK_PHRASE_COUNT) {
	    printf "Dependency accuracy = %.5f (%d / %d)\n", $OK_DEP_COUNT / $OK_PHRASE_COUNT, $OK_DEP_COUNT, $OK_PHRASE_COUNT;
	}
    }
}

my $dep_precision;
if ($ALL_PHRASE_COUNT_AUTO) {
    $dep_precision = $OK_DEP_COUNT / $ALL_PHRASE_COUNT_AUTO;
    printf "Dependency precision = %.5f (%d / %d)\n", $dep_precision, $OK_DEP_COUNT, $ALL_PHRASE_COUNT_AUTO;
}
if ($ALL_PHRASE_COUNT_GOLD) {
    my $dep_recall = $OK_DEP_COUNT / $ALL_PHRASE_COUNT_GOLD;
    printf "Dependency recall = %.5f (%d / %d)\n", $dep_recall, $OK_DEP_COUNT, $ALL_PHRASE_COUNT_GOLD;
    printf "Dependency F-measure = %.5f\n", &calc_f($dep_precision, $dep_recall);
}


# process sentences in a file
sub process_one_file {
    my ($parser, $filename) = @_;

    my $xmldat;
    open(XML, '< :utf8', $filename) or die;
    while (<XML>) {
	$xmldat .= $_;
    }
    close(XML);

    my $doc = $parser->parse_string($xmldat);
    return $doc->getElementsByTagName('S');
}

# process a sentence
sub process_one_sentence {
    my ($gold_sentence_node, $auto_sentence_node, $sentence_count) = @_;

    my $gold_sf = new StandardFormat;
    my $auto_sf = new StandardFormat;

    $gold_sf->read_annotation_from_node($gold_sentence_node);
    $auto_sf->read_annotation_from_node($auto_sentence_node);
    my $gold_phrase_hr = &make_phrase_hash($gold_sf);

    if (scalar(keys %{$auto_sf->{phrases}}) > 0) { # parse success
	my ($ok_dep_count, $ok_phrase_count, $all_phrase_count);
	my $auto_phrase_hr = &make_phrase_hash($auto_sf);
	my $all_phrase_count_auto = $opt{ignore_root} ? scalar(keys %{$auto_phrase_hr}) - 1 : scalar(keys %{$auto_phrase_hr});

	# check dependency
	my $all_dependency_ok = 1;
	for my $range (keys %{$gold_phrase_hr}) {
	    next if $opt{ignore_root} and $gold_phrase_hr->{$range}{head_ids}[0] == -1;
	    if (exists($auto_phrase_hr->{$range})) {
		if (&match_list($auto_phrase_hr->{$range}{head_ids}, $gold_phrase_hr->{$range}{head_ids})) {
		    $ok_dep_count++;
		}
		else {	# dependency mismatch
		    $all_dependency_ok = 0;
		    printf "DEP MISMATCH %s: %s -> G:%s A:%s\n", $gold_phrase_hr->{$range}{phrase_str}, $range, join('/', @{$gold_phrase_hr->{$range}{head_ids}}), join('/', @{$auto_phrase_hr->{$range}{head_ids}}) if ($opt{debug});
		}
		$ok_phrase_count++;
	    }
	    else {
		$all_dependency_ok = 0;
		printf "PHRASE MISMATCH G:%s (%s) is not found in the automatic parse.\n", $gold_phrase_hr->{$range}{phrase_str}, $range if $opt{debug};
	    }
	    $all_phrase_count++;
	}

	if ($opt{each} and $all_phrase_count) {
	    printf "Sentence %s phrase accuracy = %.5f (%d / %d)\n", $auto_sf->{id}, $ok_phrase_count / $all_phrase_count, $ok_phrase_count, $all_phrase_count;
	    # printf "Sentence %s phrase precision = %.5f (%d / %d)\n", $auto_sf->{id}, $ok_phrase_count / $all_phrase_count_auto, $ok_phrase_count, $all_phrase_count_auto;
	    if ($ok_phrase_count) {
		printf "Sentence %s dependency accuracy = %.5f (%d / %d)\n", $auto_sf->{id}, $ok_dep_count / $ok_phrase_count, $ok_dep_count, $ok_phrase_count;
		# printf "Sentence %s dependency precision = %.5f (%d / %d)\n", $auto_sf->{id}, $ok_dep_count / $all_phrase_count_auto, $ok_dep_count, $all_phrase_count_auto;
	    }
	}

	$OK_DEP_COUNT += $ok_dep_count;
	$OK_PHRASE_COUNT += $ok_phrase_count;
	$ALL_PHRASE_COUNT += $all_phrase_count;
	$ALL_PHRASE_COUNT_AUTO += $all_phrase_count_auto;
	$OK_PARSE_COUNT++ if $all_dependency_ok;
	$SUCCESS_PARSE_COUNT++;
    }

    $ALL_PHRASE_COUNT_GOLD += $opt{ignore_root} ? scalar(keys %{$gold_phrase_hr}) - 1 : scalar(keys %{$gold_phrase_hr});
    $ALL_PARSE_COUNT++;
}

sub make_phrase_hash {
    my ($sf) = @_;
    my (%phrases);
    for my $id (keys %{$sf->{phrases}}) {
	my $begin_position_in_chr = $sf->{phrases}{$id}{position_in_chr};
	my $end_position_in_chr = $begin_position_in_chr + length($sf->{phrases}{$id}{phrase_str}) - 1;
	$phrases{"$begin_position_in_chr-$end_position_in_chr"} = $sf->{phrases}{$id};
    }
    return \%phrases;
}

sub match_list {
    my ($list1, $list2) = @_;

    for my $elem1 (@{$list1}) {
	for my $elem2 (@{$list2}) {
	    if ($elem1 == $elem2) {
		return 1;
	    }
	}
    }
    return 0;
}

sub calc_f {
    my ($p, $r) = @_;

    return 2 * $p * $r / ($p + $r);
}
