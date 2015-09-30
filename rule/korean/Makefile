RULE2DATA := ../rule2data.pl

rule_files := $(wildcard *.rule)
data_files := $(patsubst %.rule,%.data,$(rule_files))

all: $(data_files)

$(data_files): %.data : %.rule
	$(RULE2DATA) $<

clean:
	rm -f $(data_files)
