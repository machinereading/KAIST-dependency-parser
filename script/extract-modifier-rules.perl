#!/usr/bin/env perl

# extract modifiers in dependency.rule
# Usage: $0 < rule/dependency.rule > modifier_rules.txt

use strict;

while (<STDIN>) {
    if (/^\( \(\(BP_TYPE:([^)]+)/) {
	print $1, "\n";
    }
}
