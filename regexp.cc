#include "common.h"
#include "knp_const.h"
#include "morpheme.h"
#include "phrase.h"
#include "grammar.h"
#include "cky.h"

#define MRPH_HINSHI	1
#define MRPH_BUNRUI	2
#define MRPH_KATA	3
#define MRPH_KEI	4
#define MRPH_GOI	5

using std::cerr;
using std::endl;

namespace Parser {

void *matched_ptr; /* memory of the matched morpheme or phrase */

const REGEXPMRPH RegexpMrphInitValue = { 
    MAT_FLG, (char) NULL, 
    /* Hinshi */
    (char) NULL, {0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* Bunrui */
    (char) NULL, {0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* Katuyou_Kata */
    (char) NULL, {0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* Katuyou_Kei */
    (char) NULL, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    /* Goi */
    (char) NULL, {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}};
const REGEXPMRPHS RegexpmrphsInitValue = {NULL, 0};

const REGEXPPHRASE RegexpPhraseInitValue = {
    MAT_FLG, (char) NULL, (char) NULL
};

extern CLASS	Class[CLASSIFY_NO + 1][CLASSIFY_NO + 1];
extern TYPE	Type[TYPE_NO];
extern FORM	Form[TYPE_NO][FORM_NO];

#define         RENGO_ID   "999"

std::vector<int> ANY_ID;


int get_hinsi_id(U_CHAR *x) {
    int i;

    if (x == NULL) {
	cerr << "NULL string for hinsi." << endl;
	return 0;
    }

    if (!strcmp((char *)x, "*")) {
	return 0;
    }

    if (!strcmp((char *)x, "連語")) {
	return(atoi(RENGO_ID));
    }

    for (i = 1; strcmp((char *)Class[i][0].id, (char *)x); ) {
	if (!Class[++i][0].id) {
	    cerr << (char *)x << " is undefined in GRAMMARFILE.";
	}
    }
    return i;
}

int get_bunrui_id(U_CHAR *x, int hinsi) {
    int i;

    if (!x ) {
	cerr << "NULL string for bunrui." << endl;
	return 0;
    }
     
    if (!strcmp((char *)x, "*")) {
	return 0;
    }

    if (!Class[hinsi][1].id) {
	cerr << (char *)Class[hinsi][0].id << " has no bunrui in GRAMMARFILE." << endl;
    }
     
    for (i = 1; strcmp((char *)Class[hinsi][i].id, (char *)x); ) {
	if (!Class[hinsi][++i].id) {
	    cerr << (char *)Class[hinsi][0].id << " does not have bunrui " << (char *)x << " in GRAMMARFILE." << endl;
	}
    }
    return i;
}

int get_type_id(U_CHAR *x) {
    int i;

    if (!x) {
	cerr << "NULL string for type." << endl;
	return 0;
    }

    if (!strcmp((char *)x, "*")) {
	return 0;
    }

    for (i = 1; strcmp((char *)Type[i].name, (char *)x); ) {
	if (!Type[++i].name) {
	    cerr << (char *)x << " is undefined in KATUYOUFILE." << endl;
	}
    }
    return i;
}


/*==================================================================*/
/*   		              Store                                 */
/*==================================================================*/

/*==================================================================*/
		REGEXPMRPH *regexpmrph_alloc(int size)
/*==================================================================*/
{
    REGEXPMRPH *tmp;
    int i;
    
    if (!(tmp = (REGEXPMRPH *)malloc(size * sizeof(REGEXPMRPH)))) {
	fprintf(stderr, "Can't allocate memory for REGEXPMRPH\n");
	exit(-1);
    }
    for (i = 0; i < size; i++) { 
	*(tmp + i) = RegexpMrphInitValue;
    }
    return tmp;
}

/*==================================================================*/
		   REGEXPMRPHS *regexpmrphs_alloc()
/*==================================================================*/
{
    REGEXPMRPHS *tmp;
    
    if (!(tmp = (REGEXPMRPHS *)malloc(sizeof(REGEXPMRPHS)))) {
	fprintf(stderr, "Can't allocate memory for REGEXPMRPHS\n");
	exit(-1);
    }

    return tmp;
}

/*==================================================================*/
	       REGEXPPHRASE *regexpbnst_alloc(int size)
/*==================================================================*/
{
    REGEXPPHRASE *tmp;
    int i;
    
    if (!(tmp = (REGEXPPHRASE *)malloc(size * sizeof(REGEXPPHRASE)))) {
	fprintf(stderr, "Can't allocate memory for REGEXPPHRASE\n");
	exit(-1);
    }
    for (i = 0; i < size; i++) { 
	*(tmp + i) = RegexpPhraseInitValue;
    }
    return tmp;
}

/*==================================================================*/
		  REGEXPPHRASES *regexpbnsts_alloc()
/*==================================================================*/
{
    REGEXPPHRASES *tmp;
    
    if (!(tmp = (REGEXPPHRASES *)malloc(sizeof(REGEXPPHRASES)))) {
	fprintf(stderr, "Can't allocate memory for REGEXPPHRASES\n");
	exit(-1);
    }

    return tmp;
}

/*==================================================================*/
	   void store_mrph_nflag(REGEXPMRPH *mp, int type)
/*==================================================================*/
{
    switch (type) {
      case MRPH_HINSHI: mp->Hinshi_not = NOT_FLG; break;
      case MRPH_BUNRUI: mp->Bunrui_not = NOT_FLG; break;
      case MRPH_KATA:   mp->Kata_not = NOT_FLG; break;
      case MRPH_KEI:    mp->Kei_not = NOT_FLG; break;
      case MRPH_GOI:    mp->Goi_not = NOT_FLG; break;
      default: break;
    }
}

/*==================================================================*/
 static CELL *store_mrph_item(REGEXPMRPH *mp, CELL *mcell, int type)
/*==================================================================*/
{
    int nth;
    CELL *list_cell;
    char *tmp;

    if (Null(car(mcell))) return NULL;

    if (Atomp(car(mcell))) {
	tmp = (char *)_Atom(car(mcell));
	if (!strcmp(tmp, AST_STR)) { 				/* "*" */
	    return cdr(mcell);    
	} 
	else if (!strcmp(tmp, NOT_STR)) { 			/* "^" */
	    store_mrph_nflag(mp, type);
	    mcell = cdr(mcell);
	    if (Atomp(car(mcell))) 
	      list_cell = cons(car(mcell), NULL);    
	    else 
	      list_cell = car(mcell);
	}
	else if (!strncmp(tmp, NOT_STR, strlen(NOT_STR))) { 	/* "^atom" */
	    store_mrph_nflag(mp, type);
	    _Atom(car(mcell)) += strlen(NOT_STR);
	    list_cell = cons(car(mcell), NULL);    
	}
	else {							/* "atom" */
	    list_cell = cons(car(mcell), NULL);
	}
    } else {							/* "(...)" */
	list_cell = car(mcell);
    }
    
    if (type == MRPH_BUNRUI && mp->Hinshi[1] != -1) {
	fprintf(stderr, "Cannot restrict Bunrui for multiple Hinshis.\n");
	error_in_lisp();
    }

    nth = 0;
    while (!Null(car(list_cell))) {
	switch (type) {
	case MRPH_HINSHI:
	    mp->Hinshi[nth] = get_hinsi_id(_Atom(car(list_cell)));
	    break;
	case MRPH_BUNRUI:
	    mp->Bunrui[nth] = 
		get_bunrui_id(_Atom(car(list_cell)), mp->Hinshi[0]);
	    break;
	case MRPH_KATA:
	    mp->Katuyou_Kata[nth] = get_type_id(_Atom(car(list_cell)));
	    break;
	case MRPH_KEI:
	    tmp = (char *)_Atom(car(list_cell));
	    mp->Katuyou_Kei[nth] = 
		(char *)malloc((strlen(tmp)+1) * sizeof(char));
	    strcpy(mp->Katuyou_Kei[nth], tmp);
	    break;
	case MRPH_GOI:
	    tmp = (char *)_Atom(car(list_cell));
	    mp->Goi[nth] = 
		(char *)malloc((strlen(tmp)+1) * sizeof(char));
	    strcpy(mp->Goi[nth], tmp);
	    break;
	default: break;
	}
	list_cell = cdr(list_cell);
	nth++;
    }
    return cdr(mcell);
}

/*==================================================================*/
	       void make_ast_regexpmrph(REGEXPMRPH *mp)
/*==================================================================*/
{
    mp->type_flag = QST_FLG;
    mp->ast_flag = AST_FLG;
}

/*==================================================================*/
	  char store_regexpmrph(REGEXPMRPH *mp, CELL *mcell)
/*==================================================================*/
{
    /* morpheme is specified by a special symbol */

    if (Atomp(mcell)) {
	if (!strcmp((char *)_Atom(mcell), NOT_STR)) {	 	/* "^" */
	    mp->type_flag = NOT_FLG;
	    return NOT_FLG;
	}
	else if (!strcmp((char *)_Atom(mcell), QST_STR)) { 	/* "?" */
	    mp->type_flag = QST_FLG;
	    return QST_FLG;
	}
	else if (!strcmp((char *)_Atom(mcell), AST_STR)) { 	/* "*" */
	    (mp - 1)->ast_flag = AST_FLG;
	    return AST_FLG;
	}
	else if (!strcmp((char *)_Atom(mcell), QST_STR AST_STR)) {	/* "?*" */
	    mp->type_flag = QST_FLG;
	    mp->ast_flag = AST_FLG;
	    return QST_FLG;
	}
	else {
	    fprintf(stderr, "Invalid string for meta mrph (%s).\n", 
		    _Atom(mcell));
	    error_in_lisp();
	    return (char) NULL;
	}
    } 

    /* normal morphme */

    else {
	mp->f_pattern.fp[0] = NULL;

	if ((mcell = store_mrph_item(mp, mcell, MRPH_HINSHI)) == NULL) 
	    return MAT_FLG;
	if ((mcell = store_mrph_item(mp, mcell, MRPH_BUNRUI)) == NULL) 
	    return MAT_FLG;
	if ((mcell = store_mrph_item(mp, mcell, MRPH_KATA)) == NULL) 
	    return MAT_FLG;
	if ((mcell = store_mrph_item(mp, mcell, MRPH_KEI)) == NULL) 
	    return MAT_FLG;
	if ((mcell = store_mrph_item(mp, mcell, MRPH_GOI)) == NULL) 
	    return MAT_FLG;

	list2feature_pattern(&(mp->f_pattern), car(mcell));
	if (Null(cdr(mcell))) {
	    return MAT_FLG;
	} else {
	    fprintf(stderr, "Invalid string for NOT_FLAG.\n");
	    error_in_lisp();
	    return (char) NULL;
	}
    }
}

/*==================================================================*/
	  void store_regexpmrphs(REGEXPMRPHS **mspp, CELL *cell)
/*==================================================================*/
{
    int mrph_num = 0;
    
    if (cell == NULL) {
	*mspp = NULL;
	return;
    }

    *mspp = regexpmrphs_alloc();
    (*mspp)->mrph = regexpmrph_alloc(length(cell));

    while (!Null(cell)) {
	switch(store_regexpmrph(((*mspp)->mrph)+mrph_num, car(cell))) {
	case MAT_FLG: case QST_FLG:
	    mrph_num++; break;
	case NOT_FLG: case AST_FLG:
	    break;
	default:
	    break;
	}
	cell = cdr(cell);
    }
    (*mspp)->mrphsize = mrph_num;
}

/*==================================================================*/
	char store_regexpphrase(REGEXPPHRASE *bp, CELL *cell)
/*==================================================================*/
{
    /* phrase is specified by a special symbol */

    if (Atomp(cell)) {
	if (!strcmp((char *)_Atom(cell), NOT_STR)) {	 	/* "^" */
	    bp->type_flag = NOT_FLG;
	    return NOT_FLG;
	}
	else if (!strcmp((char *)_Atom(cell), QST_STR)) { 	/* "?" */
	    bp->type_flag = QST_FLG;
	    return QST_FLG;
	}
	else if (!strcmp((char *)_Atom(cell), AST_STR)) { 	/* "*" */
	    (bp - 1)->ast_flag = AST_FLG;
	    return AST_FLG;
	}
	else if (!strcmp((char *)_Atom(cell), QST_STR AST_STR)) {	/* "?*" */
	    bp->type_flag = QST_FLG;
	    bp->ast_flag = AST_FLG;
	    return QST_FLG;
	}
	else {
	    fprintf(stderr, "Invalid string for meta bnst (%s).\n", 
		    _Atom(cell));
	    error_in_lisp();
	    return (char) NULL;
	}
    } 

    /* normal phrase */

    else {
	store_regexpmrphs(&(bp->mrphs), car(cell));
	if (!Null(cdr(cell))) 
	    list2feature_pattern(&(bp->f_pattern), car(cdr(cell)));
	else
	    bp->f_pattern.fp[0] = NULL;

	return MAT_FLG;
    }
}

/*==================================================================*/
      void store_regexpphrases(REGEXPPHRASES **bspp, CELL *cell)
/*==================================================================*/
{
    int bnst_num = 0;
    
    if (cell == NULL) {
	*bspp = NULL;
	return;
    }

    *bspp = regexpbnsts_alloc();
    (*bspp)->bnst = regexpbnst_alloc(length(cell));

    while (!Null(cell)) {
	switch(store_regexpphrase(((*bspp)->bnst)+bnst_num, car(cell))) {
	case MAT_FLG: case QST_FLG:
	    bnst_num++; break;
	case NOT_FLG: case AST_FLG:
	    break;
	default:
	    break;
	}
	cell = cdr(cell);
    }
    (*bspp)->bnstsize = bnst_num;
}


/*==================================================================*/
/*   		              Matching                              */
/*==================================================================*/

/*==================================================================*/
    int rule_HBK_cmp(char flg, int r_data[], int data)
/*==================================================================*/
{
    /* Matching of POS, fine-grained POS and inflection type */
    int i, tmp_ret = FALSE;

    if (r_data[0] == -1 || r_data[0] == 0)
	return TRUE;
    else {
	for (i = 0; r_data[i]!=-1; i++)
	    if (r_data[i] == data) {
		tmp_ret = TRUE;
		break;
	    }
	if ((flg == MAT_FLG && tmp_ret == TRUE) ||
	    (flg == NOT_FLG && tmp_ret == FALSE))
	    return TRUE;
	else 
	    return FALSE;
    }
}

/*==================================================================*/
    int rule_Kei_cmp(char flg, char *r_string[], int kata, int kei)
/*==================================================================*/
{
    /* Matching of inflection form */
    int i, tmp_ret = FALSE;

    if (r_string[0] == NULL || !strcmp(r_string[0], AST_STR)) {
	return TRUE;
    }
    else if (kata == 0 || kei == 0) {
	return FALSE;
    }
    else {
	// std::cerr << ";; rule_Kei_cmp is not implemented yet." << std::endl;
	// return FALSE;

	for (i = 0; r_string[i]; i++)
	    if (!strcmp(r_string[i], (char *)Form[kata][kei].name)) {
		tmp_ret = TRUE;
		break;
	    }
	if ((flg == MAT_FLG && tmp_ret == TRUE) || 
	    (flg == NOT_FLG && tmp_ret == FALSE)) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }
}

/*==================================================================*/
	   int mrph_check_function(char *rule, char *data)
/*==================================================================*/
{
    if (0) {
	;
    }
    else {
	fprintf(stderr, ";; Invalid Mrph-Feature-Function (%s)\n", rule);
	return FALSE;
    }
    return FALSE;
}

/*==================================================================*/
     int rule_Goi_cmp(char flg, char *r_string[], const char *d_string)
/*==================================================================*/
{
    /* Matching of word string */
    int i, tmp_ret = FALSE;

    if (r_string[0] == NULL || !strcmp(r_string[0], AST_STR))
	return TRUE;
    else {
	for (i = 0; r_string[i]; i++) {
	    /* function call
	    if (r_string[i][0] == '&')
		if (mrph_check_function(r_string[i], d_string)) {
		    tmp_ret = TRUE;
		    break;
		} */
	    if (!strcmp(r_string[i], d_string)) {
		tmp_ret = TRUE;
		break;
	    }
	}
	if ((flg == MAT_FLG && tmp_ret == TRUE) ||
	    (flg == NOT_FLG && tmp_ret == FALSE))
	    return TRUE;
	else 
	    return FALSE;
    }
}

/*==================================================================*/
	int regexpmrph_match(REGEXPMRPH *ptr1, Morpheme *ptr2)
/*==================================================================*/
{
    /* Matching of morphemes */

    int ret_mrph;

    /* '?' */
    if (ptr1->type_flag == QST_FLG) {
	return TRUE;
    }
    else {
	if (rule_HBK_cmp(ptr1->Hinshi_not, ptr1->Hinshi, ptr2->get_pos()) &&
	    rule_HBK_cmp(ptr1->Bunrui_not, ptr1->Bunrui, ptr2->get_spos()) &&
	    rule_HBK_cmp(ptr1->Kata_not, ptr1->Katuyou_Kata, ptr2->get_conj_type()) &&
	    rule_Kei_cmp(ptr1->Kei_not, ptr1->Katuyou_Kei, ptr2->get_conj_type(), ptr2->get_conj_form()) &&
	    rule_Goi_cmp(ptr1->Goi_not, ptr1->Goi, ptr2->get_genkei().c_str())) {
	    ret_mrph = TRUE;
	}
	else {
	    ret_mrph = FALSE;
	}

	if ((ptr1->type_flag == MAT_FLG && ret_mrph == TRUE) ||
	    (ptr1->type_flag == NOT_FLG && ret_mrph == FALSE)) {
	    return feature_pattern_match(&(ptr1->f_pattern), ptr2->get_f(), NULL, ptr2);
	}
	else {
	    return FALSE;
	}
    }
}

/*==================================================================*/
	  int regexpmrphs_match(REGEXPMRPH *r_ptr, int r_num,
				std::vector<Morpheme *>::iterator d_ptr, int d_num, 
				int fw_or_bw, 
				int all_or_part, 
				int short_or_long)
/*==================================================================*/
{
    int step, return_num;

    (fw_or_bw == FW_MATCHING) ? (step = 1) : (step = -1);

    if (r_num == 0) {
	if (d_num == 0 || all_or_part == PART_MATCHING)
	    return d_num;
	else 
	    return -1;
    } else {
        if (r_ptr->ast_flag == AST_FLG) {
	    
	    /* 
	       if "condition*" is in the pattern, check the following possibilities

	        1. proceed only the pattern (skip "*" in the pattern)
	        2. proceed only the data (if condition matches the data)

	       do 1 first: SHORT_MATCHING, do 2 first: LONG_MATCHING
	    */
	    
	    if (short_or_long == SHORT_MATCHING) {
		if ((return_num = 
		     regexpmrphs_match(r_ptr + step, r_num - 1,
				       d_ptr, d_num,
				       fw_or_bw,
				       all_or_part,
				       short_or_long)) != -1)
		    return return_num;
		else if (d_num &&
			regexpmrph_match(r_ptr, *d_ptr) &&
			(return_num = 
			 regexpmrphs_match(r_ptr,r_num,
					   d_ptr + step, d_num - 1,
					   fw_or_bw,
					   all_or_part,
					   short_or_long)) != -1)
		    return return_num;
		else 
		    return -1;
	    } else {
		if (d_num &&
		    regexpmrph_match(r_ptr, *d_ptr) &&
		    (return_num = 
		     regexpmrphs_match(r_ptr, r_num,
				       d_ptr + step, d_num - 1,
				       fw_or_bw,
				       all_or_part,
				       short_or_long)) != -1)
		    return return_num;
		else if ((return_num = 
			  regexpmrphs_match(r_ptr + step, r_num - 1,
					    d_ptr, d_num,
					    fw_or_bw,
					    all_or_part,
					    short_or_long)) != -1)
		    return return_num;
		else 
		    return -1;
	    }		
        } else {
	    if (d_num &&
		regexpmrph_match(r_ptr, *d_ptr) &&
		(return_num = 
		 regexpmrphs_match(r_ptr + step, r_num - 1,
				   d_ptr + step, d_num - 1,
				   fw_or_bw,
				   all_or_part,
				   short_or_long)) != -1)
		return return_num;
	    else 
		return -1;
	} 
    }
}

/*==================================================================*/
	  int regexpmrphs_match(REGEXPMRPH *r_ptr, int r_num,
				Morpheme *d_ptr, int d_num, 
				int fw_or_bw, 
				int all_or_part, 
				int short_or_long)
/*==================================================================*/
{
    std::vector<Morpheme *> morphemes; // Morpheme pointer -> std::vector<Morpheme *>
    for (int i = 0; i < d_num; i++) {
	morphemes.push_back(d_ptr + i);
    }
    return regexpmrphs_match(r_ptr, r_num, morphemes.begin(), morphemes.size(), 
			     fw_or_bw, all_or_part, short_or_long);
}

/*==================================================================*/
     int regexpmrphrule_match(MrphRule *r_ptr, std::vector<Morpheme *>::iterator d_ptr,
			      int bw_length, int fw_length)
/*==================================================================*/
{
    /* 
       pre_pattern  (shortest match)
       self_pattern (longest match)
       post_pattern (shortest match)
       
       first, check pre_pattern, then from longest match of self_pattern, 
       after that post_pattern
    */

    int match_length, match_rest;

    matched_ptr = NULL;	/* initialize the memory of matched morpheme or phrase */

    /* first check pre_pattern */

    if ((r_ptr->pre_pattern == NULL &&	/* differ! */
	 bw_length != 0) ||
	(r_ptr->pre_pattern != NULL &&
	 regexpmrphs_match(r_ptr->pre_pattern->mrph + 
			   r_ptr->pre_pattern->mrphsize - 1,
			   r_ptr->pre_pattern->mrphsize,
			   d_ptr - 1, 
			   bw_length,	/* differ! */
			   BW_MATCHING, 
			   ALL_MATCHING,/* differ! */
			   SHORT_MATCHING) == -1))
	return -1;

    
    /* then, check from longest match of self_pattern, after that check post_pattern
       match_length: the (possible) length of matching of self_pattern match */

    match_length = fw_length;		/* differ! */

    while (match_length > 0) {
	if (r_ptr->self_pattern == NULL) {
	    match_length = 1;	/* if there is no self_pattern, 
				   length is assumed to be 1 */
	}
	else if ((match_rest = 
		  regexpmrphs_match(r_ptr->self_pattern->mrph, 
				    r_ptr->self_pattern->mrphsize,
				    d_ptr,
				    match_length,
				    FW_MATCHING, 
				    PART_MATCHING,
				    LONG_MATCHING)) != -1) {
	    match_length -= match_rest;
	}
	else {
	    return -1;
	}

	if (r_ptr->post_pattern == NULL || 
	    regexpmrphs_match(r_ptr->post_pattern->mrph, 
			      r_ptr->post_pattern->mrphsize,
			      d_ptr + match_length,
			      fw_length - match_length,	/* differ! */
			      FW_MATCHING, 
			      ALL_MATCHING,		/* differ! */ 
			      SHORT_MATCHING) != -1) {
	    return match_length;
	}
	match_length --;
    }

    return -1;
}

int regexpmrphs_match_for_lattice(CKY *cky, REGEXPMRPH *r_ptr,int r_num,
				  Morpheme *d_ptr, int d_pos, int d_num, 
				  int fw_or_bw, 
				  int all_or_part, 
				  int short_or_long);

/*==================================================================*/
int regexpmrphs_lattice_match(CKY *cky, REGEXPMRPH *r_ptr, int r_num,
			      Morpheme *d_ptr, int d_pos, int d_num, 
			      int fw_or_bw, 
			      int all_or_part, 
			      int short_or_long, vector<int> &valid_mrph_id)
/*==================================================================*/
{
    int i, return_num;

    /* to match ?* in rule, call at least once if there is no data */
    if (d_num == 0) {
	if ((return_num = regexpmrphs_match_for_lattice(cky, r_ptr, r_num, 
							NULL, d_pos, d_num, 
							fw_or_bw, 
							all_or_part, 
							short_or_long)) != -1) {
	    return return_num;
	}
    }
    else {
	/* if morpheme is specified (first call) */
	if (d_ptr) {
	    if ((return_num = regexpmrphs_match_for_lattice(cky, r_ptr, r_num,
							    d_ptr, d_pos, d_num, 
							    fw_or_bw, 
							    all_or_part, 
							    short_or_long)) != -1) {
		return return_num;
	    }
	}
	/* if checking context, check all the possibilities of morpheme lengths */
	else {
	    if (fw_or_bw == FW_MATCHING) {
		/* check morpheme cells in CKY from (d_pos, d_pos) to (d_pos, Chr_num - 1) */
		for (i = d_pos; i < cky->get_token_num(); i++) {
		    CKY_cell *cky_ptr = cky->get_base_cky_cell(d_pos, i);
		    while (cky_ptr && cky_ptr->exist()) {
			if (cky_ptr->get_mrph_ptr() && 
			    cky_ptr->get_mrph_ptr()->check_id(valid_mrph_id) && 
			    (return_num = regexpmrphs_match_for_lattice(cky, r_ptr, r_num,
									cky_ptr->get_mrph_ptr(), d_pos, d_num, 
									fw_or_bw, 
									all_or_part, 
									short_or_long)) != -1) {
			    return return_num;
			}
			cky_ptr = cky_ptr->get_next();
		    }
		}
	    }
	    /* reverse direction */
	    else {
		/* check morpheme cells in CKY from (d_pos, d_pos) to (0, d_pos) */
		for (i = d_pos; i >= 0; i--) {
		    CKY_cell *cky_ptr = cky->get_base_cky_cell(i, d_pos);
		    while (cky_ptr && cky_ptr->exist()) {
			if (cky_ptr->get_mrph_ptr() && 
			    cky_ptr->get_mrph_ptr()->check_id(valid_mrph_id) && 
			    (return_num = regexpmrphs_match_for_lattice(cky, r_ptr, r_num,
									cky_ptr->get_mrph_ptr(), d_pos, d_num, 
									fw_or_bw, 
									all_or_part, 
									short_or_long)) != -1) {
			    return return_num;
			}
			cky_ptr = cky_ptr->get_next();
		    }
		}
	    }
	}
    }

    return -1;
}

/*==================================================================*/
int regexpmrphs_match_for_lattice(CKY *cky, REGEXPMRPH *r_ptr,int r_num,
				  Morpheme *d_ptr, int d_pos, int d_num, 
				  int fw_or_bw, 
				  int all_or_part, 
				  int short_or_long)
/*==================================================================*/
{
    int step, return_num;

    (fw_or_bw == FW_MATCHING) ? (step = 1) : (step = -1);

    if (r_num == 0) {
	if (d_num == 0 || all_or_part == PART_MATCHING)
	    return d_num;
	else 
	    return -1;
    } else {
        if (r_ptr->ast_flag == AST_FLG) {
	    
	    /* 
               if "condition*" is in the pattern, check the following possibilities

	        1. proceed only the pattern (skip "*" in the pattern)
	        2. proceed only the data (if condition matches the data)

	       do 1 first: SHORT_MATCHING, do 2 first: LONG_MATCHING
	    */
	    
	    if (short_or_long == SHORT_MATCHING) {
		if ((return_num = 
		     regexpmrphs_match_for_lattice(cky, r_ptr + step, r_num - 1, 
						   d_ptr, d_pos, d_num, 
						   fw_or_bw,
						   all_or_part,
						   short_or_long)) != -1)
		    return return_num;
		else if (d_num &&
			regexpmrph_match(r_ptr, d_ptr) &&
			(return_num = 
			 regexpmrphs_lattice_match(cky, r_ptr, r_num,
						   NULL, (fw_or_bw == FW_MATCHING) ? d_pos + d_ptr->get_token_num() : d_pos - d_ptr->get_token_num(), d_num - d_ptr->get_token_num(), 
						   fw_or_bw,
						   all_or_part,
						   short_or_long, 
						   (fw_or_bw == FW_MATCHING) ? d_ptr->get_forward_id() : d_ptr->get_backward_id())) != -1)
		    return return_num;
		else 
		    return -1;
	    } else {
		if (d_num &&
		    regexpmrph_match(r_ptr, d_ptr) &&
		    (return_num = 
		     regexpmrphs_lattice_match(cky, r_ptr, r_num,
					       NULL, (fw_or_bw == FW_MATCHING) ? d_pos + d_ptr->get_token_num() : d_pos - d_ptr->get_token_num(), d_num - d_ptr->get_token_num(), 
					       fw_or_bw,
					       all_or_part,
					       short_or_long, 
					       (fw_or_bw == FW_MATCHING) ? d_ptr->get_forward_id() : d_ptr->get_backward_id())) != -1)
		    return return_num;
		else if ((return_num = 
			  regexpmrphs_match_for_lattice(cky, r_ptr + step, r_num - 1,
							d_ptr, d_pos, d_num, 
							fw_or_bw,
							all_or_part,
							short_or_long)) != -1)
		    return return_num;
		else 
		    return -1;
	    }		
        } else {
	    if (d_num &&
		regexpmrph_match(r_ptr, d_ptr) &&
		(return_num = 
		 regexpmrphs_lattice_match(cky, r_ptr + step, r_num - 1, 
					   NULL, (fw_or_bw == FW_MATCHING) ? d_pos + d_ptr->get_token_num() : d_pos - d_ptr->get_token_num(), d_num - d_ptr->get_token_num(), 
					   fw_or_bw,
					   all_or_part,
					   short_or_long, 
					   (fw_or_bw == FW_MATCHING) ? d_ptr->get_forward_id() : d_ptr->get_backward_id())) != -1)
		return return_num;
	    else 
		return -1;
	} 
    }
}

/*==================================================================*/
int regexpmrphrule_lattice_match(CKY *cky, MrphRule *r_ptr, Morpheme *d_ptr, int d_pos, 
				 int bw_length, int fw_length)
/*==================================================================*/
{
    /* 
       pre_pattern  (shortest match)
       self_pattern (longest match)
       post_pattern (shortest match)
       
       first, check pre_pattern, then from longest match of self_pattern, 
       after that post_pattern
    */

    int match_length, match_rest;

    matched_ptr = NULL;	/* initialize the memory of matched morpheme or phrase */

    /* first check pre_pattern */

    if ((r_ptr->pre_pattern == NULL &&	/* differ! */
	 bw_length != 0) ||
	(r_ptr->pre_pattern != NULL &&
	 regexpmrphs_lattice_match(cky, 
				   r_ptr->pre_pattern->mrph + r_ptr->pre_pattern->mrphsize - 1,
				   r_ptr->pre_pattern->mrphsize,
				   NULL, d_pos - 1, 
				   bw_length,	/* differ! */
				   BW_MATCHING, 
				   ALL_MATCHING,/* differ! */
				   SHORT_MATCHING, ANY_ID) == -1))
	return -1;

 
    /* then, check from longest match of self_pattern, after that check post_pattern
       match_length: the (possible) length of matching of self_pattern match */   

    match_length = fw_length;		/* differ! */

    while (match_length > 0) {
	if (r_ptr->self_pattern == NULL) {
	    match_length = 1;	/* if there is no self_pattern, 
				   length is assumed to be 1 */
	}
	else if ((match_rest = 
		  regexpmrphs_lattice_match(cky, 
					    r_ptr->self_pattern->mrph, 
					    r_ptr->self_pattern->mrphsize,
					    d_ptr, d_pos, 
					    match_length,
					    FW_MATCHING, 
					    PART_MATCHING,
					    LONG_MATCHING, ANY_ID)) != -1) {
	    match_length -= match_rest;
	}
	else {
	    return -1;
	}

	if (r_ptr->post_pattern == NULL || 
	    regexpmrphs_lattice_match(cky, 
				      r_ptr->post_pattern->mrph, 
				      r_ptr->post_pattern->mrphsize,
				      NULL, d_pos + match_length, 
				      fw_length - match_length,	/* differ! */
				      FW_MATCHING, 
				      ALL_MATCHING,		/* differ! */ 
				      SHORT_MATCHING, ANY_ID) != -1) {
	    return match_length;
	}
	match_length --;
    }

    return -1;
}

/*==================================================================*/
       int regexpphrase_match(REGEXPPHRASE *ptr1, Phrase *ptr2)
/*==================================================================*/
{
    /* Matching of phrases */

    int ret_mrph;

    /* '?' */
    if (ptr1->type_flag == QST_FLG) {
	return TRUE;
    }
    else {
	if (regexpmrphs_match(ptr1->mrphs->mrph, ptr1->mrphs->mrphsize, 
			      ptr2->get_morphemes().begin(), ptr2->get_mrph_num(), 
			      FW_MATCHING, ALL_MATCHING, SHORT_MATCHING) != -1) {
	    ret_mrph = TRUE;
	}
	else {
	    ret_mrph = FALSE;
	}

	if ((ptr1->type_flag == MAT_FLG && ret_mrph == TRUE) ||
	    (ptr1->type_flag == NOT_FLG && ret_mrph == FALSE)) {
	    return feature_pattern_match(&(ptr1->f_pattern), ptr2->get_f(), NULL, ptr2);
	}
	else {
	    return FALSE;
	}
    }
}

/*==================================================================*/
       int regexpphrases_match(REGEXPPHRASE *r_ptr, int r_num,
			       std::vector<Phrase *>::iterator d_ptr, int d_num, 
			       int fw_or_bw, 
			       int all_or_part, 
			       int short_or_long)
/*==================================================================*/
{
    int step, return_num;

    (fw_or_bw == FW_MATCHING) ? (step = 1) : (step = -1);

    if (r_num == 0) {
	if (d_num == 0 || all_or_part == PART_MATCHING)
	    return d_num;
	else 
	    return -1;
    } else {
        if (r_ptr->ast_flag == AST_FLG) {
	    
	    /* 
               if "condition*" is in the pattern, check the following possibilities

	        1. proceed only the pattern (skip "*" in the pattern)
	        2. proceed only the data (if condition matches the data)

	       do 1 first: SHORT_MATCHING, do 2 first: LONG_MATCHING
	    */

	    if (short_or_long == SHORT_MATCHING) {
		if ((return_num = 
		     regexpphrases_match(r_ptr + step, r_num - 1, 
					 d_ptr, d_num, 
					 fw_or_bw, 
					 all_or_part, 
					 short_or_long)) != -1)
		    return return_num;
		else if (d_num &&
			 regexpphrase_match(r_ptr, *d_ptr) &&
			 (return_num = 
			  regexpphrases_match(r_ptr, r_num, 
					      d_ptr + step, d_num - 1, 
					      fw_or_bw, 
					      all_or_part, 
					      short_or_long)) != -1)
		    return return_num;
		else 
		    return -1;
	    } else {
		if (d_num &&
		    regexpphrase_match(r_ptr, *d_ptr) &&
		    (return_num = 
		     regexpphrases_match(r_ptr, r_num, 
					 d_ptr + step, d_num - 1, 
					 fw_or_bw, 
					 all_or_part, 
					 short_or_long)) != -1)
		    return return_num;
		else if ((return_num = 
			  regexpphrases_match(r_ptr + step, r_num - 1, 
					      d_ptr, d_num, 
					      fw_or_bw, 
					      all_or_part, 
					      short_or_long)) != -1)
		    return return_num;
		else
		    return -1;
	    }
	} else {
	    if (d_num &&
		regexpphrase_match(r_ptr, *d_ptr) &&
		(return_num = 
		 regexpphrases_match(r_ptr + step, r_num - 1, 
				     d_ptr + step, d_num - 1, 
				     fw_or_bw, 
				     all_or_part, 
				     short_or_long)) != -1)
		return return_num;
	    else 
		return -1;
	} 
    }
}

/*==================================================================*/
int regexpphraserule_match(PhraseRule *r_ptr, std::vector<Phrase *>::iterator d_ptr,
			   int bw_length, int fw_length)
/*==================================================================*/
{
    /* 
       pre_pattern  (shortest match)
       self_pattern (longest match)
       post_pattern (shortest match)

       first, check pre_pattern, then from longest match of self_pattern, 
       after that post_pattern       
    */

    int match_length, match_rest;

    matched_ptr = NULL;	/* initialize the memory of matched morpheme or phrase */

    /* first check pre_pattern */

    if ((r_ptr->pre_pattern == NULL &&	/* differ! */
	 bw_length != 0) ||
	(r_ptr->pre_pattern != NULL &&
	 regexpphrases_match(r_ptr->pre_pattern->bnst + 
			     r_ptr->pre_pattern->bnstsize - 1,
			     r_ptr->pre_pattern->bnstsize,
			     d_ptr - 1, 
			     bw_length,	/* differ! */
			     BW_MATCHING, 
			     ALL_MATCHING,/* differ! */
			     SHORT_MATCHING) == -1))
	return -1;


    /* then, check from longest match of self_pattern, after that check post_pattern
       match_length: the (possible) length of matching of self_pattern match */    

    match_length = fw_length;		/* differ! */

    while (match_length > 0) {
	if (r_ptr->self_pattern == NULL) {
	    match_length = 1;	/* if there is no self_pattern, 
				   length is assumed to be 1 */
	}
	else if ((match_rest = 
		  regexpphrases_match(r_ptr->self_pattern->bnst, 
				      r_ptr->self_pattern->bnstsize,
				      d_ptr,
				      match_length,
				      FW_MATCHING, 
				      PART_MATCHING,
				      LONG_MATCHING)) != -1) {
	    match_length -= match_rest;
	}
	else {
	    return -1;
	}

	if (r_ptr->post_pattern == NULL || 
	    regexpphrases_match(r_ptr->post_pattern->bnst, 
				r_ptr->post_pattern->bnstsize,
				d_ptr + match_length,
				fw_length - match_length,	/* differ! */
				FW_MATCHING, 
				ALL_MATCHING,		/* differ! */ 
				SHORT_MATCHING) != -1) {
	    return match_length;
	}
	match_length --;
    }

    return -1;
}

int regexpphrases_match_for_lattice(CKY *cky, REGEXPPHRASE *r_ptr,int r_num,
				    Phrase *d_ptr, int d_pos, int d_num, 
				    int fw_or_bw, 
				    int all_or_part, 
				    int short_or_long);

/*==================================================================*/
int regexpphrases_lattice_match(CKY *cky, REGEXPPHRASE *r_ptr, int r_num,
				Phrase *d_ptr, int d_pos, int d_num, 
				int fw_or_bw, 
				int all_or_part, 
				int short_or_long)
/*==================================================================*/
{
    int i, return_num;

    /* to match ?* in rule, call at least once if there is no data */
    if (d_num == 0) {
	if ((return_num = regexpphrases_match_for_lattice(cky, r_ptr, r_num, 
							  NULL, d_pos, d_num, 
							  fw_or_bw, 
							  all_or_part, 
							  short_or_long)) != -1) {
	    return return_num;
	}
    }
    else {
	/* if phrase is specified (first call) */
	if (d_ptr) {
	    if ((return_num = regexpphrases_match_for_lattice(cky, r_ptr, r_num,
							      d_ptr, d_pos, d_num, 
							      fw_or_bw, 
							      all_or_part, 
							      short_or_long)) != -1) {
		return return_num;
	    }
	}
	/* if checking context, all the possibilities of phrase lengths */
	else {
	    if (fw_or_bw == FW_MATCHING) {
		/* check phrase cells in CKY from (d_pos, d_pos) to (d_pos, Chr_num - 1) */
		for (i = d_pos; i < cky->get_token_num(); i++) {
		    CKY_cell *cky_ptr = cky->get_base_cky_cell(d_pos, i);
		    while (cky_ptr && cky_ptr->exist()) {
			if (cky_ptr->get_bp_ptr() && 
			    (return_num = regexpphrases_match_for_lattice(cky, r_ptr, r_num,
									  cky_ptr->get_bp_ptr(), d_pos, d_num, 
									  fw_or_bw, 
									  all_or_part, 
									  short_or_long)) != -1) {
			    return return_num;
			}
			cky_ptr = cky_ptr->get_next();
		    }
		}
	    }
	    /* reverse direction */
	    else {
		/* check phrase cells in CKY from (d_pos, d_pos) to (0, d_pos) */
		for (i = d_pos; i >= 0; i--) {
		    CKY_cell *cky_ptr = cky->get_base_cky_cell(i, d_pos);
		    while (cky_ptr && cky_ptr->exist()) {
			if (cky_ptr->get_bp_ptr() && 
			    (return_num = regexpphrases_match_for_lattice(cky, r_ptr, r_num,
									  cky_ptr->get_bp_ptr(), d_pos, d_num, 
									  fw_or_bw, 
									  all_or_part, 
									  short_or_long)) != -1) {
			    return return_num;
			}
			cky_ptr = cky_ptr->get_next();
		    }
		}
	    }
	}
    }

    return -1;
}

/*==================================================================*/
int regexpphrases_match_for_lattice(CKY *cky, REGEXPPHRASE *r_ptr,int r_num,
				    Phrase *d_ptr, int d_pos, int d_num, 
				    int fw_or_bw, 
				    int all_or_part, 
				    int short_or_long)
/*==================================================================*/
{
    int step, return_num;

    (fw_or_bw == FW_MATCHING) ? (step = 1) : (step = -1);

    if (r_num == 0) {
	if (d_num == 0 || all_or_part == PART_MATCHING)
	    return d_num;
	else 
	    return -1;
    } else {
        if (r_ptr->ast_flag == AST_FLG) {
	    
	    /* 
               if "condition*" is in the pattern, check the following possibilities

	        1. proceed only the pattern (skip "*" in the pattern)
	        2. proceed only the data (if condition matches the data)

	       do 1 first: SHORT_MATCHING, do 2 first: LONG_MATCHING
	    */

	    if (short_or_long == SHORT_MATCHING) {
		if ((return_num = 
		     regexpphrases_match_for_lattice(cky, r_ptr + step, r_num - 1, 
						     d_ptr, d_pos, d_num, 
						     fw_or_bw, 
						     all_or_part, 
						     short_or_long)) != -1)
		    return return_num;
		else if (d_num &&
		     regexpphrase_match(r_ptr, d_ptr) &&
		     (return_num = 
		      regexpphrases_lattice_match(cky, r_ptr, r_num, 
						  NULL, (fw_or_bw == FW_MATCHING) ? d_pos + d_ptr->get_token_num() : d_pos - d_ptr->get_token_num(), d_num - d_ptr->get_token_num(), 
					      fw_or_bw, 
					      all_or_part, 
					      short_or_long)) != -1)
		    return return_num;
		else 
		    return -1;
	    } else {
		if (d_num &&
		    regexpphrase_match(r_ptr, d_ptr) &&
		    (return_num = 
		     regexpphrases_lattice_match(cky, r_ptr, r_num, 
						 NULL, (fw_or_bw == FW_MATCHING) ? d_pos + d_ptr->get_token_num() : d_pos - d_ptr->get_token_num(), d_num - d_ptr->get_token_num(), 
					     fw_or_bw, 
					     all_or_part, 
					     short_or_long)) != -1)
		    return return_num;
		else if ((return_num = 
			  regexpphrases_match_for_lattice(cky, r_ptr + step, r_num - 1, 
							  d_ptr, d_pos, d_num, 
							  fw_or_bw, 
							  all_or_part, 
							  short_or_long)) != -1)
		    return return_num;
		else
		    return -1;
	    }
	} else {
	    if (d_num &&
		regexpphrase_match(r_ptr, d_ptr) &&
		(return_num = 
		 regexpphrases_lattice_match(cky, r_ptr + step, r_num - 1, 
					     NULL, (fw_or_bw == FW_MATCHING) ? d_pos + d_ptr->get_token_num() : d_pos - d_ptr->get_token_num(), d_num - d_ptr->get_token_num(), 
					 fw_or_bw, 
					 all_or_part, 
					 short_or_long)) != -1)
		return return_num;
	    else 
		return -1;
	} 
    }
}

/*==================================================================*/
int regexpphraserule_lattice_match(CKY *cky, PhraseRule *r_ptr, Phrase *d_ptr, int d_pos, 
				   int bw_length, int fw_length)
/*==================================================================*/
{
    /* 
       pre_pattern  (shortest match)
       self_pattern (longest match)
       post_pattern (shortest match)

       first, check pre_pattern, then from longest match of self_pattern, 
       after that post_pattern              
    */

    int match_length, match_rest;

    matched_ptr = NULL;	/* initialize the memory of matched morpheme or phrase */

    /* first check pre_pattern */

    if ((r_ptr->pre_pattern == NULL &&	/* differ! */
	 bw_length != 0) ||
	(r_ptr->pre_pattern != NULL &&
	 regexpphrases_lattice_match(cky, 
				     r_ptr->pre_pattern->bnst + r_ptr->pre_pattern->bnstsize - 1,
				     r_ptr->pre_pattern->bnstsize,
				     NULL, d_pos - 1, 
				     bw_length,	/* differ! */
				     BW_MATCHING, 
				     ALL_MATCHING, /* differ! */
				     SHORT_MATCHING) == -1))
	return -1;

 
    /* then, check from longest match of self_pattern, after that check post_pattern
       match_length: the (possible) length of matching of self_pattern match */       

    match_length = fw_length;		/* differ! */

    while (match_length > 0) {
	if (r_ptr->self_pattern == NULL) {
	    match_length = 1;	/* if there is no self_pattern, 
				   length is assumed to be 1 */
	}
	else if ((match_rest = 
		  regexpphrases_lattice_match(cky, 
					      r_ptr->self_pattern->bnst, 
					      r_ptr->self_pattern->bnstsize,
					      d_ptr, d_pos,
					      match_length,
					      FW_MATCHING, 
					      PART_MATCHING,
					      LONG_MATCHING)) != -1) {
	    match_length -= match_rest;
	}
	else {
	    return -1;
	}

	if (r_ptr->post_pattern == NULL || 
	    regexpphrases_lattice_match(cky, 
					r_ptr->post_pattern->bnst, 
					r_ptr->post_pattern->bnstsize,
					NULL, d_pos + match_length,
					fw_length - match_length,	/* differ! */
					FW_MATCHING, 
					ALL_MATCHING,		/* differ! */ 
					SHORT_MATCHING) != -1) {
	    return match_length;
	}
	match_length --;
    }

    return -1;
}

}

/*====================================================================
				 END
====================================================================*/
