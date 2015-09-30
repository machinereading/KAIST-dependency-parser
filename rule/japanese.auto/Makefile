PHRASE2RULE = ../phrase2rule.pl
RULE2DATA = ../rule2data.pl

.phrase.rule:
	$(PHRASE2RULE) < $< > $@

.rule.data:
	$(RULE2DATA) $<

all:
	$(RULE2DATA) *.rule

clean:
	rm -f *.data
