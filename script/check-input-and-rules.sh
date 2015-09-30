#!/bin/sh

# check whether input morphemes are covered in the dependency rules

usage() {
    echo "$0 output_MorphAnalyzerToJuman.txt"
    exit 1
}

parser_dir=`dirname $0`/..

if [ ! -f "$1" ]; then
    usage
fi
kma_output=$1

# perl $parser_dir/script/conv-kma-output.perl < $kma_output > kma_out_conv.txt
# perl $parser_dir/script/delete-sentence-end-morpheme.perl < kma_out_conv.txt | LANG=C sort | LANG=C uniq -c | LANG=C sort -nr > input.sorted
perl $parser_dir/script/delete-sentence-end-morpheme.perl < $kma_output | LANG=C sort | LANG=C uniq -c | LANG=C sort -nr > input.sorted
perl $parser_dir/script/extract-modifier-rules.perl < $parser_dir/rule/korean/dependency.rule > rules.txt
perl $parser_dir/script/check-input-and-rules.perl input.sorted < rules.txt
rm -f input.sorted rules.txt
