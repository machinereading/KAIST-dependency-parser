// #include "case_analysis.h"
#include "common.h"
#include "caseframe.h"
#include "cky.h"

namespace Parser {

using std::cout;
using std::cerr;
using std::endl;

double Current_max_score;
int Current_max_num;
vector<AssignedInfo *> Current_max_assigned_info_dat;
vector<AssignedInfo *> Current_max_assigned_info_pat;

extern char *check_feature(FEATURE *fp, const char *fname);
extern void assign_cfeature(FEATURE **fpp, const char *fname, int temp_assign_flag);

bool CaseFrame::_make_data_from_feature_to_pp(Phrase *b_ptr, bool adjacent_flag, bool clausal_modifier_flag) {
    FEATURE *fp;
    CaseSlot *cs_ptr = new CaseSlot(type);
    cs_ptr->set_adjacent(adjacent_flag);

    // arguments of predicate
    if (type == CF_PRED) {
	fp = b_ptr->get_f();
	while (fp) {
	    if (clausal_modifier_flag) { // 被連体修飾詞 (とりあえず用言のときのみ)
		if (!strncmp(fp->cp, "Ｔ解析連格-", strlen("Ｔ解析連格-"))) {
		    cs_ptr->set_pp(fp->cp + strlen("Ｔ解析連格-"));
		}
	    }
	    else { // arguments
		if (!strncmp(fp->cp, "ARG-", strlen("ARG-"))) {
		    cs_ptr->set_pp(fp->cp + strlen("ARG-"));
		}
		else if (!strcmp(fp->cp, "Ｔ必須格")) {
		    cs_ptr->set_oblig(true);
		}
		/*
		  else if (!strcmp(fcp, "Ｔ用言同文節")) { // 「〜を〜に」のとき
		  if (cpm_ptr->pred_b_ptr->num != b_ptr->num) {
		  return FALSE;
		  }
		  }
		*/
	    }
	    fp = fp->next;
	}
	if (cs_ptr->get_pp_num()) {
	    new_assigned(cs_ptr);
	    set_cs(cs_ptr);
	    return true;
	}
    }

    // arguments of noun
    fp = b_ptr->get_f();
    while (fp) {
	if (!check_feature(b_ptr->get_f(), "PRED")) { // && 
	    // !strcmp(fp->cp, "Ｔ名詞項")) {
	    cs_ptr->set_pp("ノ");
	    /*
	    // 条件: 同格ではない 
	    // 連体修飾節の場合はその関係が外の関係
	    if (b_ptr->dpnd_type != 'A' &&
	    (!check_feature(b_ptr->f, "係:連格") || 
	    check_cc_relation(b_ptr->cpm_ptr, cpm_ptr->pred_b_ptr, "外の関係"))) {
	    c_ptr->pp[c_ptr->element_num][(*pp_num)++] = 0;
	    c_ptr->pp_str[c_ptr->element_num] = NULL;
	    }
	    */
	}
	fp = fp->next;
    }
    if (cs_ptr->get_pp_num()) {
	cs_ptr->set_type(CF_NOUN);
	new_assigned(cs_ptr);
	set_cs(cs_ptr);
	return true;
    }
    else {
	delete cs_ptr;
	return false;
    }
}

Phrase *CaseFrame::_make_data_cframe_pp(Phrase *b_ptr, bool adjacent_flag, bool clausal_modifier_flag) {
    // clausal_modifier_flag == false: normal argument
    // clausal_modifier_flag == true : clausal modifiee

    // normal argument
    if (clausal_modifier_flag == false) {
	// FIX ME: 複合辞
	// FIX ME: 係り先をみる場合
	// FIX ME: 「〜の(方)」などの格は「方」の方の格をみる
	;
    }

    // from feature to case
    if (_make_data_from_feature_to_pp(b_ptr, adjacent_flag, clausal_modifier_flag) == true) {
	return b_ptr;
    }
    else {
	return NULL;
    }
}

bool CaseFrame::make_data_cframe_child(Phrase *child_ptr, bool adjacent_flag, int dpnd_cand_num) {
    Phrase *cel_b_ptr;
    if ((cel_b_ptr = _make_data_cframe_pp(child_ptr, adjacent_flag, false))) {
	// 「みかん三個を食べる」 ひとつ前の名詞を格要素とするとき
	// 「みかんを三個食べる」 の場合はそのまま両方格要素になる
	/* if (sp && 
	    check_feature(cel_b_ptr->f, "QUANT") && // quantitiy
	    (check_feature(cel_b_ptr->f, "係:ガ格") || check_feature(cel_b_ptr->f, "係:ヲ格")) && 
	    cel_b_ptr->num > 0 && 
	    (check_feature((sp->tag_data + cel_b_ptr->num - 1)->f, "係:隣") || 
	     check_feature((sp->tag_data + cel_b_ptr->num - 1)->f, "係:同格未格")) && 
	    !check_feature((sp->tag_data + cel_b_ptr->num - 1)->f, "QUANT") && 
	    !check_feature((sp->tag_data + cel_b_ptr->num - 1)->f, "TIME")) {
	    _make_data_cframe_sm(cpm_ptr, sp->tag_data + cel_b_ptr->num - 1);
	    _make_data_cframe_ex(cpm_ptr, sp->tag_data + cel_b_ptr->num - 1);
	    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = sp->tag_data + cel_b_ptr->num - 1;
	    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
	    }
	    else { */
	cel_b_ptr->set_dpnd_cand_num(dpnd_cand_num);
	CaseSlot *cs_ptr = get_last_cs_ptr();
	cs_ptr->set_ex(cel_b_ptr);
	// cs_ptr->set_gex(cel_b_ptr);
	cs_ptr->set_elem_bp(cel_b_ptr);

	/* 格が明示されていないことをマーク
	if (check_feature(cel_b_ptr->f, "係:未格") || 
	    check_feature(cel_b_ptr->f, "係:ノ格") || 
	    cel_b_ptr->inum > 0) {
	    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
	}
	else {
	    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = child_num;
	    } *
	cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
	*/
	return true;
    }
    else {
	return false;
    }
}

// add coordinated case components
bool CaseFrame::add_coordinated_phrases(CKY_cell *cky_ptr) {
    while (cky_ptr) { // skip modifiers
	if (cky_ptr->get_dpnd_type() == 'P') { // || cky_ptr->para_flag
	    break;
	}
	cky_ptr = cky_ptr->get_right();
    }

    if (!cky_ptr) {
	return false;
    }
    // else if (cky_ptr->para_flag) { // parent of <PARA> + <PARA>
    //	return add_coordinated_phrases(cky_ptr->left, add_coordinated_phrases(cky_ptr->right, next));
    // }
    else if (cky_ptr->get_dpnd_type() == 'P') {
	Phrase *bp_ptr = cky_ptr->get_left()->get_bp_ptr();
	CaseSlot *cs_ptr = get_last_cs_ptr();

	cs_ptr->set_ex(bp_ptr);
	// cs_ptr->set_gex(bp_ptr);
	cs_ptr->set_elem_bp(bp_ptr);

	add_coordinated_phrases(cky_ptr->get_right());
	return true;
    }
    else {
	return false;
    }
}

// add a clausal modifiee to cfd
void CaseFrame::make_data_cframe_rentai_simple(Phrase *d_ptr, Phrase *t_ptr, int dpnd_cand_num) {
    // Phrase *cel_b_ptr = _make_data_cframe_pp(d_ptr, false, true);
    // /* 外の関係以外のときは格要素に (外の関係でも形容詞のときは格要素にする) */
    // if (!check_feature(t_ptr->get_f(), "外の関係") || check_feature(d_ptr->get_f(), "PRED:形")) {
    //     pre_cfd->_make_data_cframe_pp(d_ptr, false, true);
    // }
    // /* 一意に外の関係にする */
    // else {
    //     pre_cpm_ptr->cf.pp[pre_cpm_ptr->cf.element_num][0] = pp_hstr_to_code("外の関係");
    //     pre_cpm_ptr->cf.pp[pre_cpm_ptr->cf.element_num][1] = END_M;
    //     pre_cpm_ptr->cf.oblig[pre_cpm_ptr->cf.element_num] = FALSE;
    // }

    d_ptr->set_dpnd_cand_num(dpnd_cand_num);
    CaseSlot *cs_ptr = get_last_cs_ptr();
    cs_ptr->set_ex(t_ptr);
    // cs_ptr->set_gex(t_ptr);
    cs_ptr->set_elem_bp(t_ptr);
}

double CaseFrames::collect_argument_probabilities_generic(CaseFrame *cfd, CKY_cell *cky_ptr) {
    Phrase *head_ptr = cky_ptr->get_bp_ptr();
    double score = 0;

    for (vector<CaseSlot *>::iterator cs_it = cfd->get_cs().begin(); cs_it != cfd->get_cs().end(); cs_it++) {
        if (cfd->get_type() == (*cs_it)->get_type()) // cf_type correspondence
            score += get_case_function_probability_generic((*cs_it)->get_first_elem_bp(), head_ptr);
    }

    return score;
}

bool CaseFrame::collect_arguments(CKY_cell *cky_ptr, CaseFrames *caseframes, Rules *rules) {
    // Phrase *pred_bp_ptr = cky_ptr->get_bp_ptr();
    CKY_cell *orig_cky_ptr = cky_ptr;

    while (cky_ptr && cky_ptr->get_dpnd_type()) {
	// left can be an argument
	if (cky_ptr->get_left() && cky_ptr->get_left()->get_bp_ptr()) { // && cky_ptr->para_flag == 0
	    Phrase *dt_ptr = cky_ptr->get_left()->get_bp_ptr();
	    bool flag = false;

	    // case component
	    if (cky_ptr->get_dpnd_type() != 'P') { // it's not a coordination
		if (make_data_cframe_child(dt_ptr, false, rules->count_dpnd_possible_candidates(cky_ptr->get_left(), cky_ptr->get_right()) + 1)) {
		    add_coordinated_phrases(cky_ptr->get_left());
		    flag = true;
		}

		if ((check_feature(dt_ptr->get_f(), "係:連用") && 
		     (!check_feature(dt_ptr->get_f(), "PRED") || !check_feature(dt_ptr->get_f(), "複合辞"))) || 
		    check_feature(dt_ptr->get_f(), "修飾")) {
		    flag = true;
                }
	    }

	    // clausal modifiee
	    if (0 && check_feature(dt_ptr->get_f(), "係:連格") && 
                cky_ptr->get_left()->get_cfd()) { // should have a case frame
		cky_ptr->get_left()->get_cfd()->make_data_cframe_rentai_simple(dt_ptr, cky_ptr->get_bp_ptr(), rules->count_dpnd_possible_candidates(cky_ptr->get_left(), cky_ptr->get_right()) + 1);
		// add_coordinated_phrases(cky_ptr->get_right(), &(pre_cpm_ptr->elem_b_ptr[pre_cpm_ptr->cf.element_num - 1]->next));

		double orig_cs_score = cky_ptr->get_left()->get_cs_score();
                double orig_score = cky_ptr->get_left()->get_score();
                orig_cky_ptr->add_score(-1 * orig_cs_score);
		caseframes->find_best_cf(cky_ptr->get_left()->get_cfd(), cky_ptr->get_left(), false);
                orig_cky_ptr->add_score(cky_ptr->get_left()->get_cs_score());
                cky_ptr->get_left()->set_cs_score(orig_cs_score);
                cky_ptr->get_left()->set_score(orig_score);
                cky_ptr->get_left()->get_cfd()->unset_cs();
 		flag = true;
            }

	    if (flag == 0) { // && // nominal argument
		// check_feature(t_ptr->f, "体言") && // 複合辞などを飛ばす
		// check_feature(dt_ptr->get_f(), "体言")) {
		if (cky_ptr->get_dpnd_type() != 'P' && 
                    make_data_cframe_child(dt_ptr, false, rules->count_dpnd_possible_candidates(cky_ptr->get_left(), cky_ptr->get_right()) + 1)) {
                    // dt_ptr->next = NULL; // 並列要素格納用(係り側)
                    // t_ptr->next = NULL; // 並列要素格納用(受け側)
                    // add_coordinated_phrases(cky_ptr->get_left(), &(dt_ptr->next));
                    // add_coordinated_phrases(cky_ptr->get_right(), &(t_ptr->next));
                    // one_score += get_noun_co_ex_probability(dt_ptr, t_ptr);
                }
            }
	}
	cky_ptr = cky_ptr->get_right();
    }
    return true;
}

// constructor for input case frame
CaseFrame::CaseFrame(cf_type in_type, CKY_cell *cky_ptr, CaseFrames *caseframes, Rules *rules) {
    type = in_type;
    freq = 0;
    pred_bp = cky_ptr->get_bp_ptr();

    char *cp;
    if ((cp = check_feature(pred_bp->get_f(), "PRED")) && 
        strlen(cp) > 4) // format is "PRED:..."
        cf_id = cp + strlen("PRED:");
    else
        cf_id = pred_bp->get_rep(); // phrase rep

    collect_arguments(cky_ptr, caseframes, rules);
}

// probability of case frame selection P(eat:v2|eat:v) (this == cfp)
double CaseFrame::get_cf_probability(int total_freq) {
    if (total_freq > 0)
        return log((double)freq / total_freq);
    else
        return FREQ0_ASSINED_SCORE;
}

// evaluation of matching between input and case frame
bool CaseFrames::eval_assign_prob(CaseFrame *cfd, CaseFrame *cfp, double score, int closest) {
    bool have_topic_flag = false;

    // probability of case frame
    score += cfp->get_cf_probability(cfd->get_freq()); // cfd's freq is now the total CF freq of the same predicate

    // check input arguments
    for (vector<CaseSlot *>::iterator cs_it = cfd->get_cs().begin(); cs_it != cfd->get_cs().end(); cs_it++) {
	if ((*cs_it)->get_assigned()->get_type() == NIL_ASSIGNED && 
	    (*cs_it)->get_first_elem_bp()->get_token_start() < cfd->get_pred_bp()->get_token_start() && 
	    check_feature((*cs_it)->get_first_elem_bp()->get_f(), "提題")) {
	    have_topic_flag = true;
        }

	// probability of generating clausal modifiee
        if (param->language == LANGUAGE_IS_JAPANESE)
            score += get_np_modifying_probability(cfd, *cs_it);

	if (cfd->get_type() == (*cs_it)->get_type()) { // cf_type == cs_type (CF_PRED OR CF_NOUN)
	    score += (*cs_it)->get_assigned()->get_score();
	}

	/* if (MatchPP(cfd->pp[i][0], "φ") || 
	    MatchPP(cfd->pp[i][0], "修飾")) {
	    ;
            } else */
	// no assignment (correspondence)
	if ((*cs_it)->get_assigned()->get_type() == NIL_ASSIGNED) {
	    // if (CF_MatchPP(cfd->pp[i][0], cfp))
		score += NIL_ASSINED_SCORE;
	    // 対応する格スロットがない場合 => 仮想的に格スロットを作成して割り当て
	    // else
	    //	score += get_case_probability_for_pred(pp_code_to_kstr(cfd->pp[i][0]), cfp, TRUE);
	}
    }

    if (param->language == LANGUAGE_IS_JAPANESE)
        score += get_topic_generating_probability(have_topic_flag, cfd->get_pred_bp());

    // probability of generating cases in case frame
    for (vector<CaseSlot *>::iterator cfp_cs_it = cfp->get_cs().begin(); cfp_cs_it != cfp->get_cs().end(); cfp_cs_it++) {
	if ((*cfp_cs_it)->get_assigned()->get_type() == UNASSIGNED) { // no assignment
	    (*cfp_cs_it)->get_assigned()->set_score(cfp->get_case_probability(*cfp_cs_it, false));
	    score += (*cfp_cs_it)->get_assigned()->get_score();
	}
    }
    // score += get_case_num_probability(cfp, cs.size()); /* number of assignments */

    // double local_score = score;
    /*
    // (入力側)必須格の直前格のマッチを条件とする
    if (closest > -1 && cfd->oblig[closest] == TRUE && list1->flag[closest] == NIL_ASSIGNED) {
	local_score -= HARSH_PENALTY_SCORE;
    }
    */

    if (score > Current_max_score) {
	Current_max_score = score;
	cfd->copy_assigned_all(Current_max_assigned_info_dat);
	cfp->copy_assigned_all(Current_max_assigned_info_pat);
	Current_max_num = 1;
    }
    /*
    if (local_score > Current_max_score) {
	Current_max_list1[0] = *list1;
	Current_max_list2[0] = *list2;
	Current_max_score = local_score;
	Current_pure_score[0] = score;
    }
    else if (local_score == Current_max_score &&
	     Current_max_num < MAX_MATCH_MAX) {
	Current_max_list1[Current_max_num] = *list1;
	Current_max_list2[Current_max_num] = *list2;
	Current_pure_score[Current_max_num] = score;
	Current_max_num++;
    }
    */
    return true;
}

// probability of case P(ga|eat:v2)
// this == cfp
double CaseFrame::get_case_probability(CaseSlot *cs_ptr, bool aflag) {
    double ret;
    // if (freq > 0)
    ret = (double)cs_ptr->get_freq() / freq;
    // else
    //     ret = 0;

    // cerr << ";; (C) P(" << cs_ptr->get_first_pp() << "|" << cf_id << ") = " << ret << endl;
    if (aflag == false) {
	ret = 1 - ret;
    }
    if (ret == 0) {
	ret = UNKNOWN_CASE_SCORE;
    }
    else {
	ret = log(ret);
    }

    /* KNP格解析結果から計算 (cfcases.prob)
    // 用言表記
    // string verb = cf_id;
    // sscanf(cf_id, "%[^0-9]%*d", verb);

    // 用言表記でやった方がよいみたい
    string key = cs_ptr->get_first_pp() + "|" + cf_id;
    // sprintf(key, "%s|%s", pp_code_to_kstr(cfp->pp[as2][0]), verb); // cfp->cf_id);

    string value = db_get(cf_case_db, key);

    double ret;
    if (value.empty()) {
	// cerr << ";; (C) P(" << key << ") = 0";
	if (aflag == false) {
	    ret = 0;
	    // cerr << " (X)";
	}
	else {
	    ret = UNKNOWN_CASE_SCORE;
	    // cerr << " (O)";
	}
	// cerr << endl;
    }
    else {
	ret = atof(value.c_str());
	// cerr << ";; (C) P(" << key << ") = " << ret << endl;
	if (aflag == false) {
	    ret = 1 - ret;
	}
	if (ret == 0) {
	    ret = UNKNOWN_CASE_SCORE;
	}
	else {
	    ret = log(ret);
	}
    }
    */
    return ret;
}

// this == cfp_cs_ptr
double CaseSlot::get_ex_probability_internal(const string &key) {
    double ret;
    if (ex.find(key) != ex.end()) { // key exists
	ret = log((double)ex[key] / freq);
    }
    else {
	ret = FREQ0_ASSINED_SCORE;
    }
    // cerr << ";; (EX) P(" << key << ") = " << ret << endl;
    return ret;
}

// this == cfd
double CaseFrame::get_ex_probability_with_para(CaseSlot *cfd_cs_ptr, CaseFrame *cfp, CaseSlot *cfp_cs_ptr) {
    int count = 0;
    double score = 0;

    map<string, int> ex = cfd_cs_ptr->get_ex();
    for (map<string, int>::iterator it = ex.begin(); it != ex.end(); it++) {
	score += cfp_cs_ptr->get_ex_probability_internal((*it).first);
	count++;
    }

    return score;
    // return score / count; 連体修飾名詞は正規化

    /* 
    bool np_modifying_flag;
    if (cfd->pred_b_ptr->num < tp->num) { // 連体修飾
	np_modifying_flag = true;
    }
    else {
	np_modifying_flag = false;
    }
    */
}

// return expression of case slot
const string &CaseFrames::make_cf_case_string(CaseFrame *cfd, CaseSlot *cfd_cs_ptr, CaseFrame *cfp, CaseSlot *cfp_cs_ptr) {
    if (cfp_cs_ptr) {
	return cfp_cs_ptr->get_first_pp();
    }
    else {
	/*
	// 格スロットがあるのにNIL_ASSIGNED
	if (CF_MatchPP(cfd->pp[as1][0], cfp))
	    return "--";
	// 格フレームがないとき、仮想的に格スロットを作ると考える
	else
	*/
	return cfd_cs_ptr->get_first_pp();
    }
}

// probability of case interpretation
double CaseFrames::get_case_interpret_probability(const string &scase, const string &cfcase, bool ellipsis_flag) {
    string key;
    if (ellipsis_flag) {
	key = scase + "|O:" + cfcase;
    }
    else {
	key = scase + "|C:" + cfcase;
    }

    if (case_prob.find(key) != case_prob.end()) { // key exists
	return log(case_prob[key]);
    }
    else {
	return UNKNOWN_CASE_SCORE;
    }
}

// noun_co probability
double CaseFrames::get_noun_co_probability(string &key) {
    if (!noun_co_prob->is_open()) {
	return 0;
    }

    string value = noun_co_prob->get(key);
    if (value.size() > 0) { // key exists
	return log(atof(value.c_str()));
    }
    else {
	return FREQ0_ASSINED_SCORE;
    }
}

double CaseFrames::collect_vp_probs(CKY_cell *cky_ptr, Rules *rules) {
    int renyou_modifying_num = 0;
    int adverb_modifying_num = 0;
    double score = 0;
    Phrase *g_ptr = cky_ptr->get_bp_ptr(); // governer

    // for each child
    while (cky_ptr && cky_ptr->get_dpnd_type()) {
	if (cky_ptr->get_left() && cky_ptr->get_left()->get_bp_ptr()) {
	    Phrase *d_ptr = cky_ptr->get_left()->get_bp_ptr(); // dependent
	    d_ptr->set_dpnd_cand_num(rules->count_dpnd_possible_candidates(cky_ptr->get_left(), cky_ptr->get_right()) + 1);
	    if (cky_ptr->get_dpnd_type() != 'P') {
		// !(cky_ptr->left->i == cky_ptr->left->j && /* 「〜に」が格要素なので除外 */
		//  check_feature(d_ptr->f, "ID:（〜を）〜に"))) {
		if (check_feature(d_ptr->get_f(), "係:連格")) {
		    score += calc_vp_modifying_probability(d_ptr, g_ptr, true, false);
		}
		// modifying predicate
		else if (check_feature(d_ptr->get_f(), "PRED")) { // (check_feature(d_ptr->get_f(), "係:連用") && check_feature(d_ptr->get_f(), "PRED")) {
		    // !check_feature(d_ptr->f, "複合辞")) {
                    score += get_pred_co_probability(d_ptr, g_ptr); // predicate generating prob
		    score += calc_vp_modifying_probability(d_ptr, g_ptr, false, false);
		    renyou_modifying_num++;
		}
		// modifying adverb
		else if ((check_feature(d_ptr->get_f(), "係:連用") && !check_feature(d_ptr->get_f(), "PRED"))) { 
		    // check_feature(d_ptr->f, "修飾")) {
		    score += calc_vp_modifying_probability(d_ptr, g_ptr, false, true);
		    adverb_modifying_num++;
		}
	    }
	}
	cky_ptr = cky_ptr->get_right();
    }
    return score;
}

double CaseFrames::calc_vp_modifying_probability(Phrase *dp, Phrase *gp, bool np_modifying_flag, bool adverb_flag) {
    if (param->language == LANGUAGE_IS_JAPANESE) {
        // the number of punctuations
        bool touten_flag = check_feature(dp->get_f(), "読点") ? true : false;

        // check dependency possibilities
        int dist = dp->get_dpnd_cand_num();
        if (dist <= 0) {
            return UNKNOWN_CASE_SCORE;
        }
        else if (dist > 1) {
            dist = 2;
        }

        return get_punctuation_generating_probability(CF_PRED, np_modifying_flag, true, adverb_flag, touten_flag, 
                                                      dist, false, 0, false);
    }
    else {
        return get_case_function_probability_generic(dp, gp);
    }
}

// probability of generating punctuation
double CaseFrames::get_punctuation_generating_probability(cf_type type, bool np_modifying_flag, 
							  bool vp_modifying_flag, bool adverb_flag, 
							  bool touten_flag, int dist, 
							  bool closest_pred_flag, int topic_score, bool wa_flag) {
    std::ostringstream oss;
    oss << dist;

    string key = touten_flag ? "1" : "0";
    if (np_modifying_flag) {
	if (type == CF_PRED) {
	    key += "|P連格:" + oss.str(); // dist
	}
	else {
	    key += "|P連体:" + oss.str(); // dist
	    key += ",1"; // closest_ok
	}
    }
    else if (vp_modifying_flag) {
	if (adverb_flag) {
	    key += "|P副詞:" + oss.str(); // dist
	}
	else {
	    key += "|P連用:" + oss.str(); // dist
	}
	key += ",1"; // closest_pred_flag
    }
    else {
	key += "|P:" + oss.str(); // dist
	key += ",";

	key += closest_pred_flag ? "1" : "0";
	key += ",";

	oss.str(""); // initialization
	oss << topic_score;
	key += oss.str(); // topic_score
	key += ",";

	key += wa_flag ? "1" : "0";
    }

    // cerr << ";; (R) P(" << key << ") = ";
    if (case_prob.find(key) != case_prob.end()) { // key exists
	// cerr << case_prob[key] << ", log(P) = " << log(case_prob[key]) << endl;
	return log(case_prob[key]);
    }
    else {
	// cerr << "0" << endl;
	return UNKNOWN_CASE_SCORE;
    }
}

// 「は」 generating probability
double CaseFrames::get_wa_generating_probability(bool np_modifying_flag, bool touten_flag, int dist, 
                                                 bool closest_pred_flag, int topic_score, bool wa_flag, 
                                                 bool negation_flag, char *vtype)
{
    std::ostringstream oss;
    oss << dist;

    if (np_modifying_flag || !vtype) {
        return 0;
    }
    else {
        string key = wa_flag ? "1" : "0";
	key += "|T:" + oss.str(); // dist
	key += ",";

	key += closest_pred_flag ? "1" : "0";
	key += ",";

	oss.str(""); // initialization
	oss << topic_score;
	key += oss.str(); // topic_score
	key += ",";

	key += touten_flag ? "1" : "0";
	key += ",";

	key += negation_flag ? "1" : "0";
	key += ",";

        key += vtype;

        // cerr << ";; (T) P(" << key << ") = "; // for debug
        if (case_prob.find(key) != case_prob.end()) { // key exists
            // cerr << case_prob[key] << ", log(P) = " << log(case_prob[key]) << endl;
            return log(case_prob[key]);
        }
        else {
            // cerr << "0" << endl; // for debug
            return UNKNOWN_CASE_SCORE;
        }
    }
}

// topic generating probability
double CaseFrames::get_topic_generating_probability(bool have_topic_flag, Phrase *gp)
{
    int topic_score = 0;
    char *cp;
    std::ostringstream oss;

    // 提題スコア
    if ((cp = check_feature(gp->get_f(), "提題受"))) {
	sscanf(cp, "%*[^:]:%d", &topic_score);
	if (topic_score > 0 && topic_score < 30) {
	    topic_score = 10;
	}
    }
    oss << topic_score;

    string key = have_topic_flag ? "1|W:" : "0|W:";
    key += oss.str();

    // cerr << ";; (W) P(" << key << ") = "; // for debug
    if (case_prob.find(key) != case_prob.end()) { // key exists
        // cerr << case_prob[key] << ", log(P) = " << log(case_prob[key]) << endl;
        return log(case_prob[key]);
    }
    else {
        // cerr << "0" << endl; // for debug
        return UNKNOWN_CASE_SCORE;
    }
}

// 
double CaseFrames::get_np_modifying_probability(CaseFrame *cfd, CaseSlot *cfd_cs_ptr)
{
    int dist = 0;
    char *type = NULL;
    std::ostringstream oss;

    // tp -> hp
    if (cfd->get_pred_bp()->get_token_start() < cfd_cs_ptr->get_first_elem_bp()->get_token_start()) { // 連体修飾
	if ((type = check_feature(cfd->get_pred_bp()->get_f(), "PRED"))) {
	    type += strlen("PRED:");
	}

	// check dependency possibilities
        dist = cfd->get_pred_bp()->get_dpnd_cand_num();
	if (dist <= 0) {
	    return UNKNOWN_CASE_SCORE;
	}
	else if (dist > 1) {
	    dist = 2;
	}
    }
    oss << dist;

    string key = type ? type : "NIL";
    key += ",";
    key += oss.str();
    key += "|R";

    // cerr << ";; (RE) P(" << key << ") = "; // for debug
    if (case_prob.find(key) != case_prob.end()) { // key exists
        // cerr << case_prob[key] << ", log(P) = " << log(case_prob[key]) << endl;
        return log(case_prob[key]);
    }
    else {
        // cerr << "0" << endl; // for debug
        return UNKNOWN_CASE_SCORE;
    }
}

double CaseFrames::get_case_function_probability_generic(Phrase *mp, Phrase *hp) {
    char *cp, *mp_type, *hp_type;
    std::string key;
    std::ostringstream oss;

    if ((cp = check_feature(mp->get_f(), "BP_TYPE")) == NULL)
        return UNKNOWN_CASE_SCORE;
    else
	mp_type = cp + strlen("BP_TYPE:");

    if ((cp = check_feature(hp->get_f(), "BP_TYPE")) == NULL)
        return UNKNOWN_CASE_SCORE;
    else
	hp_type = cp + strlen("BP_TYPE:");

    size_t dist = abs(hp->get_num() - mp->get_num());
    if (dist == 0)
        dist = mp->get_dpnd_cand_num();
    else if (dist == 1)
        ;
    else if (dist < 6) // 1 < dist < 6 ... 2
        dist = 2;
    else // dist >= 6 ... 6
        dist = 6;
    oss << dist;

    key = mp_type;
    key += "," + oss.str() + "|G:" + hp_type;

    // cerr << ";; (G) P(" << key << ") = ";
    if (case_prob.find(key) != case_prob.end()) { // key exists
        // cerr << case_prob[key] << ", log(P) = " << log(case_prob[key]) << endl;
        return log(case_prob[key]);
    }
    else {
        // cerr << "0" << endl; // for debug
        return UNKNOWN_CASE_SCORE;
    }
}

double CaseFrames::get_case_function_probability_generic(CaseFrame *cfd, CaseSlot *cfd_cs_ptr, CaseFrame *cfp, CaseSlot *cfp_cs_ptr) {
    Phrase *mp, *hp;    

    mp = cfd_cs_ptr->get_first_elem_bp();
    hp = cfd->get_pred_bp();

    return get_case_function_probability_generic(mp, hp);
}

double CaseFrames::get_case_function_probability(CaseFrame *cfd, CaseSlot *cfd_cs_ptr, CaseFrame *cfp, CaseSlot *cfp_cs_ptr) {
    int topic_score = 0, dist;
    bool wa_flag, touten_flag, negation_flag, np_modifying_flag, closest_pred_flag = false;
    char *cp, *vtype;
    double score1, score2, score3;
    Phrase *tp, *tmp_tp, *hp;

    // tp -> hp
    if (cfd->get_pred_bp()->get_token_start() < cfd_cs_ptr->get_first_elem_bp()->get_token_start()) { // 連体修飾
	tmp_tp = cfd->get_pred_bp();
	hp = cfd_cs_ptr->get_first_elem_bp();
	np_modifying_flag = true;
    }
    else {
	tmp_tp = cfd_cs_ptr->get_first_elem_bp();
	hp = cfd->get_pred_bp();
	np_modifying_flag = false;
    }

    /*
    // 複合辞
    if (cfd->pp[as1][0] >= FUKUGOJI_START && cfd->pp[as1][0] <= FUKUGOJI_END) {
	scase = pp_code_to_kstr(cfd->pp[as1][0]); // 入力側の表層格
	tp = &(current_sentence_data.tag_data[tp_tmp->num + 1]); // 読点や「は」などをチェックするタグ単位
    }
    else {  */
    string scase;
    if ((cp = check_feature(tmp_tp->get_f(), "係")) == NULL) {
	return UNKNOWN_CASE_SCORE;
    }
    else {
	scase = cp + strlen("係:"); // 入力側の表層格
	tp = tmp_tp;
    }

    // 隣に用言があるかどうか
    if (np_modifying_flag == false) {
        //// if (get_dist_from_work_mgr(tp2->b_ptr, current_sentence_data.tag_data[tp2->num + 1].b_ptr) > 0) {
        // if (tp->get_token_end() + 1 == hp->get_token_start()) {
        //     closest_pred_flag = 1;
        // }
    }

    // 「は」, 読点, 否定のチェック
    wa_flag = check_feature(tp->get_f(), "ハ") ? true : false;
    touten_flag = check_feature(tp->get_f(), "読点") ? true : false;
    negation_flag = check_feature(hp->get_f(), "否定表現")   ? true 
		  : check_feature(hp->get_f(), "準否定表現") ? true 
		  : false;

    // 提題スコア
    if ((cp = check_feature(hp->get_f(), "提題受"))) {
	sscanf(cp, "%*[^:]:%d", &topic_score);
	if (topic_score > 0 && topic_score < 30) {
	    topic_score = 10;
	}
    }

    // 候補数チェック
    dist = tp->get_dpnd_cand_num();
    if (dist <= 0) {
	return UNKNOWN_CASE_SCORE;
    }
    else if (dist > 1) {
	dist = 2;
    }

    // 格の解釈
    string cfcase = make_cf_case_string(cfd, cfd_cs_ptr, cfp, cfp_cs_ptr);
    score1 = get_case_interpret_probability(scase, cfcase, FALSE);

    // 読点の生成
    score2 = get_punctuation_generating_probability(CF_PRED, np_modifying_flag, false, false, touten_flag, dist, 
     						    closest_pred_flag, topic_score, wa_flag);

    // 「は」の生成
    if ((vtype = check_feature(hp->get_f(), "PRED"))) {
	vtype += strlen("PRED:");
    }
    score3 = get_wa_generating_probability(np_modifying_flag, touten_flag, dist, 
    					   closest_pred_flag, topic_score, wa_flag, negation_flag, vtype);

    return score1 + score2 + score3;
}

double CaseFrames::elmnt_match_score(CaseFrame *cfd, CaseSlot *cfd_cs_ptr, CaseFrame *cfp, CaseSlot *cfp_cs_ptr) {
    if (param->language == LANGUAGE_IS_JAPANESE)
        return cfd->get_ex_probability_with_para(cfd_cs_ptr, cfp, cfp_cs_ptr) + cfp->get_case_probability(cfp_cs_ptr, true) + get_case_function_probability(cfd, cfd_cs_ptr, cfp, cfp_cs_ptr);
    else
        return cfd->get_ex_probability_with_para(cfd_cs_ptr, cfp, cfp_cs_ptr) + cfp->get_case_probability(cfp_cs_ptr, true) + get_case_function_probability_generic(cfd, cfd_cs_ptr, cfp, cfp_cs_ptr);
}

bool CaseFrames::_assign_list(CaseFrame *cfd, CaseFrame *cfp, double score, bool assign_flag, bool closest_flag) {
    CaseSlot *cfd_cs_ptr = NULL; // データ側の処理対象の格要素
    bool case_available = false;
    // bool gaflag = false, sotoflag = false, toflag = false;
    double elmnt_score;
    
    // まだ割り当てのない格助詞のチェック
    for (vector<CaseSlot *>::iterator it = cfd->get_cs().begin(); it != cfd->get_cs().end(); it++) {
	if ((*it)->get_assigned()->get_type() == UNASSIGNED && 
	    cfd->get_type() == (*it)->get_type()) { // タイプ一致
	    if ((assign_flag == true  && (*it)->get_pp().size() == 1) || // have only one candidate case
		(assign_flag == false && (*it)->get_pp().size() > 1)) {  // have multiple candidate cases
		cfd_cs_ptr = *it;
		break;
	    }
	}
    }

    if (cfd_cs_ptr) {
	/* すでにガ格に割り当てがあるかどうか (ガ２割り当て可能かどうか)
	for (vector<CaseSlot *>::iterator it = cfp->get_cs().begin(); it != cfp->get_cs().end(); it++) {
	    set<string> pp = (*it)->get_pp();
	    if ((*it)->get_assigned()->get_type() != UNASSIGNED && pp.find("ガ") != pp.end()) {
		gaflag = true;
		break;
	    }
	}
	*/
	/* <AGENT>かどうか (外の関係割り当て可能かどうか)
	if (!cf_match_element(cfd->sm[target], "AGENT", FALSE)) {
	    sotoflag = 1;
	}
	*/
	/* すでに補文ト格に割り当てがあるかどうか (ヲ格割り当て可能かどうか)
	for (i = 0; i < cfp->element_num; i++) {
	    if (list2.flag[i] != UNASSIGNED && 
		MatchPP2(cfp->pp[i], "ト")) {
		toflag = 1;
		break;
	    }
	}
	*/

	// 格フレームの格ループ
	for (vector<CaseSlot *>::iterator cfp_cs_it = cfp->get_cs().begin(); cfp_cs_it != cfp->get_cs().end(); cfp_cs_it++) {
	    // 格フレームの空いている格
	    if ((*cfp_cs_it)->get_assigned()->get_type() == UNASSIGNED) {
		// 解釈されうる格のループ
		set<string> cfd_pp = cfd_cs_ptr->get_pp();
		for (set<string>::iterator cfd_pp_it = cfd_pp.begin(); cfd_pp_it != cfd_pp.end(); cfd_pp_it++) {
		    set<string> cfp_pp = (*cfp_cs_it)->get_pp();
		    if (cfp_pp.find(*cfd_pp_it) != cfp_pp.end()) { // 格一致
		    // set<string>::iterator cfp_pp_it = cfp_pp.begin();
		    // if (*cfp_pp_it == *cfd_pp_it) {
			 /* !((cfp->pp[i][j] == pp_kstr_to_code("外の関係") && !sotoflag) || 
			   (cfp->pp[i][j] == pp_kstr_to_code("ガ２") && !gaflag) || 
			   (cfp->pp[i][j] == pp_kstr_to_code("ノ") && 
			    check_adjacent_assigned(cfd, cfp, &list1) == FALSE))) || 
			(cfd->pp[target][target_pp] == pp_kstr_to_code("未") && 
			check_same_case(cfd->sp[target], cfp->pp[i][j], cfp))) { */
			case_available = true;
			// pos = MATCH_NONE;
			elmnt_score = elmnt_match_score(cfd, cfd_cs_ptr, cfp, *cfp_cs_it);
			// ★名詞の場合は閾値あり?
			// 対応付けをして，残りの格要素の処理に進む
			/* if (cfd->weight[target]) {
			    elmnt_score /= cfd->weight[target];
			    } */
			(*cfp_cs_it)->get_assigned()->set_type(ASSIGNED);
			(*cfp_cs_it)->get_assigned()->set_cs(cfd_cs_ptr);
			(*cfp_cs_it)->get_assigned()->set_score(elmnt_score);
			cfd_cs_ptr->get_assigned()->set_type(ASSIGNED);
			cfd_cs_ptr->get_assigned()->set_cs(*cfp_cs_it);
			cfd_cs_ptr->get_assigned()->set_score(elmnt_score);
			// list2.pos[i] = pos;
			assign_list(cfd, cfp, score + elmnt_score, closest_flag);
			(*cfp_cs_it)->get_assigned()->set_type(UNASSIGNED);
			// list2.pos[i] = MATCH_NONE;
		    }
		}
	    }
	}

	if (case_available == false) {
	    // target番目の格要素には対応付けを行わないマーク
	    cfd_cs_ptr->get_assigned()->set_type(NIL_ASSIGNED);

	    // 割り当てなしのスコア
	    // elmnt_score = FREQ0_ASSINED_SCORE + get_case_function_probability(target, cfd, NIL_ASSIGNED, cfp);
	    elmnt_score = FREQ0_ASSINED_SCORE;
	    /* if (cfd->weight[target]) {
		elmnt_score /= cfd->weight[target];
		} */
	    cfd_cs_ptr->get_assigned()->set_score(elmnt_score);
	    assign_list(cfd, cfp, score + elmnt_score, closest_flag);
	}
	return false;
    }
    return true;
}

// case assignment
bool CaseFrames::assign_list(CaseFrame *cfd, CaseFrame *cfp, double score, bool closest_flag) {
    // 未格, 連格以外を先に割り当て
    if (_assign_list(cfd, cfp, score, true, closest_flag) == false) {
	return false;
    }
    if (_assign_list(cfd, cfp, score, false, closest_flag) == false) {
	return false;
    }

    // 評価: すべての対応付けが終わった場合
    eval_assign_prob(cfd, cfp, score, closest_flag);
    return true;
}

// 格フレームのマッチング (this == cfd)
double CaseFrames::caseframe_match(CaseFrame *cfd, CaseFrame *cfp, int closest_flag) {
    // initialization
    Current_max_score = CASE_MATCH_FAILURE_PROB;
    Current_max_num = 0;
    /* Current_sufficiency = 0;
       Current_max_m_e = 0;
       Current_max_m_p = 0;
       Current_max_c_e = 0; */

    assign_list(cfd, cfp, 0, closest_flag);

    if (Current_max_num > 0) {
	cfd->move_assigned_all(Current_max_assigned_info_dat);
	cfp->move_assigned_all(Current_max_assigned_info_pat);
	// cerr << "OK: " << Current_max_score << endl;
    }
    else {
	// cerr << "NG " << endl;
    }
    return Current_max_score;

    /*
    cmm_ptr->sufficiency = Current_sufficiency;
    cmm_ptr->result_num = Current_max_num;
    for (int i = 0; i < Current_max_num; i++) {
	cmm_ptr->result_lists_p[i] = Current_max_list2[i];
	cmm_ptr->result_lists_d[i] = Current_max_list1[i];
	cmm_ptr->pure_score[i] = Current_pure_score[i];
    }

    // 直前格要素のスコアのみを用いるとき
    if (closest > -1 && Current_max_score >= 0 && 
	Current_max_list1[0].flag[closest] != NIL_ASSIGNED) {
	// 直前格要素の割り当てがあることが条件
	cmm_ptr->score = Current_max_list1[0].score[closest];
    }
    else {
	cmm_ptr->score = Current_max_score;
    }
    */
}

bool CaseFrames::find_best_cf(CaseFrame *cfd, CKY_cell *cky_ptr, int closest_flag) {
    double best_score = CASE_MATCH_FAILURE_PROB;
    // get the corresponding case frames to the input cf_id
    vector<CaseFrame *> *cfs = get_caseframes(cfd->get_cf_id()); // cf_id == rep
    if (!cfs) {
        std::string pseudo_cf_id = DEFAULT_CASE_FRAME_ID;
        cfs = get_caseframes(pseudo_cf_id); // search for default case frame
        if (cfs) {
            cfs->at(0)->set_freq_force(0);
            assign_cfeature(cky_ptr->get_bp_ptr()->get_fp(), "NO_CF", FALSE);
        }
    }

    if (cfs) {
        // cout << "# of case frames for " << cfd->get_cf_id() << " : " << cfs->size() << endl;
	CaseFrame *best_cfp = NULL;
	vector<AssignedInfo *> best_cfd_assigned;
	vector<AssignedInfo *> best_cfp_assigned;

	int freq_of_predicate = 0;
	// sum the frequency of all the case frames
	for (vector<CaseFrame *>::iterator cfp_it = cfs->begin(); cfp_it != cfs->end(); cfp_it++) {
	    freq_of_predicate += (*cfp_it)->get_freq();
	}
	cfd->set_freq(freq_of_predicate); // temporarily set the total freq of case frames to cfd->freq

	// for each case frame
	for (vector<CaseFrame *>::iterator cfp_it = cfs->begin(); cfp_it != cfs->end(); cfp_it++) {
	    (*cfp_it)->new_assigned_all();

	    // closest があれば、直前格要素のみのスコアになる
	    double current_score = caseframe_match(cfd, *cfp_it, closest_flag);
	    if (current_score > best_score) {
		best_score = current_score;
		best_cfp = *cfp_it;
		best_cfp->copy_assigned_all(best_cfp_assigned);
		cfd->copy_assigned_all(best_cfd_assigned);
	    }

	    // 結果を格納 -> スコア順にソート

	    // clean assigned information
	    cfd->clear_assigned_all(); // for cfd
	    // (*cfp_it)->clear_assigned_all(); // for cfp
	}

	if (best_cfp) {
	    cfd->move_assigned_all(best_cfd_assigned);
	    best_cfp->move_assigned_all(best_cfp_assigned);
	    best_cfp->copy_assigned_all(cky_ptr->get_cfp_assigned());
            cky_ptr->set_cs_score(best_score);
	    cky_ptr->add_score(best_score);
	    cky_ptr->set_cfp(best_cfp);
	    return true;
	}
    }
    else { // no case frame
        // cout << "Case frames are not found: " << cfd->get_cf_id() << endl;
        if (param->language == LANGUAGE_IS_GENERIC)
            best_score = collect_argument_probabilities_generic(cfd, cky_ptr);
        cky_ptr->set_cs_score(best_score);
        cky_ptr->add_score(best_score);
        cky_ptr->set_cfp(NULL);
    }
    return false;
}

double CaseFrames::get_pred_co_probability(Phrase *mp, Phrase *hp) {
    if (!pred_co_prob->is_open()) {
	return 0;
    }

    string key = mp->get_rep() + "|" + hp->get_rep();
    string value = pred_co_prob->get(key);
    if (value.size() > 0) { // key exists
	return log(atof(value.c_str()));
    }
    else {
	return FREQ0_ASSINED_SCORE;
    }
}

double CaseFrames::get_noun_co_probability_with_para(CaseSlot *cs_ptr, string &cf_id) {
    double score = 0;
    for (vector<Phrase *>::iterator bp_it = cs_ptr->get_elem_bp().begin(); bp_it != cs_ptr->get_elem_bp().end(); bp_it++) { // coordinated phrases
	string key = (*bp_it)->get_rep() + "|" + cf_id;
	score += get_noun_co_probability(key);
    }
    return score;
}

double CaseFrames::pseudo_noun_analysis(CaseFrame *cfd, CKY_cell *cky_ptr) {
    double score = 0;
    for (vector<CaseSlot *>::iterator cs_it = cfd->get_cs().begin(); cs_it != cfd->get_cs().end(); cs_it++) {
	if ((*cs_it)->get_type() == CF_NOUN) { // && 
	    // (*cs_it)->get_first_pp() == "ノ") {
	    // score += cfd->get_ex_probability_with_para(*cs_it, cfp, cfp_cs_ptr); if a nominal case frame exists
	    score += get_noun_co_probability_with_para(*cs_it, cfd->get_cf_id()); // noun-noun cooccurrences

	    Phrase *bp = (*cs_it)->get_first_elem_bp();

            if (param->language == LANGUAGE_IS_JAPANESE) {
                // check dependency possibilities
                int dist = bp->get_dpnd_cand_num();
                if (dist <= 0)
                    score += UNKNOWN_CASE_SCORE;
                else if (dist > 1)
                    dist = 2;

                // check the existence of comma
                bool touten_flag = check_feature(bp->get_f(), "読点") ? true : false;
                score += get_punctuation_generating_probability(CF_NOUN, true, false, false, touten_flag, 
                                                                dist, false, 0, false);
            }
            else {
                score += get_case_function_probability_generic(bp, cky_ptr->get_bp_ptr());
            }
	}
    }
    if (score != 0) {
        score += NOMINAL_CASEFRAME_SCORE; // for temporal probability for nominal case frame
    }
    cky_ptr->add_score(score);
    return score;
}

}
