package StandardFormat;

# $Id: StandardFormat.pm,v 1.5 2012/06/06 10:45:11 kawahara Exp $

# Read (new) standard format

use strict;
use XML::LibXML;
use utf8;

sub new {
    my ($this) = @_;

    $this = {parser => new XML::LibXML, word_count => 0};
    bless $this;
}


# read first annotation
sub read_first_annotation {
    my ($this, $xmldat) = @_;

    $this->{doc} = $this->{parser}->parse_string($xmldat);

    for my $annotation_node ($this->{doc}->getElementsByTagName('S')) { # parse
	$this->read_annotation_from_node($annotation_node);
	return;
    }
}

# read annotation from a given annotation node
sub read_annotation_from_node {
    my ($this, $annotation_node) = @_;

    $this->{id} = $annotation_node->getAttribute('Id');

    undef $this->{words};
    undef $this->{phrases};
    my $position_in_chr = 0;

    for my $phrase_node ($annotation_node->getElementsByTagName('phrase')) {
	my (@words, $phrase_str);
	for my $word_node ($phrase_node->getElementsByTagName('word')) {
	    my $str = $word_node->getAttribute('str'); # string that appeared
	    my $lem = $word_node->getAttribute('lem'); # lemma
	    my $repname = $word_node->getAttribute('repname'); # repname
	    my $id = $word_node->getAttribute('id');
	    $this->{words}{$id} = {str => $str, lem => $lem, repname => $repname, 
				   pos => $word_node->getAttribute('pos'), 
				   feature => $word_node->getAttribute('feature'), 
				   content_p => $word_node->getAttribute('content_p'), 
				   position => $this->{word_count}};
	    push(@words, $this->{words}{$id});
	    $this->{word_count}++;
	    $phrase_str .= $str;

	    for my $synnode ($word_node->getElementsByTagName('synnode')) { # synonym node
		my $synid = $synnode->getAttribute('synid');
		my $score = $synnode->getAttribute('score');
		next unless $score < 1; # except the identical expression with the word
		my $wordid = (split(',', $synnode->getAttribute('wordid')))[0]; # the first wordid
		push(@{$this->{words}{$id}{synnodes}}, {synid => $synid, score => $score, wordid => $wordid});
	    }
	}
	if (scalar(@words) == 0) { # ERROR: this phrase doesn't contain any words
	    # warn("WARNING: this phrase doesn't contain any words:\n", $annotation_node->toString());
	    # return;
	    $phrase_str = $phrase_node->textContent();
	}
	my $id = $phrase_node->getAttribute('id');
	my $word_head_num = &get_phrase_head_num(\@words);
	$this->{phrases}{$id} = {head_ids => [split('/', $phrase_node->getAttribute('head'))], 
				 words => \@words, 
				 word_head_num => $word_head_num, # head word in this phrase
				 phrase_str => $phrase_str, 
				 str => @words ? $words[$word_head_num]{str} : '', 
				 lem => @words ? $words[$word_head_num]{lem} : '', 
				 repname => @words ? $words[$word_head_num]{repname} : '', 
				 position => @words ? $words[$word_head_num]{position} : 0, 
				 position_in_chr => $position_in_chr, 
				};
	map({$position_in_chr += length($_->{str})} @words);
    }
}

# get the number of headword in the phrase
sub get_phrase_head_num {
    my ($words_ar) = @_;

    for my $i (reverse(0 .. $#{$words_ar})) {
	if ($words_ar->[$i]{content_p}) {
	    return $i;
	}
    }
    return $#{$words_ar}; # default is the last one in the phrase
}

1;
