#!/usr/bin/env perl

# convert cf XML format to knpdict format
# $0 cf-sample.xml > cf.knpdict

use XML::LibXML;
use Getopt::Long;
use strict;
use utf8;

binmode(STDERR, ':encoding(utf8)');
binmode(STDOUT, ':encoding(utf8)');

our %opt;
&GetOptions(\%opt, 'verbose', 'language=s', 'default');

our $parser = new XML::LibXML;
our $MAX_ARG = 20;

our $ENTRY_NUM = 0;
our $CASEFRAME_NUM = 0;
our $CASE_NUM = 0;

for my $file (@ARGV) {
    my $xmldat;

    open(XML, '< :utf8', $file) or die;
    while (<XML>) {
	next if /^<\?xml/ or /^<\/?caseframedata/ or /^(?:\s|\@)*<\!--/;
	$xmldat .= $_;
	if (m|^\s*</entry|) {
	    &process_one_file($parser, $xmldat) if $xmldat;
	    $xmldat = ''
	}
    }
    close(XML);

    &process_one_file($parser, $xmldat) if $xmldat;
}

# default case frame
&output_default_cf() if $opt{default};

if ($opt{verbose}) {
    print STDERR "# of predicates (entries): ", $ENTRY_NUM, "\n";
    print STDERR "# of case frames: ", $CASEFRAME_NUM, "\n";
    print STDERR "# of cases: ", $CASE_NUM, "\n";
    printf STDERR "Average # of case frames for a predicate: %.3f\n", $CASEFRAME_NUM / $ENTRY_NUM, if $ENTRY_NUM;
    printf STDERR "Average # of cases for a case frame: %.3f\n", $CASE_NUM / $CASEFRAME_NUM, if $CASEFRAME_NUM;
}


sub process_one_file {
    my ($parser, $xmldat) = @_;

    my $doc = $parser->parse_string($xmldat);

    for my $entry_node ($doc->getElementsByTagName('entry')) { # sentence loop
	my $headword = $entry_node->getAttribute('headword');
	&process_one_entry($entry_node, $headword);
	$ENTRY_NUM++;
    }
}

sub extract_yomi {
    my ($str) = @_;

    if ($str =~ m|^[^/]+/(.+)|) {
	return $1;
    }
    else {
	return $str;
    }
}

sub process_one_entry {
    my ($entry_node, $headword) = @_;

    for my $cf_node ($entry_node->getElementsByTagName('caseframe')) { # caseframe
	my $cfid = $cf_node->getAttribute('id');
	print "ID $cfid\n";
	printf "読み %s\n", &extract_yomi($headword);
	print "表記 $headword\n";
	print "素性 NIL\n";

	my $arg_count = 1;
	for my $arg_node ($cf_node->getElementsByTagName('argument')) { # argument
	    &process_one_arg($arg_node, $arg_count++);
	    $CASE_NUM++;
	    last if $arg_count > $MAX_ARG;
	}
	$CASEFRAME_NUM++;
    }
}

sub process_one_arg {
    my ($arg_node, $n) = @_;

    my $case = $arg_node->getAttribute('case');
    my $closest = $arg_node->getAttribute('closest');
    $case =~ s/格$//; # delete "格" for Japanese
    $case .= '*' if $closest;
    print "格$n $case\n";


    print "用例$n";
    my $count = 0;
    for my $comp_node ($arg_node->getElementsByTagName('component')) { # component
	my $freq = $comp_node->getAttribute('frequency');
	my $str = $comp_node->textContent();
	print " $str:$freq";
	$count++;
    }
    print ' NIL' if $count == 0;
    print "\n";
    print "意味$n NIL\n";
}

sub output_default_cf {
    if ($opt{language} =~ /korean/i) {
	print << 'EOF';
ID DEFAULT
読み DEFAULT
表記 DEFAULT
素性 NIL
格1 가
用例1 NIL
意味1 NIL
格2 를*
用例2 NIL
意味2 NIL
格3 에
用例3 NIL
意味3 NIL
格4 로
用例4 NIL
意味4 NIL
EOF
    }
}
