#include "common.h"
#include "knp_const.h"
#include "morpheme.h"
#include "phrase.h"

namespace Parser {

using std::cout;
using std::cerr;
using std::endl;

void FEATURE::print() {
    print(std::cout);
}

void FEATURE::print_xml() {
    print_xml(std::cout);
}

void FEATURE::_print(std::ostream &os, const char *open_bracket, const char *closed_bracket) {
    FEATURE *fp = this;
    while (fp) {
	os << open_bracket << fp->cp << closed_bracket;
	fp = fp->next;
    }
}

void FEATURE::print(std::ostream &os) {
    _print(os, "<", ">");
}

void FEATURE::print_xml(std::ostream &os) {
    _print(os, "&lt;", "&gt;");
}

/*==================================================================*/
		   void clear_feature(FEATURE **fpp)
/*==================================================================*/
{
    FEATURE *fp, *next;

    fp = *fpp;
    *fpp = NULL;

    while (fp) {
	next = fp->next;
	free(fp->cp);
	free(fp);
	fp = next;
    }
}

/*==================================================================*/
	   void copy_cfeature(FEATURE **fpp, char *fname)
/*==================================================================*/
{
    while (*fpp) fpp = &((*fpp)->next);

    if (!((*fpp) = (FEATURE *)(malloc(sizeof(FEATURE)))) ||
	!((*fpp)->cp = (char *)(malloc(strlen(fname) + 1)))) {
	cerr << ";; Can't allocate memory for FEATURE" << endl;
	return;
    }
    strcpy((*fpp)->cp, fname);
    (*fpp)->next = NULL;
}

/*==================================================================*/
	      void list2feature(CELL *cp, FEATURE **fpp)
/*==================================================================*/
{
    while (!Null(car(cp))) {
	copy_cfeature(fpp, (char *)_Atom(car(cp)));
	fpp = &((*fpp)->next);
	cp = cdr(cp);	    
    }
}

/*==================================================================*/
      void list2feature_pattern(FEATURE_PATTERN *f, CELL *cell)
/*==================================================================*/
{
    /* transform list of ((BOS)(NP)(TOPIC)) etc. to FEATURE_PATTERN */

    int nth = 0;

    while (!Null(car(cell))) {
	clear_feature(f->fp+nth);		/* ?? &(f->fp[nth]) */ 
	list2feature(car(cell), f->fp+nth);	/* ?? &(f->fp[nth]) */ 
	cell = cdr(cell);
	nth++;
    }
    f->fp[nth] = NULL;
}

/*==================================================================*/
          int comp_feature(char *data, const char *pattern)
/*==================================================================*/
{
    /* 
     *  exact match or partial match (pattern is short, next char is ':')
     */
    if (data && !strcmp(data, pattern)) {
	return TRUE;
    } else if (data && !strncmp(data, pattern, strlen(pattern)) &&
	       data[strlen(pattern)] == ':') {
	return TRUE;
    } else {
	return FALSE;
    }
}

/*==================================================================*/
         char *check_feature(FEATURE *fp, const char *fname)
/*==================================================================*/
{
    while (fp) {
	if (comp_feature(fp->cp, fname) == TRUE) {
	    return fp->cp;
	}
	fp = fp->next;
    }
    return NULL;
}

/*==================================================================*/
		  int levelcmp(char *cp1, char *cp2)
/*==================================================================*/
{
    int level1, level2;
    if (!strcmp(cp1, "A-"))      level1 = 1;
    else if (!strcmp(cp1, "A"))  level1 = 2;
    else if (!strcmp(cp1, "B-")) level1 = 3;
    else if (!strcmp(cp1, "B"))  level1 = 4;
    else if (!strcmp(cp1, "B+")) level1 = 5;
    else if (!strcmp(cp1, "C"))  level1 = 6;
    else fprintf(stderr, "Invalid level (%s)\n", cp1);
    if (!strcmp(cp2, "A-"))      level2 = 1;
    else if (!strcmp(cp2, "A"))  level2 = 2;
    else if (!strcmp(cp2, "B-")) level2 = 3;
    else if (!strcmp(cp2, "B"))  level2 = 4;
    else if (!strcmp(cp2, "B+")) level2 = 5;
    else if (!strcmp(cp2, "C"))  level2 = 6;
    else fprintf(stderr, "Invalid level (%s)\n", cp2);
    return level1 - level2;
}

/*==================================================================*/
	int subordinate_level_comp(Phrase *ptr1, Phrase *ptr2)
/*==================================================================*/
{
    char *level1, *level2;

    level1 = check_feature(ptr1->get_f(), "LEVEL");
    level2 = check_feature(ptr2->get_f(), "LEVEL");

    if (level1 == NULL) return TRUE;		/* NONE: ALL -> T */
    else if (level2 == NULL) return FALSE;	/* ALL: NONE -> F */
    else if (levelcmp(level1 + strlen("LEVEL:"), 
		      level2 + strlen("LEVEL:")) <= 0)	/* ptr1 <= ptr2 -> T */
	return TRUE;
    else return FALSE;
}

/*==================================================================*/
	  int subordinate_level_check(char *cp, FEATURE *f)
/*==================================================================*/
{
    char *level1, *level2;

    level1 = cp;
    level2 = check_feature(f, "LEVEL");

    if (level1 == NULL) return TRUE;		/* NONE: ALL -> T */
    else if (level2 == NULL) return FALSE;	/* ALL: NONE -> F */
    else if (levelcmp(level1, level2 + strlen("LEVEL:")) <= 0)
	return TRUE;				/* cp <= f -> T */
    else return FALSE;
}

/*==================================================================*/
 int check_function(char *rule, FEATURE *fd, void *ptr1, void *ptr2)
/*==================================================================*/
{
    /* rule : rule
       fd : FEATURE of data
       p1 : if dependency, the class of dependency (Morpheme, Phrase etc.)
       p2 : the class of data (Morpheme, Phrase etc.)
    */

    // the morpheme consists of KANJIs
    if (!strcmp(rule, "&KANJI")) {
	if (check_str_type(((Morpheme *)ptr2)->get_goi()) == CHR_KANJI) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }
    // the morpheme consists of symbols, alphabets, figures or KATAKANAs
    else if (!strcmp(rule, "&記英数カ")) {
	chr_type type = check_str_type(((Morpheme *)ptr2)->get_goi());
	if (type == CHR_SYMBOL || 
	    type == CHR_ALPHABET || 
	    type == CHR_FIGURE || 
	    type == CHR_KATAKANA) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }
    // the length of the morpheme is 1
    else if (!strcmp(rule, "&ONE_CHAR")) {
	if (str_length(((Morpheme *)ptr2)->get_goi()) == 1) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }
    // &D : distance check
    else if (!strncmp(rule, "&D:", strlen("&D:"))) {
	int given_dist = atoi(rule + strlen("&D:"));
	if (given_dist == 1) {
	    if ((((Phrase *)ptr2)->get_token_start() - ((Phrase *)ptr1)->get_token_end()) == 1) {
		return TRUE;
	    }
	    else {
		return FALSE;
	    }
	}
	// we don't know distance >= 2
	else {
	    return TRUE;
	}
    }
    // &CONTENT_WORD_MATCH : matching of content word
    else if (!strcmp(rule, "&CONTENT_WORD_MATCH")) {
	if (((Phrase *)ptr1)->get_rep() == ((Phrase *)ptr2)->get_rep()) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }
    // &LEVEL:STRONG : comparation of the level of predicate
    else if (!strcmp(rule, "&LEVEL:STRONG")) {
	return subordinate_level_comp((Phrase *)ptr1, (Phrase *)ptr2);
    }
    // &LEVEL:X : the level of predicate is X or more than X
    else if (!strncmp(rule, "&LEVEL:", strlen("&LEVEL:"))) {
	return subordinate_level_check(rule + strlen("&LEVEL:"), fd);
    }
    return TRUE;
}

/*==================================================================*/
 int feature_AND_match(FEATURE *fp, FEATURE *fd, void *p1, void *p2)
/*==================================================================*/
{
    int value;

    while (fp) {
	if (fp->cp[0] == '^' && fp->cp[1] == '&') {
	    value = check_function(fp->cp+1, fd, p1, p2);
	    if (value == TRUE) {
		return FALSE;
	    }
	} else if (fp->cp[0] == '&') {
	    value = check_function(fp->cp, fd, p1, p2);
	    if (value == FALSE) {
		return FALSE;
	    }
	} else if (fp->cp[0] == '^') {
	    if (check_feature(fd, fp->cp+1)) {
		return FALSE;
	    }
	} else {
	    if (!check_feature(fd, fp->cp)) {
		return FALSE;
	    }
	}
	fp = fp->next;
    }
    return TRUE;
}

/*==================================================================*/
int feature_pattern_match(FEATURE_PATTERN *fr, FEATURE *fd,
			  void *p1, void *p2)
/*==================================================================*/
{
    /* fr : FEATURE_PATTERN of rule
       fd : FEATURE of data
       p1 : if dependency, the class of dependency of dependency (Morpheme, Phrase etc.)
       p2 : the class of data (Morpheme, Phrase etc.)
    */

    int i, value;

    /* if no PATTERN is specified, it is regarded as MATCH */
    if (fr->fp[0] == NULL) return TRUE;

    /* check the condition of each OR */
    for (i = 0; fr->fp[i]; i++) {
	value = feature_AND_match(fr->fp[i], fd, p1, p2);
	if (value == TRUE) 
	    return TRUE;
    }
    return FALSE;
}

/*==================================================================*/
void assign_cfeature(FEATURE **fpp, const char *fname, int temp_assign_flag)
/*==================================================================*/
{
    char feature_buffer[DATA_LEN];

    /* temp_assign_flag: if TRUE, attach "TEMP" to the head */

    /* check override */
    sscanf(fname, "%[^:]", feature_buffer);	/* ※ fnameに":"がない場合は
						   feature_bufferはfname全体になる */

    /* recover if cut by ":" in quote('"') */
    if (strcmp(feature_buffer, fname)) {
	int i, count = 0;

	for (i = 0; i < (int)strlen(feature_buffer); i++) {
	    if (feature_buffer[i] == '"') {
		count++;
	    }
	}
	if (count % 2 == 1) { /* the number of '"' is odd */
	    strcpy(feature_buffer, fname);
	}
    }

    while (*fpp) {
	if (comp_feature((*fpp)->cp, feature_buffer) == TRUE) {
	    free((*fpp)->cp);
	    if (!((*fpp)->cp = (char *)(malloc(strlen(fname) + 1)))) {
		cerr << ";; Can't allocate memory for FEATURE" << endl;
		return;
	    }
	    strcpy((*fpp)->cp, fname);
	    return;	/* end with override */
	}
	fpp = &((*fpp)->next);
    }

    /* if not override, push it */

    if (!((*fpp) = (FEATURE *)(malloc(sizeof(FEATURE)))) ||
	!((*fpp)->cp = (char *)(malloc(strlen(fname) + 8)))) {
	cerr << ";; Can't allocate memory for FEATURE" << endl;
	return;
    }
    if (temp_assign_flag) {
	strcpy((*fpp)->cp, "TEMP:");
	strcat((*fpp)->cp, fname);
    }
    else {
	strcpy((*fpp)->cp, fname);
    }
    (*fpp)->next = NULL;
}    

/*==================================================================*/
void assign_feature(FEATURE **fpp1, FEATURE **fpp2, void *ptr, int offset, int length, int temp_assign_flag)
/*==================================================================*/
{
    /*
     *  after matching a rule, assign FEATURE to a class from the rule
     *  it is possible to perform a process to the class itself
     */

    FEATURE **fpp, *next;
    int assign_pos;

    while (*fpp2) {
	if (*((*fpp2)->cp) == '^') { // if deletion
	    fpp = fpp1;
	    
	    while (*fpp) {
		if (comp_feature((*fpp)->cp, &((*fpp2)->cp[1])) == TRUE) {
		    free((*fpp)->cp);
		    next = (*fpp)->next;
		    free(*fpp);
		    *fpp = next;
		} else {
		    fpp = &((*fpp)->next);
		}
	    }
	}
	else if (*((*fpp2)->cp) == '&') { // if function
	    if (!strcmp((*fpp2)->cp, "&break")) { // this is used when this rule is applied
		;
	    }
	    // 自動辞書 : 自動獲得した辞書をチェック (マッチ部分全体)
	    else if (!strncmp((*fpp2)->cp, "&自動辞書:", strlen("&自動辞書:"))) {
		if (offset == 0) {
		    if (!strncmp((*fpp2)->cp + strlen("&自動辞書:"), "先頭:", strlen("先頭:"))) {
			assign_pos = 0;
		    }
		    else if (!strncmp((*fpp2)->cp + strlen("&自動辞書:"), "末尾:", strlen("末尾:"))) {
			assign_pos = length - 1;
		    }
		    else {
			cerr << ";; Invalid feature:" << (*fpp2)->cp << endl;
                        exit(-1);
		    }
		    // check_auto_dic((Morpheme *)ptr, assign_pos, length, (*fpp2)->cp + strlen("&自動辞書:先頭:"), temp_assign_flag);
		}
	    }
	    else {
		cerr << ";; Not implemented yet: assign_feature - function (" << (*fpp2)->cp << ")" << endl;
	    }
	} 
	else { // if addition
	    assign_cfeature(fpp1, (*fpp2)->cp, temp_assign_flag);	
	}

	fpp2 = &((*fpp2)->next);
    }
}

}
