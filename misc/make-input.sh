#!/bin/sh

piconv -f utf-8 -t euc-jp | juman -m -E | misc/euc2utf8.perl
