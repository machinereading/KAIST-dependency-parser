#include "common.h"
#include "grammar.h"

using std::cerr;
using std::endl;

extern int LineNo;
extern int LineNoForError;

namespace Parser {

CLASS	Class[CLASSIFY_NO + 1][CLASSIFY_NO + 1];
std::map<std::string, int> POS2POSID;
TYPE	Type[TYPE_NO];
FORM	Form[TYPE_NO][FORM_NO];

Grammar::Grammar(Parameter *in_param) {
    param = in_param;

    for (int i = 0; i < CLASSIFY_NO + 1; i++) {
	for (int j = 0; j < CLASSIFY_NO + 1; j++) {
	    Class[i][j].id = (U_CHAR *)NULL;
	    Class[i][j].kt = FALSE;
	    Class[i][j].cost = 0;
	}
    }

    for (int i = 0; i < TYPE_NO; i++) {
	Type[i].name = (U_CHAR *)NULL;
	for (int j = 0; j < FORM_NO; j++) {
	    Form[i][j].name = (U_CHAR *)NULL;
	    Form[i][j].gobi = (U_CHAR *)NULL;
	    Form[i][j].gobi_yomi = (U_CHAR *)NULL;
	}
    }

    read_grammar(in_param->juman_grammar_filename.c_str()); // GRAMMARFILE
    read_katuyou(in_param->juman_katuyou_filename.c_str()); // KATUYOUFILE
}

bool Grammar::read_katuyou(const char *filename) {
    FILE *fp;
    CELL *cell1, *cell2;
    int	i, j;

    if (param->debug_flag)
        cerr << filename << " parsing... ";
    if ((fp = fopen(filename, "r")) == NULL) {
	cerr << ";; Cannot open file (" << filename << ") !!" << endl;
	exit(1);
    }

    LineNo = 1;
    i = 1;
    while (! s_feof(fp)) {
	LineNoForError = LineNo;
	cell1 = s_read(fp);
	Type[i].name = (U_CHAR *)
	    my_alloc((sizeof(U_CHAR)*strlen((char *)_Atom(car(cell1)))) + 1);
	strcpy((char *)Type[i].name, (char *)_Atom(car(cell1)));
	cell1 = car(cdr(cell1));
	j = 1;
	while (!Null(cell2 = car(cell1))) {
	    Form[i][j].name = (U_CHAR *)
		my_alloc((sizeof(U_CHAR) * strlen((char *)_Atom(car(cell2)))) + 1);
	    strcpy((char *)Form[i][j].name, (char *)_Atom(car(cell2)));

	    Form[i][j].gobi = (U_CHAR *)
		my_alloc((sizeof(U_CHAR) * strlen((char *)_Atom(car(cdr(cell2))))) + 1);
	    if (strcmp((char *)_Atom(car(cdr(cell2))), "*") == 0)
		strcpy((char *)Form[i][j].gobi, "");
	    else
		strcpy((char *)Form[i][j].gobi, (char *)_Atom(car(cdr(cell2))));

	    if (!Null(car(cdr(cdr(cell2))))) {
		/* KANJI is mixed in suffix */
		Form[i][j].gobi_yomi = (U_CHAR *)
		    my_alloc((sizeof(U_CHAR) * strlen((char *)_Atom(car(cdr(cdr(cell2)))))) + 1);
		if (strcmp((char *)_Atom(car(cdr(cdr(cell2)))), "*") == 0)
		    strcpy((char *)Form[i][j].gobi_yomi, "");
		else
		    strcpy((char *)Form[i][j].gobi_yomi, (char *)_Atom(car(cdr(cdr(cell2)))));
	    } else {
		Form[i][j].gobi_yomi = (U_CHAR *)
		    my_alloc(sizeof(U_CHAR) * strlen((char *)Form[i][j].gobi) + 1);
		strcpy((char *)Form[i][j].gobi_yomi, (char *)Form[i][j].gobi);
	    }

	    j++;
	    cell1 = cdr(cell1);
	}
	i++;
    }
    fclose(fp);
    if (param->debug_flag)
        cerr << "done." << endl;
    return true;
}

bool Grammar::read_grammar(const char *filename) {
    FILE *fp;
    CELL *cell1, *cell2;
    int i, j;
    int katuyou_flag = 0;

    if (param->debug_flag)
        cerr << filename << " parsing... ";
    if ((fp = fopen(filename, "r")) == NULL) {
	cerr << ";; Cannot open file (" << filename << ") !!" << endl;
	exit(1);
    }

    LineNo = 1;
    i = 1;
    while (!s_feof(fp)) {
	j = 0;
	LineNoForError = LineNo;

	cell1 = s_read(fp);

	if ( !Null(cell2 = car(cell1)) ) {
	    Class[i][j].id = (U_CHAR *)
		my_alloc((sizeof(U_CHAR)*strlen((char *)_Atom(car(cell2)))) + 1);
	    strcpy((char *)Class[i][j].id, (char *)_Atom(car(cell2)));
            std::string tmp_pos_str = (char *)_Atom(car(cell2));
            POS2POSID[tmp_pos_str] = i;
	    if ( !Null(cdr(cell2)) ) {
		katuyou_flag = 1;
		Class[i][j].kt = TRUE;
	    } else
		katuyou_flag = 0;
	    cell1 = car(cdr(cell1));
	    j++;
	} else {
	    fprintf(stderr, "\nparse error at line %d\n", LineNo);
	    exit(GramError);
	}

	while (!Null(cell2 = car(cell1))) {
	    Class[i][j].id = (U_CHAR *)
		my_alloc((sizeof(U_CHAR)*strlen((char *)_Atom(car(cell2)))) + 1);
	    strcpy((char *)Class[i][j].id, (char *)_Atom(car(cell2)));

	    if ( katuyou_flag || !Null(cdr(cell2)) )
		Class[i][j].kt = TRUE;

	    cell1 = cdr(cell1);
	    j++;
	    if ( j >= CLASSIFY_NO ) {
		fprintf(stderr, "\nparse error at line %d\n", LineNo);
		fprintf(stderr, "\ttoo many sub-classfication.\n");
		exit(GramError); 
	    }
	}
	i++;
	if ( i >= CLASSIFY_NO ) {
	    fprintf(stderr, "\nparse error at line %d\n", LineNo);
	    fprintf(stderr, "\ttoo many classfication.\n");
	    exit(GramError);
	}
    }
    fclose(fp);
    if (param->debug_flag)
        cerr << "done." << endl;
    return true;
}

}
