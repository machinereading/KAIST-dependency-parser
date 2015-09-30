#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "lisp.h"
#include "knp_const.h"
#include "parameter.h"

namespace Parser {

#define		GRAMMARFILE	"JUMAN.grammar"
#define 	CLASSIFY_NO	128

#define		KATUYOUFILE	"JUMAN.katuyou"
#define		TYPE_NO		128
#define		FORM_NO		128

// POS and fine-grained POS
typedef		struct		_CLASS {
     U_CHAR	*id;
     int        cost;
     int	kt;
} CLASS;

// Conjugation type
typedef		struct		_TYPE {
     U_CHAR	*name;
} TYPE;

// Inflection form
typedef		struct		_FORM {
     U_CHAR	*name;
     U_CHAR	*gobi;
    U_CHAR	*gobi_yomi;	/* for YOMI of カ変動詞来 etc */
} FORM;

class Grammar {
    Parameter *param;
  public:
    Grammar(Parameter *in_param);
    ~Grammar() {
	;
    }
    bool read_grammar(const char *filename);
    bool read_katuyou(const char *filename);
};

}

#endif
