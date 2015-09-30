CXX = g++
CXXFLAGS = -I lisp -g -Wall # -pg
OBJECTS = morpheme.o phrase.o sentence.o parser.o cky.o regexp.o rule.o feature.o grammar.o caseframe.o case_analysis.o lexeme.o config.o tool.o
MAKE_DB_OBJECTS = make_db.cc
LIBS = -lcdb -lz -Llisp -llisp

all: liblisp.a knp++

knp++: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LIBS)

make_db: $(MAKE_DB_OBJECTS)
	$(CXX) -o $@ $(MAKE_DB_OBJECTS) $(LIBS)

liblisp.a:
	cd lisp && $(MAKE) && cd ..

rules:
	$(MAKE) -C rule

clean:
	rm -f $(OBJECTS) knp++ make_db
	$(MAKE) -C lisp clean
