#include "common.h"
#include "rule.h"
#include "cky.h"

extern int LineNo;
extern int LineNoForError;

namespace Parser {

using std::cout;
using std::cerr;
using std::endl;

extern int regexpmrphs_match(REGEXPMRPH *r_ptr, int r_num,
			     Morpheme *d_ptr, int d_num, 
			     int fw_or_bw, 
			     int all_or_part, 
			     int short_or_long);
extern int regexpmrphrule_match(MrphRule *r_ptr, std::vector<Morpheme *>::iterator d_ptr,
				int bw_length, int fw_length);
extern int regexpmrphrule_lattice_match(CKY *cky, MrphRule *r_ptr, Morpheme *d_ptr, int d_pos, 
					int bw_length, int fw_length);
extern int regexpphraserule_match(PhraseRule *r_ptr, std::vector<Phrase *>::iterator d_ptr,
				  int bw_length, int fw_length);
extern int regexpphraserule_lattice_match(CKY *cky, PhraseRule *r_ptr, Phrase *d_ptr, int d_pos, 
					  int bw_length, int fw_length);
extern char *check_feature(FEATURE *fp, const char *fname);
extern REGEXPMRPH *regexpmrph_alloc(int size);
extern REGEXPMRPHS *regexpmrphs_alloc();
extern void copy_cfeature(FEATURE **fpp, char *fname);

KoouRule	KoouRuleArray[KoouRule_MAX];
int		CurKoouRuleSize = 0;

PhraseRule 	ContRuleArray[ContRule_MAX];
int 	        ContRuleSize = 0;

void		*EtcRuleArray = NULL;
int		CurEtcRuleSize = 0;
int		ExistEtcRule = 0;


Rules::Rules(Parameter *in_param) {
    param = in_param;
    // for general languages
    // read_mrph_rule(param->rule_dirname + "/mrph_unary.data", RLOOP_RMM, RLOOP_BREAK_NONE, LtoR, false);
    // // read_mrph_rule(param->rule_dirname + "/mrph_unary.data", RLOOP_RMM, RLOOP_BREAK_NONE, LtoR, false);
    // read_mrph_rule(param->rule_dirname + "/mrph_basic.data", RLOOP_RMM, RLOOP_BREAK_NONE, LtoR, true);
    // read_cfg_rule(param->rule_dirname + "/cfg_phrase_build.data", RLOOP_RMM);
    // read_phrase_rule(param->rule_dirname + "/phrase_basic.data", RLOOP_RMM, RLOOP_BREAK_NONE, LtoR, true);
    // // read_phrase_rule(param->rule_dirname + "/case_analysis.data", RLOOP_MRM, RLOOP_BREAK_NONE, RtoL, true);
    // read_dpnd_rule(param->rule_dirname + "/dependency.data");

    // for Japanese
    read_mrph_rule(param->rule_dirname + "/mrph_unary.data", RLOOP_RMM, RLOOP_BREAK_NONE, LtoR, false);
    read_homo_rule(param->rule_dirname + "/mrph_homo.data");
    read_mrph_rule(param->rule_dirname + "/mrph_unary.data", RLOOP_RMM, RLOOP_BREAK_NONE, LtoR, false);
    read_mrph_rule(param->rule_dirname + "/mrph_basic.data", RLOOP_RMM, RLOOP_BREAK_NONE, LtoR, true);
    read_cfg_rule(param->rule_dirname + "/cfg_phrase_build.data", RLOOP_RMM);
    read_phrase_rule(param->rule_dirname + "/phrase_unary.data", RLOOP_RMM, RLOOP_BREAK_NONE, RtoL, false);
    read_phrase_rule(param->rule_dirname + "/phrase_basic.data", RLOOP_RMM, RLOOP_BREAK_NONE, LtoR, true);
    read_phrase_rule(param->rule_dirname + "/phrase_type.data", RLOOP_MRM, RLOOP_BREAK_NORMAL, RtoL, true);
    read_phrase_rule(param->rule_dirname + "/phrase_etc.data", RLOOP_RMM, RLOOP_BREAK_NONE, RtoL, true);
    read_phrase_rule(param->rule_dirname + "/case_analysis.data", RLOOP_MRM, RLOOP_BREAK_NONE, RtoL, true);
    read_dpnd_rule(param->rule_dirname + "/dependency.data");
}

bool Rules::read_homo_rule(std::string filename) {
    FILE *fp;
    CELL *body_cell;

    GeneralRuleType *rule = new GeneralRuleType(filename.c_str(), HomoRuleType);
    HomoRule *rp = (HomoRule *)(rule->get_rule_array());
    GeneralRuleArray.push_back(rule);

    // filename = check_rule_filename(filename);
    if ((fp = fopen(filename.c_str(), "r")) == NULL ) {
        if (param->debug_flag)
            cerr << ";; Cannot open file (" << filename << ") !!" << endl;
	return false;
    }
    if (param->debug_flag)
        cerr << "Reading " << filename;

    LineNo = 1;
    while (!s_feof(fp)) {
	LineNoForError = LineNo;

	body_cell = s_read(fp);

	store_regexpmrphs(&(rp->pre_pattern), car(body_cell)); // read forward context of morphemes
	store_regexpmrphs(&(rp->pattern), car(cdr(body_cell))); // read homograph morphemes

	rp->f = NULL;
	list2feature(cdr(cdr(body_cell)), &(rp->f));

	if (rule->increase_rulesize() == false) {
	    break;
	}
	rp++;
    }

    if (param->debug_flag)
        cerr << " (" << rule->get_rulesize() << ") done." << endl;
    // free(filename);
    fclose(fp);
    return true;
}

bool Rules::read_mrph_rule(std::string filename, int mode, int breakmode, int direction, bool lattice_flag) {
    FILE *fp;
    CELL *body_cell;

    GeneralRuleType *rule = new GeneralRuleType(filename.c_str(), lattice_flag ? LatticeMorphRuleType : MorphRuleType, mode, breakmode, direction);
    MrphRule *rp = (MrphRule *)(rule->get_rule_array());
    GeneralRuleArray.push_back(rule);

    // filename = check_rule_filename(filename);
    if ((fp = fopen(filename.c_str(), "r")) == NULL ) {
        if (param->debug_flag)
            cerr << ";; Cannot open file (" << filename << ") !!" << endl;
	return false;
    }
    if (param->debug_flag)
        cerr << "Reading " << filename;

    LineNo = 1;
    while (!s_feof(fp)) {
	LineNoForError = LineNo;

	body_cell = s_read(fp);
	store_regexpmrphs(&(rp->pre_pattern), car(body_cell));
	store_regexpmrphs(&(rp->self_pattern), car(cdr(body_cell)));
	store_regexpmrphs(&(rp->post_pattern), car(cdr(cdr(body_cell))));

	rp->f = NULL;
	list2feature(cdr(cdr(cdr(body_cell))), &(rp->f));

	if (rule->increase_rulesize() == false) {
	    break;
	}
	rp++;
    }

    if (param->debug_flag)
        cerr << " (" << rule->get_rulesize() << ") done." << endl;
    // free(filename);    
    fclose(fp);
    return true;
}

bool Rules::read_phrase_rule(std::string filename, int mode, int breakmode, int direction, bool lattice_flag) {
    FILE *fp;
    CELL *body_cell;

    GeneralRuleType *rule = new GeneralRuleType(filename.c_str(), lattice_flag ? LatticePhraseRuleType : PhraseRuleType, mode, breakmode, direction);
    PhraseRule *rp = (PhraseRule *)(rule->get_rule_array());
    GeneralRuleArray.push_back(rule);

    // filename = check_rule_filename(filename);
    if ((fp = fopen(filename.c_str(), "r")) == NULL ) {
        if (param->debug_flag)
            cerr << ";; Cannot open file (" << filename << ") !!" << endl;
	return false;
    }
    if (param->debug_flag)
        cerr << "Reading " << filename;

    LineNo = 1;
    while (!s_feof(fp)) {
	LineNoForError = LineNo;

	body_cell = s_read(fp);
	store_regexpphrases(&(rp->pre_pattern), car(body_cell));
	store_regexpphrases(&(rp->self_pattern), car(cdr(body_cell)));
	store_regexpphrases(&(rp->post_pattern), car(cdr(cdr(body_cell))));

	rp->f = NULL;
	list2feature(cdr(cdr(cdr(body_cell))), &(rp->f));

	if (rule->increase_rulesize() == false) {
	    break;
	}
	rp++;
    }

    if (param->debug_flag)
        cerr << " (" << rule->get_rulesize() << ") done." << endl;
    // free(filename);
    fclose(fp);
    return true;
}

parser_data_type check_pattern_type(REGEXPMRPH *mrph) {
    if (check_feature(mrph->f_pattern.fp[0], "BP")) { // *** FIX ME!
	return IS_PHRASE_DATA;
    }
    else {
	return IS_MRPH_DATA;
    }
}

bool make_unary_cfg_rule(GeneralRuleType *rule, CFGRule *cfg_rule) {
    CFGRule *next_rule = cfg_rule + 1;

    if (rule->increase_rulesize() == false) {
	return false;
    }

    next_rule->pre_pattern = cfg_rule->pre_pattern;
    next_rule->post_pattern = cfg_rule->post_pattern;
    next_rule->pre_pattern_type = cfg_rule->pre_pattern_type;
    next_rule->post_pattern_type = cfg_rule->post_pattern_type;
    next_rule->f = cfg_rule->f;

    cfg_rule->post_pattern = NULL;
    cfg_rule->post_pattern_type = NONE;

    return true;
}

bool Rules::read_cfg_rule(std::string filename, int mode) {
    FILE *fp;
    CELL *body_cell;
    CFGRule *orig_rp;
    char buf[DATA_LEN];
    int i, mrph_size, orig_rule_num, sub_rule_num;

    GeneralRuleType *rule = new GeneralRuleType(filename.c_str(), CFGRuleType, mode);
    CFGRule *rp = (CFGRule *)(rule->get_rule_array());
    GeneralRuleArray.push_back(rule);

    // filename = check_rule_filename(filename);
    if ((fp = fopen(filename.c_str(), "r")) == NULL ) {
        if (param->debug_flag)
            cerr << ";; Cannot open file (" << filename << ") !!" << endl;
	return false;
    }
    if (param->debug_flag)
        cerr << "Reading " << filename;
    
    LineNo = 1;
    while (!s_feof(fp)) {
	LineNoForError = LineNo;
        rp->f = NULL;

	body_cell = s_read(fp);

	/* 要素列の読込 */
	store_regexpmrphs(&(rp->pre_pattern), car(body_cell));

	if (rp->pre_pattern->mrphsize < 1) { /* 1つ未満 */
	    cerr << ";;; CFG rule format error." << endl;
	    return false;
	}
	else if (rp->pre_pattern->mrphsize == 1) { /* unary rule */
	    rp->pre_pattern_type = check_pattern_type(rp->pre_pattern->mrph);
	    rp->post_pattern = NULL;
	    rp->post_pattern_type = NONE;
	}
	else {
	    mrph_size = rp->pre_pattern->mrphsize;
	    rp->pre_pattern->mrphsize = 1;
	    rp->pre_pattern_type = check_pattern_type(rp->pre_pattern->mrph);
	    /* pre_patternに入れた2つ目の要素をpost_patternに移動 */
	    rp->post_pattern = regexpmrphs_alloc();
	    rp->post_pattern->mrph = rp->pre_pattern->mrph + 1;
	    rp->post_pattern->mrphsize = 1;
	    rp->post_pattern_type = check_pattern_type(rp->post_pattern->mrph);

	    /* 3個以上の形態素からなる場合は、2項ルールに分解 */
	    if (mrph_size > 2) {
		i = 1;
		sub_rule_num = 1;
		orig_rule_num = rule->get_rulesize();
		i++;
		sprintf(buf, "NT-RULE%d-%d", orig_rule_num, sub_rule_num++);
		copy_cfeature(&(rp->f), buf); /* 読み込んだルールを2項にし、non terminal featureを付与 */
		if (rule->increase_rulesize() == false) {
		    break;
		}
		if (rp->post_pattern->mrph->ast_flag) {
		    if (make_unary_cfg_rule(rule, rp) == false) {
			break;
		    }
		    rp++;
		}
		orig_rp = rp;
		rp++;

		for (; i < mrph_size; i++) {
		    rp->pre_pattern = regexpmrphs_alloc();
		    rp->pre_pattern->mrphsize = 1;
		    rp->pre_pattern->mrph = regexpmrph_alloc(1);
		    copy_cfeature(&(rp->pre_pattern->mrph->f_pattern.fp[0]), buf);
		    rp->pre_pattern_type = IS_PHRASE_DATA;

		    rp->post_pattern = regexpmrphs_alloc();
		    rp->post_pattern->mrphsize = 1;
		    rp->post_pattern->mrph = orig_rp->pre_pattern->mrph + i; /* 2〜 */
		    rp->post_pattern_type = IS_MRPH_DATA;

		    if (i != mrph_size - 1) { /* 最終回以外 */
			if (!rp->post_pattern->mrph->ast_flag) { /* *でないときは次のsub_rule */
			    sprintf(buf, "NT-RULE%d-%d", orig_rule_num, sub_rule_num++);
			}
			copy_cfeature(&(rp->f), buf);
			if (rule->increase_rulesize() == false) {
			    break;
			}
			rp++;
		    }
		}
	    }
	}

	list2feature(cdr(body_cell), &(rp->f));

	if (rule->increase_rulesize() == false) {
	    break;
	}
	rp++;
    }

    if (param->debug_flag)
        cerr << " (" << rule->get_rulesize() << ") done." << endl;
    // free(filename);
    fclose(fp);
    return true;
}

bool Rules::read_dpnd_rule(std::string filename) {
    int i;
    FILE *fp;
    CELL *body_cell, *loop_cell;

    GeneralRuleType *rule = new GeneralRuleType(filename.c_str(), DpndRuleType);
    DpndRule *rp = (DpndRule *)(rule->get_rule_array());
    GeneralRuleArray.push_back(rule);

    // filename = check_rule_filename(filename);
    if ((fp = fopen(filename.c_str(), "r")) == NULL ) {
        if (param->debug_flag)
            cerr << ";; Cannot open file (" << filename << ") !!" << endl;
	return false;
    }
    if (param->debug_flag)
        cerr << "Reading " << filename;

    LineNo = 1;
    while (!s_feof(fp)) {
	LineNoForError = LineNo;

	body_cell = s_read(fp);

	list2feature_pattern(&(rp->dependant), car(body_cell));	
	loop_cell = car(cdr(body_cell));
	i = 0;
	while (!Null(car(loop_cell))) {
	    list2feature_pattern(&(rp->governor[i]), car(car(loop_cell)));
	    rp->dpnd_type[i] = *(_Atom(car(cdr(car(loop_cell)))));

	    loop_cell = cdr(loop_cell);
	    if (++i == DpndRule_G_MAX) {
		cerr << ";;; Too many Governors in a DpndRule." << endl;
		return false;
	    }
	}
	rp->dpnd_type[i] = 0;	/* dpnd_type[i] != 0 がgovernorのある印 */

	list2feature_pattern(&(rp->barrier), car(cdr(cdr(body_cell))));
	rp->preference = atoi((char *)_Atom(car(cdr(cdr(cdr(body_cell))))));

	/* 一意に決定するかどうか */
	if (!Null(car(cdr(cdr(cdr(cdr(body_cell)))))) && 
	    !strcmp((char *)_Atom(car(cdr(cdr(cdr(cdr(body_cell)))))), "U")) {
	    rp->decide = 1;
	}
	else {
	    rp->decide = 0;
	}

	if (rule->increase_rulesize() == false) {
	    break;
	}
	rp++;
    }

    if (param->debug_flag)
        cerr << " (" << rule->get_rulesize() << ") done." << endl;
    // free(filename);
    fclose(fp);
    return true;
}


int break_feature(FEATURE *fp) {
    while (fp) {
	if (!strcmp(fp->cp, "&break:normal")) 
	    return RLOOP_BREAK_NORMAL;
	else if (!strcmp(fp->cp, "&break:jump")) 
	    return RLOOP_BREAK_JUMP;
	else if (!strncmp(fp->cp, "&break", strlen("&break")))
	    return RLOOP_BREAK_NORMAL;
	fp = fp->next;
    }
    return RLOOP_BREAK_NONE;
}

// ある範囲(文全体,文節内など)に対して形態素のマッチングを行う
bool GeneralRuleType::assign_mrph_feature(std::vector<Morpheme *>::iterator s_m_ptr, int m_length,
					  int temp_assign_flag) {
    if (type != MorphRuleType) {
	return false;
    }

    int i, j, k, match_length, feature_break_mode;
    MrphRule *s_r_ptr = (MrphRule *)RuleArray, *r_ptr;
    int r_size = CurRuleSize;
    std::vector<Morpheme *>::iterator m_ptr;

    /* 逆方向に適用する場合はデータのおしりをさしておく必要がある */
    if (direction == RtoL)
	s_m_ptr += m_length - 1;
    
    /* MRM
       	1.self_patternの先頭の形態素位置
	  2.ルール
	    3.self_patternの末尾の形態素位置
	の順にループが回る (3のループはregexpmrphrule_matchの中)
	
	breakmode == RLOOP_BREAK_NORMAL
	    2のレベルでbreakする
	breakmode == RLOOP_BREAK_JUMP
	    2のレベルでbreakし，self_pattern長だけ1のループを進める
     */

    if (mode == RLOOP_MRM) {
	for (i = 0; i < m_length; i++) {
	    r_ptr = s_r_ptr;
	    m_ptr = s_m_ptr + (i * direction);
	    for (j = 0; j < r_size; j++, r_ptr++) {
		if ((match_length = 
		     regexpmrphrule_match(r_ptr, m_ptr, 
					  direction == LtoR ? i : m_length - i - 1, 
					  direction == LtoR ? m_length - i : i + 1)) != -1) {
		    for (k = 0; k < match_length; k++)
			assign_feature((*(s_m_ptr + i * direction + k))->get_fp(), 
				       &(r_ptr->f), *(s_m_ptr + i * direction), k, match_length - k, temp_assign_flag);
		    feature_break_mode = break_feature(r_ptr->f);
		    if (breakmode == RLOOP_BREAK_NORMAL ||
			feature_break_mode == RLOOP_BREAK_NORMAL) {
			break;
		    } else if (breakmode == RLOOP_BREAK_JUMP ||
			       feature_break_mode == RLOOP_BREAK_JUMP) {
			i += match_length - 1;
			break;
		    }
		}
	    }
	}
    }

    /* RMM
       	1.ルール
	  2.self_patternの先頭の形態素位置
	    3.self_patternの末尾の形態素位置
	の順にループが回る (3のループはregexpmrphrule_matchの中)
	
	breakmode == RLOOP_BREAK_NORMAL||RLOOP_BREAK_JUMP
	    2のレベルでbreakする (※この使い方は考えにくいが)
    */

    else if (mode == RLOOP_RMM) {
	r_ptr = s_r_ptr;
	for (j = 0; j < r_size; j++, r_ptr++) {
	    feature_break_mode = break_feature(r_ptr->f);
	    for (i = 0; i < m_length; i++) {
		m_ptr = s_m_ptr + (i * direction);
		if ((match_length = 
		     regexpmrphrule_match(r_ptr, m_ptr, 
					  direction == LtoR ? i : m_length - i - 1, 
					  direction == LtoR ? m_length - i : i + 1)) != -1) {
		    for (k = 0; k < match_length; k++)
			assign_feature((*(s_m_ptr + i * direction + k))->get_fp(), 
				       &(r_ptr->f), *(s_m_ptr + i * direction), k, match_length - k, temp_assign_flag);
		    if (breakmode == RLOOP_BREAK_NORMAL ||
			breakmode == RLOOP_BREAK_JUMP ||
			feature_break_mode == RLOOP_BREAK_NORMAL ||
			feature_break_mode == RLOOP_BREAK_JUMP) {
			break;
		    }
		}
	    }
	}
    }

    return true;
}

// ある範囲(文全体,文節内など)に対してフレーズのマッチングを行う
bool GeneralRuleType::assign_phrase_feature(std::vector<Phrase *>::iterator s_b_ptr, int b_length,
					    int also_assign_flag, int temp_assign_flag) {
    if (type != PhraseRuleType) {
	return false;
    }

    int i, j, k, match_length, feature_break_mode;
    PhraseRule *s_r_ptr = (PhraseRule *)RuleArray, *r_ptr;
    int r_size = CurRuleSize;
    std::vector<Phrase *>::iterator b_ptr;

    /* 逆方向に適用する場合はデータのおしりをさしておく必要がある */
    if (direction == RtoL)
	s_b_ptr += b_length - 1;
    
    /* MRM
       	1.self_patternの先頭の文節位置
	  2.ルール
	    3.self_patternの末尾の文節位置
	の順にループが回る (3のループはregexpbnstrule_matchの中)
	
	breakmode == RLOOP_BREAK_NORMAL
	    2のレベルでbreakする
	breakmode == RLOOP_BREAK_JUMP
	    2のレベルでbreakし，self_pattern長だけ1のループを進める
     */

    if (mode == RLOOP_MRM) {
	for (i = 0; i < b_length; i++) {
	    r_ptr = s_r_ptr;
	    b_ptr = s_b_ptr + (i * direction);
	    for (j = 0; j < r_size; j++, r_ptr++) {
		if ((match_length = 
		     regexpphraserule_match(r_ptr, b_ptr, 
					    direction == LtoR ? i : b_length - i - 1, 
					    direction == LtoR ? b_length - i : i + 1)) != -1) {
		    for (k = 0; k < match_length; k++) {
			assign_feature((*(s_b_ptr + i * direction + k))->get_fp(), 
				       &(r_ptr->f), *(s_b_ptr + i * direction), k, match_length - k, temp_assign_flag);
			// if (also_assign_flag) { /* 属する文節にも付与する場合 */
			//     assign_feature(&((s_b_ptr+i*direction+k)->b_ptr->f), 
			// 		   &(r_ptr->f), s_b_ptr+i*direction, k, match_length - k, temp_assign_flag);
			// }
		    }
		    feature_break_mode = break_feature(r_ptr->f);
		    if (breakmode == RLOOP_BREAK_NORMAL ||
			feature_break_mode == RLOOP_BREAK_NORMAL) {
			break;
		    } else if (breakmode == RLOOP_BREAK_JUMP ||
			       feature_break_mode == RLOOP_BREAK_JUMP) {
			i += match_length - 1;
			break;
		    }
		}
	    }
	}
    }

    /* RMM
       	1.ルール
	  2.self_patternの先頭の文節位置
	    3.self_patternの末尾の文節位置
	の順にループが回る (3のループはregexpbnstrule_matchの中)
	
	breakmode == RLOOP_BREAK_NORMAL||RLOOP_BREAK_JUMP
	    2のレベルでbreakする (※この使い方は考えにくいが)
    */

    else if (mode == RLOOP_RMM) {
	r_ptr = s_r_ptr;
	for (j = 0; j < r_size; j++, r_ptr++) {
	    feature_break_mode = break_feature(r_ptr->f);
	    for (i = 0; i < b_length; i++) {
		b_ptr = s_b_ptr + (i * direction);
		if ((match_length = 
		     regexpphraserule_match(r_ptr, b_ptr, 
					    direction == LtoR ? i : b_length - i -1, 
					    direction == LtoR ? b_length - i : i + 1)) != -1) {
		    for (k = 0; k < match_length; k++) {
			assign_feature((*(s_b_ptr + i * direction + k))->get_fp(), 
				       &(r_ptr->f), *(s_b_ptr + i * direction), k, match_length - k, temp_assign_flag);
			// if (also_assign_flag) { /* 属する文節にも付与する場合 */
			//     assign_feature(&((s_b_ptr+i*direction+k)->b_ptr->f), 
			// 		   &(r_ptr->f), s_b_ptr+i*direction, k, match_length - k, temp_assign_flag);
			// }
		    }
		    if (breakmode == RLOOP_BREAK_NORMAL ||
			breakmode == RLOOP_BREAK_JUMP ||
			feature_break_mode == RLOOP_BREAK_NORMAL ||
			feature_break_mode == RLOOP_BREAK_JUMP) {
			break;
		    }
		}
	    }
	}
    }
    return true;
}

// 形態素ラティスに対して形態素のマッチングを行う
bool GeneralRuleType::assign_mrph_feature_for_lattice(CKY *cky, int temp_assign_flag) {
    if (type != LatticeMorphRuleType) {
	return false;
    }

    int i, j, k, start, end, match_length, feature_break_mode;
    MrphRule *s_r_ptr = (MrphRule *)RuleArray, *r_ptr;
    int r_size = CurRuleSize;
    int token_num = cky->get_token_num();

    /* MRM
       	1.self_patternの先頭の形態素位置
	  2.ルール
	    3.self_patternの末尾の形態素位置
	の順にループが回る (3のループはregexpmrphrule_matchの中)
	
	breakmode == RLOOP_BREAK_NORMAL
	    2のレベルでbreakする
	breakmode == RLOOP_BREAK_JUMP
	    2のレベルでbreakし，self_pattern長だけ1のループを進める
     */

    if (mode == RLOOP_MRM) {
	for (i = 0; i < token_num; i++) {
	    k = direction == LtoR ? i : token_num - i - 1;
	    while (k >= 0 && k < token_num) {
		start = direction == LtoR ? i : k;
		end = direction == LtoR ? k : token_num - i - 1;
		CKY_cell *cky_ptr = cky->get_base_cky_cell(start, end);
		while (cky_ptr && cky_ptr->exist()) { /* その位置から始まるすべての形態素について */
		    if (cky_ptr->get_mrph_ptr()) {
			r_ptr = s_r_ptr;
			for (j = 0; j < r_size; j++, r_ptr++) {
			    if (r_ptr->self_pattern && r_ptr->self_pattern->mrphsize > 1) {
				cerr << ";;; self pattern consists of more than one morpheme. (rule: " << j << ")" << endl;
				continue;
			    }
			    if ((match_length = 
				 regexpmrphrule_lattice_match(cky, r_ptr, cky_ptr->get_mrph_ptr(), start, start, token_num - start)) != -1) {
				assign_feature(cky_ptr->get_mrph_ptr()->get_fp(), 
					       &(r_ptr->f), cky_ptr->get_mrph_ptr(), 0, 1, temp_assign_flag);
				feature_break_mode = break_feature(r_ptr->f);
				if (breakmode == RLOOP_BREAK_NORMAL ||
				    feature_break_mode == RLOOP_BREAK_NORMAL) {
				    break;
				} else if (breakmode == RLOOP_BREAK_JUMP ||
					   feature_break_mode == RLOOP_BREAK_JUMP) {
				    break;
				}
			    }
			}
		    }
		    cky_ptr = cky_ptr->get_next();
		}
		k += direction;
	    }
	}
    }

    /* RMM
       	1.ルール
	  2.self_patternの先頭の形態素位置
	    3.self_patternの末尾の形態素位置
	の順にループが回る (3のループはregexpmrphrule_matchの中)
	
	breakmode == RLOOP_BREAK_NORMAL||RLOOP_BREAK_JUMP
	    2のレベルでbreakする (※この使い方は考えにくいが)
    */

    else if (mode == RLOOP_RMM) {
	r_ptr = s_r_ptr;
	for (j = 0; j < r_size; j++, r_ptr++) {
	    if (r_ptr->self_pattern && r_ptr->self_pattern->mrphsize > 1) {
		cerr << ";;; self pattern consists of more than one morpheme. (rule: " << j << ")" << endl;
		continue;
	    }
	    feature_break_mode = break_feature(r_ptr->f);
	    for (i = 0; i < token_num; i++) {
		k = direction == LtoR ? i : token_num - i - 1;
		while (k >= 0 && k < token_num) {
		    start = direction == LtoR ? i : k;
		    end = direction == LtoR ? k : token_num - i - 1;
		    CKY_cell *cky_ptr = cky->get_base_cky_cell(start, end);
		    while (cky_ptr && cky_ptr->exist()) { /* その位置から始まるすべての形態素について */
			if (cky_ptr->get_mrph_ptr() && 
			    (match_length = 
			     regexpmrphrule_lattice_match(cky, r_ptr, cky_ptr->get_mrph_ptr(), start, start, token_num - start)) != -1) {
			    assign_feature(cky_ptr->get_mrph_ptr()->get_fp(), 
					   &(r_ptr->f), cky_ptr->get_mrph_ptr(), 0, 1, temp_assign_flag);
			    if (breakmode == RLOOP_BREAK_NORMAL ||
				breakmode == RLOOP_BREAK_JUMP ||
				feature_break_mode == RLOOP_BREAK_NORMAL ||
				feature_break_mode == RLOOP_BREAK_JUMP) {
				break;
			    }
			}
			cky_ptr = cky_ptr->get_next();
		    }
		    k += direction;
		}
	    }
	}
    }
    return true;
}

// ある範囲(文全体,文節内など)に対してフレーズのマッチングを行う
bool GeneralRuleType::assign_phrase_feature_for_lattice(CKY *cky, int also_assign_flag, int temp_assign_flag) {
    if (type != LatticePhraseRuleType) {
	return false;
    }

    int i, j, k, start, end, match_length, feature_break_mode;
    PhraseRule *s_r_ptr = (PhraseRule *)RuleArray, *r_ptr;
    int r_size = CurRuleSize;
    int token_num = cky->get_token_num();

    /* MRM
       	1.self_patternの先頭の文節位置
	  2.ルール
	    3.self_patternの末尾の文節位置
	の順にループが回る (3のループはregexpbnstrule_matchの中)
	
	breakmode == RLOOP_BREAK_NORMAL
	    2のレベルでbreakする
	breakmode == RLOOP_BREAK_JUMP
	    2のレベルでbreakし，self_pattern長だけ1のループを進める
     */

    if (mode == RLOOP_MRM) {
	for (i = 0; i < token_num; i++) {
	    k = direction == LtoR ? i : token_num - i - 1;
	    while (k >= 0 && k < token_num) {
		start = direction == LtoR ? i : k;
		end = direction == LtoR ? k : token_num - i - 1;
		CKY_cell *cky_ptr = cky->get_base_cky_cell(start, end);
		while (cky_ptr && cky_ptr->exist()) { /* その位置から始まるすべての基本句について */
		    if (cky_ptr->get_bp_ptr()) {
			r_ptr = s_r_ptr;
			for (j = 0; j < r_size; j++, r_ptr++) {
			    if (r_ptr->self_pattern && r_ptr->self_pattern->bnstsize > 1) {
				cerr << ";;; self pattern consists of more than one phrase. (rule: " << j << ")" << endl;
				continue;
			    }
			    if ((match_length = 
				 regexpphraserule_lattice_match(cky, r_ptr, cky_ptr->get_bp_ptr(), start, start, token_num - start)) != -1) {
				assign_feature(cky_ptr->get_bp_ptr()->get_fp(), 
					       &(r_ptr->f), cky_ptr->get_bp_ptr(), 0, 1, temp_assign_flag);
				feature_break_mode = break_feature(r_ptr->f);
				if (breakmode == RLOOP_BREAK_NORMAL ||
				    feature_break_mode == RLOOP_BREAK_NORMAL) {
				    break;
				} else if (breakmode == RLOOP_BREAK_JUMP ||
					   feature_break_mode == RLOOP_BREAK_JUMP) {
				    break;
				}
			    }
			}
		    }
		    cky_ptr = cky_ptr->get_next();
		}
		k += direction;
	    }
	}
    }

    /* RMM
       	1.ルール
	  2.self_patternの先頭の文節位置
	    3.self_patternの末尾の文節位置
	の順にループが回る (3のループはregexpbnstrule_matchの中)
	
	breakmode == RLOOP_BREAK_NORMAL||RLOOP_BREAK_JUMP
	    2のレベルでbreakする (※この使い方は考えにくいが)
    */

    else if (mode == RLOOP_RMM) {
	r_ptr = s_r_ptr;
	for (j = 0; j < r_size; j++, r_ptr++) {
	    if (r_ptr->self_pattern && r_ptr->self_pattern->bnstsize > 1) {
		cerr << ";;; self pattern consists of more than one phrase. (rule: " << j << ")" << endl;
		continue;
	    }
	    feature_break_mode = break_feature(r_ptr->f);
	    for (i = 0; i < token_num; i++) {
		k = direction == LtoR ? i : token_num - i - 1;
		while (k >= 0 && k < token_num) {
		    start = direction == LtoR ? i : k;
		    end = direction == LtoR ? k : token_num - i - 1;
		    CKY_cell *cky_ptr = cky->get_base_cky_cell(start, end);
		    while (cky_ptr && cky_ptr->exist()) { /* その位置から始まるすべての基本句について */
			if (cky_ptr->get_bp_ptr() && 
			    (match_length = 
			     regexpphraserule_lattice_match(cky, r_ptr, cky_ptr->get_bp_ptr(), start, start, token_num - start)) != -1) {
			    assign_feature(cky_ptr->get_bp_ptr()->get_fp(), 
					   &(r_ptr->f), cky_ptr->get_bp_ptr(), 0, 1, temp_assign_flag);
			    if (breakmode == RLOOP_BREAK_NORMAL ||
				breakmode == RLOOP_BREAK_JUMP ||
				feature_break_mode == RLOOP_BREAK_NORMAL ||
				feature_break_mode == RLOOP_BREAK_JUMP) {
				break;
			    }
			}
			cky_ptr = cky_ptr->get_next();
		    }
		    k += direction;
		}
	    }
	}
    }
    return true;
}

bool GeneralRuleType::check_possibility_and_make_bp(CKY_cell *cky_ptr, CKY_cell *left_ptr, CKY_cell *right_ptr, Rules *rules, CaseFrames *caseframes) {
    if (type != CFGRuleType) {
	return false;
    }

    bool flag = false;
    CFGRule *r_ptr = (CFGRule *)RuleArray;
    for (int i = 0; i < CurRuleSize; i++, r_ptr++) {
	// unary cfg rule
	if (r_ptr->post_pattern == NULL && right_ptr == NULL) {
	    if ((left_ptr->get_bp_ptr() && r_ptr->pre_pattern_type == IS_PHRASE_DATA && /* bp (non terminal) */
		 feature_pattern_match(&(r_ptr->pre_pattern->mrph->f_pattern), 
				       left_ptr->get_bp_ptr()->get_f(), NULL, left_ptr->get_bp_ptr())) || 
		(left_ptr->get_mrph_ptr() && r_ptr->pre_pattern_type == IS_MRPH_DATA && /* mrph (自立 -> bp) */
		 regexpmrphs_match(r_ptr->pre_pattern->mrph, 
				   r_ptr->pre_pattern->mrphsize, /* 1 */
				   left_ptr->get_mrph_ptr(), 
				   1, 
				   BW_MATCHING, 
				   ALL_MATCHING, 
				   SHORT_MATCHING) != -1)) {
		if (left_ptr->set(left_ptr, NULL, r_ptr->f, rules, caseframes)) {
		    flag = true;
		}
	    }
	}
	// binary cfg rule
	else if (right_ptr && 
		 ((left_ptr->get_mrph_ptr() && right_ptr->get_mrph_ptr() && /* bp <- morpheme + morpheme */
		   r_ptr->pre_pattern_type == IS_MRPH_DATA && r_ptr->post_pattern_type == IS_MRPH_DATA && 
		   regexpmrphs_match(r_ptr->pre_pattern->mrph, 
				     r_ptr->pre_pattern->mrphsize, /* 1 */
				     left_ptr->get_mrph_ptr(), 
				     1, 
				     BW_MATCHING, 
				     ALL_MATCHING, 
				     SHORT_MATCHING) != -1 && 
		   regexpmrphs_match(r_ptr->post_pattern->mrph,  
				     r_ptr->post_pattern->mrphsize, /* 1 */
				     right_ptr->get_mrph_ptr(), 
				     1, 
				     BW_MATCHING, 
				     ALL_MATCHING, 
				     SHORT_MATCHING) != -1) || 
		  (left_ptr->get_bp_ptr() && right_ptr->get_mrph_ptr() && /* bp <- bp + morpheme */
		   r_ptr->pre_pattern_type == IS_PHRASE_DATA && r_ptr->post_pattern_type == IS_MRPH_DATA && 
		   feature_pattern_match(&(r_ptr->pre_pattern->mrph->f_pattern), 
					 left_ptr->get_bp_ptr()->get_f(), NULL, left_ptr->get_bp_ptr()) && 
		   regexpmrphs_match(r_ptr->post_pattern->mrph, 
				     r_ptr->post_pattern->mrphsize, /* 1 */
				     right_ptr->get_mrph_ptr(), 
				     1, 
				     BW_MATCHING, 
				     ALL_MATCHING, 
				     SHORT_MATCHING) != -1) || 
		  (right_ptr->get_bp_ptr() && left_ptr->get_mrph_ptr() && /* bp <- morpheme + bp */
		   r_ptr->post_pattern_type == IS_PHRASE_DATA && r_ptr->pre_pattern_type == IS_MRPH_DATA && 
		   feature_pattern_match(&(r_ptr->post_pattern->mrph->f_pattern), 
					 right_ptr->get_bp_ptr()->get_f(), NULL, right_ptr->get_bp_ptr()) && 
		   regexpmrphs_match(r_ptr->pre_pattern->mrph, 
				     r_ptr->pre_pattern->mrphsize, /* 1 */
				     left_ptr->get_mrph_ptr(), 
				     1, 
				     BW_MATCHING, 
				     ALL_MATCHING, 
				     SHORT_MATCHING) != -1))) {
	    if (cky_ptr->set(left_ptr, right_ptr, r_ptr->f, rules, caseframes)) {
		flag = true;
	    }
	}
    }

    if (!cky_ptr->exist()) {
	// delete cky_ptr;
	return flag;
    }
    else {
	return flag;
    }
}

bool GeneralRuleType::assign_dpnd_rule_to_bp(Phrase *bp_ptr) {
    if (type != DpndRuleType) {
	return false;
    }

    DpndRule *r_ptr = (DpndRule *)RuleArray;
    for (int j = 0; j < CurRuleSize; j++, r_ptr++) {
	if (feature_pattern_match(&(r_ptr->dependant), bp_ptr->get_f(), NULL, bp_ptr) == TRUE) {
	    bp_ptr->set_dpnd_rule(r_ptr); 
	}
    }

    if (bp_ptr->get_dpnd_rule_size() == 0) {
	cerr << ";;; No DpndRule for bp (";
	bp_ptr->get_f()->print(cerr);
	cerr << ")" << endl;
	return false;
    }

    return true;
}

char Rules::match_dpnd_rule(Phrase *k_ptr, Phrase *u_ptr) {
    for (vector<DpndRule *>::iterator rule_it = k_ptr->get_dpnd_rule().begin(); rule_it != k_ptr->get_dpnd_rule().end(); rule_it++) {
	for (int k = 0; (*rule_it)->dpnd_type[k]; k++) {
	    if (feature_pattern_match(&((*rule_it)->governor[k]),
				      u_ptr->get_f(), k_ptr, u_ptr)) {
		return (*rule_it)->dpnd_type[k];
	    }
	}
    }
    return '\0';
}

bool Rules::check_predicate_argument_possibility(Phrase *k_ptr, Phrase *u_ptr) {
    FEATURE *fp = k_ptr->get_f();
    while (fp) {
        if (!strncmp(fp->cp, "ARG-", strlen("ARG-"))) { // k_ptr is an argument
            if (check_feature(u_ptr->get_f(), "PRED")) // then, u_ptr should be a predicate
                return true;
            else
                return false;
        }
        fp = fp->next;
    }
    return true;
}

char Rules::check_dpnd_possibility(CKY_cell *left_ptr, CKY_cell *right_ptr) {
    char dpnd_type;
    bool gov_eos_flag = false;

    if (left_ptr->get_bp_ptr() && right_ptr->get_bp_ptr()) {
	if (check_feature(right_ptr->get_bp_ptr()->get_f(), "EOS")) // End of sentence
	    gov_eos_flag = true;

	if (check_feature(left_ptr->get_bp_ptr()->get_f(), "BP_TYPE") && 
	    (gov_eos_flag || check_feature(right_ptr->get_bp_ptr()->get_f(), "BP_TYPE"))) {
	    // if left_ptr->bp_ptr does not have a dpnd_rule yet, assign it
	    if (left_ptr->get_bp_ptr()->get_dpnd_rule_size() == 0) {
		if (assign_dpnd_rule_to_bp(left_ptr->get_bp_ptr()) == FALSE) {
		    return '\0';
		}
	    }

	    // check dependency rules
	    if ((param->strict_pa_checking_flag == false || check_predicate_argument_possibility(left_ptr->get_bp_ptr(), right_ptr->get_bp_ptr())) && 
                (dpnd_type = match_dpnd_rule(left_ptr->get_bp_ptr(), right_ptr->get_bp_ptr()))) {
		return dpnd_type;
	    }
	    else if (gov_eos_flag) { // relax for head-final languages
		return 'r';
	    }
	}
    }
    return '\0';
}

// count the number of dependable heads between the dependent and its head
int Rules::count_dpnd_possible_candidates(CKY_cell *dpnd_cky_ptr, CKY_cell *cky_ptr) {
    int count = 0;
    while (cky_ptr && cky_ptr->get_dpnd_type()) {
	if (cky_ptr->get_left() && cky_ptr->get_left()->get_bp_ptr() && 
	    check_dpnd_possibility(dpnd_cky_ptr, cky_ptr->get_left())) {
	    count++;
	}
	count += count_dpnd_possible_candidates(dpnd_cky_ptr, cky_ptr->get_left());
	cky_ptr = cky_ptr->get_right();	
    }
    return count;
}

}
