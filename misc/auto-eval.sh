#!/bin/sh

rm -f set.out
for f in `cat ../parser_dat/list.id.sorted`; do cat output/$f.out >> set.out; done
cat set.out | ./misc/tmp-ext.perl | perl -lpe 's/^\+/*/' | nkf -e > set.auto.bnst
eval.pl set.corpus.bnst set.auto.bnst set
tree_cmp.pl set.corpus.bnst set.auto.bnst < set.error > set.tree
nkf -w set.result
