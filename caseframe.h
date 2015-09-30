#ifndef CASEFRAME_H
#define CASEFRAME_H

#include "common.h"
#include "parameter.h"
#include "rule.h"
#include "phrase.h"
// #include "hash.h"
#include "dbm.h"

namespace Parser {

using std::string;
using std::vector;
using std::map;
using std::set;
using std::cout;

class CaseFrames;
class CaseSlot;

class AssignedInfo {
    cs_assigned_type type;
    CaseSlot *cs;
    double score;
  public:
    AssignedInfo() : type(UNASSIGNED), cs(NULL) {
	;
    }
    AssignedInfo(AssignedInfo &src) {
	type = src.type;
	cs = src.cs;
	score = src.score;
    }
    cs_assigned_type get_type() {
	return type;
    }
    cs_assigned_type set_type(cs_assigned_type in_type) {
	return type = in_type;
    }
    CaseSlot *set_cs(CaseSlot *in_cs) {
	return cs = in_cs;
    }
    double set_score(double in_score) {
	return score = in_score;
    }
    double get_score() {
	return score;
    }
    bool clear() {
	type = UNASSIGNED;
	cs = NULL;
	return true;
    }
    bool print() {
	if (type == NIL_ASSIGNED) {
	    cout << "X";
	}
	else if (type == ASSIGNED) {
	    cout << "O";
	}
	else { // UNASSIGNED
	    cout << "-";
	}
	return true;
    }
};

class CaseSlot {
    cf_type type;
    bool oblig;
    bool adjacent;
    int freq;
    set<string> pp;
    map<string, int> ex;
    map<string, int> gex;

    vector<Phrase *> elem_bp; // for input case structure (並列要素格納用に複数)
    AssignedInfo *assigned;
  public:
    CaseSlot(cf_type in_type) : oblig(false), adjacent(false), freq(0), assigned(NULL) {
	type = in_type;
    }
    ~CaseSlot() {
	pp.clear();
	if (assigned) {
	    delete assigned;
	}
    }
    cf_type get_type() {
	return type;
    }
    cf_type set_type(cf_type in_type) {
	return type = in_type;
    }
    set<string> &get_pp() {
	return pp;
    }
    const string &get_first_pp() {
	return *(pp.begin());
    }
    map<string, int> &get_ex() {
	return ex;
    }
    map<string, int> &get_gex() {
	return gex;
    }
    int get_pp_num() {
	return pp.size();
    }
    bool set_pp(string &str);
    bool set_pp(const char *cp);
    bool set_oblig(bool in_oblig) {
	return oblig = in_oblig;
    }
    bool set_adjacent(bool in_adjacent) {
	return adjacent = in_adjacent;
    }
    int get_freq() {
	return freq;
    }
    bool process_and_set_pp(string &str);
    bool set_ex(Phrase *bp_ptr);
    bool set_ex(string &str, int str_freq);
    bool set_ex(const char *cp, int str_freq);
    bool process_and_set_ex(string &str);
    bool set_gex(string &str, int str_freq);
    bool set_gex(const char *cp, int str_freq);
    bool set_gex(Phrase *bp_ptr);
    int set_elem_bp(Phrase *bp_ptr) {
	elem_bp.push_back(bp_ptr);
	return elem_bp.size();
    }
    int unset_elem_bp() {
	elem_bp.pop_back();
	return elem_bp.size();
    }
    vector<Phrase *> &get_elem_bp() {
	return elem_bp;
    }
    Phrase *get_first_elem_bp() {
	return elem_bp.front();
    }
    AssignedInfo *set_assigned(AssignedInfo *in_assigned) {
	return assigned = in_assigned;
    }
    AssignedInfo *get_assigned() {
	return assigned;
    }
/*     AssignedInfo *new_assigned() { */
/* 	if (assigned == NULL) { */
/* 	    assigned = new AssignedInfo; */
/* 	} */
/* 	return assigned; */
/*     } */
/*     bool clear_assigned() { */
/* 	if (assigned) { */
/* 	    return assigned->clear(); */
/* 	} */
/* 	else { */
/* 	    return false; */
/* 	} */
/*     } */
    double get_ex_probability_internal(const string &key);
    bool print();
};

class CaseFrame {
    cf_type type;
    int voice;
    int freq;
    string cf_id;
    string pred_type;
    string entry;
    string read;
    vector<string> feature;
    vector<CaseSlot *> cs;

    Phrase *pred_bp; // predicate bp
    vector<AssignedInfo *> assigned;
  public:
    CaseFrame(cf_type in_type, string &in_cf_id, string &in_read, string &in_entry);
    CaseFrame(cf_type in_type, CKY_cell *cky_ptr, CaseFrames *caseframes, Rules *rules); // for input case frame
    ~CaseFrame();
    cf_type get_type() {
	return type;
    }
    cf_type set_type(cf_type in_type) {
	return type = in_type;
    }
    int set_freq(int in_freq) {
	if (freq < in_freq) { // assume the max freq of a case slot as the freq of CF
	    freq = in_freq;
	}
	return freq;
    }
    void set_freq_force(int in_freq) {
        freq = in_freq;
    }
    int get_freq() {
	return freq;
    }
    string &get_cf_id() {
	return cf_id;
    }
    bool str2feature(std::string &str);
    vector<CaseSlot *> &get_cs() {
	return cs;
    }
    bool set_cs(CaseSlot *cs_ptr) {
	cs.push_back(cs_ptr);
	return true;
    }
    CaseSlot *get_last_cs_ptr() {
	return cs.back();
    }
    bool unset_cs() {
        // delete elem_bp

	cs.pop_back();
	return true;
    }
    bool print();

    // for input case structure
    bool _make_data_from_feature_to_pp(Phrase *b_ptr, bool adjacent_flag, bool clausal_modifier_flag);
    Phrase *_make_data_cframe_pp(Phrase *b_ptr, bool adjacent_flag, bool clausal_modifier_flag);
    bool make_data_cframe_child(Phrase *child_ptr, bool adjacent_flag, int dpnd_cand_num);
    bool add_coordinated_phrases(CKY_cell *cky_ptr);
    void make_data_cframe_rentai_simple(Phrase *d_ptr, Phrase *t_ptr, int dpnd_cand_num);
    bool collect_arguments(CKY_cell *cky_ptr, CaseFrames *caseframes, Rules *rules);
    double get_ex_probability_with_para(CaseSlot *cfd_cs_ptr, CaseFrame *cfp, CaseSlot *cfp_cs_ptr);
    double get_case_probability(CaseSlot *cs_ptr, bool aflag);
    double get_cf_probability(int total_freq);

    Phrase *get_pred_bp() {
	return pred_bp;
    }
    bool new_assigned(CaseSlot *cs_ptr) {
	return set_assigned(cs_ptr, new AssignedInfo);
    }
    bool set_assigned(CaseSlot *cs_ptr, AssignedInfo *ai_ptr) {
	assigned.push_back(ai_ptr);
	cs_ptr->set_assigned(ai_ptr);
	return true;
    }
    bool clear_assigned_all() {
	for (vector<AssignedInfo *>::iterator ai_it = assigned.begin(); ai_it != assigned.end(); ai_it++) {
	    (*ai_it)->clear();
	}
	return true;
    }
    bool new_assigned_all() {
	if (assigned.empty()) {
	    for (vector<CaseSlot *>::iterator cs_it = cs.begin(); cs_it != cs.end(); cs_it++) {
		new_assigned(*cs_it);
	    }
	}
	else { // already exist
	    clear_assigned_all();
	}
	return true;
    }
    bool copy_assigned_all(vector<AssignedInfo *> &dest_ai) {
	if (!dest_ai.empty()) { // delete exsiting pointers in destination
	    for (vector<AssignedInfo *>::iterator ai_it = dest_ai.begin(); ai_it != dest_ai.end(); ai_it++) {
		delete *ai_it;
	    }
	    dest_ai.clear();
	}
	for (vector<AssignedInfo *>::iterator ai_it = assigned.begin(); ai_it != assigned.end(); ai_it++) {
	    AssignedInfo *ai_ptr = new AssignedInfo(**ai_it); // copy
	    dest_ai.push_back(ai_ptr);
	}
	return true;
    }
    bool move_assigned_all(vector<AssignedInfo *> &src_ai) {
	// delete existing pointers
	for (vector<AssignedInfo *>::iterator ai_it = assigned.begin(); ai_it != assigned.end(); ai_it++) {
	    delete *ai_it;
	}
	assigned.clear();

	vector<AssignedInfo *>::iterator ai_it = src_ai.begin();
	for (vector<CaseSlot *>::iterator cs_it = cs.begin(); cs_it != cs.end(); cs_it++, ai_it++) {
	    set_assigned(*cs_it, *ai_it); // move *ai_it to *cs_it
	}
	src_ai.clear(); // clear src vector
	return true;
    }
};

class CaseFrames {
    Parameter *param;
    map<string, vector<CaseFrame *> > cf;
    map<string, double> case_prob;
    Dbm *noun_co_prob;
    Dbm *pred_co_prob;
  public:
    CaseFrames(Parameter *in_param);
    ~CaseFrames();
    Parameter *get_param() {
        return param;
    }
    bool read_caseframes(const string &filename);
    bool read_case_prob(const string &filename);
    void init_db(Dbm **db);
    bool open_db(const string &filename, Dbm *db);
    bool print();
    vector<CaseFrame *> *get_caseframes(string &key);

    double collect_argument_probabilities_generic(CaseFrame *cfd, CKY_cell *cky_ptr);
    bool find_best_cf(CaseFrame *cfd, CKY_cell *cky_ptr, int closest_flag);
    double caseframe_match(CaseFrame *cfd, CaseFrame *cfp, int closest_flag);
    bool assign_list(CaseFrame *cfd, CaseFrame *cfp, double score, bool closest_flag);
    bool _assign_list(CaseFrame *cfd, CaseFrame *cfp, double score, bool assign_flag, bool closest_flag);
    bool eval_assign_prob(CaseFrame *cfd, CaseFrame *cfp, double score, int closest);
    double elmnt_match_score(CaseFrame *cfd, CaseSlot *cfd_cs_ptr, CaseFrame *cfp, CaseSlot *cfp_cs_ptr);
    double get_case_function_probability_generic(Phrase *mp, Phrase *hp);
    double get_case_function_probability_generic(CaseFrame *cfd, CaseSlot *cfd_cs_ptr, CaseFrame *cfp, CaseSlot *cfp_cs_ptr);
    double get_case_function_probability(CaseFrame *cfd, CaseSlot *cfd_cs_ptr, CaseFrame *cfp, CaseSlot *cfp_cs_ptr);
    double calc_vp_modifying_probability(Phrase *dp, Phrase *gp, bool np_modifying_flag, bool adverb_flag);
    const string &make_cf_case_string(CaseFrame *cfd, CaseSlot *cfd_cs_ptr, CaseFrame *cfp, CaseSlot *cfp_cs_ptr);
    double get_case_interpret_probability(const string &scase, const string &cfcase, bool ellipsis_flag);
    double get_punctuation_generating_probability(cf_type type, bool np_modifying_flag, bool vp_modifying_flag, 
						  bool adverb_flag, bool touten_flag, int dist, 
						  bool closest_pred_flag, int topic_score, bool wa_flag);
    double get_wa_generating_probability(bool np_modifying_flag, bool touten_flag, int dist, 
                                         bool closest_pred_flag, int topic_score, bool wa_flag, 
                                         bool negation_flag, char *vtype);
    double get_topic_generating_probability(bool have_topic_flag, Phrase *gp);
    double get_np_modifying_probability(CaseFrame *cfd, CaseSlot *cfd_cs_ptr);
    double get_noun_co_probability(string &key);
    double get_noun_co_probability_with_para(CaseSlot *cs_ptr, string &cf_id);
    double get_pred_co_probability(Phrase *mp, Phrase *hp);
    double pseudo_noun_analysis(CaseFrame *cfd, CKY_cell *cky_ptr);
    double collect_vp_probs(CKY_cell *cky_ptr, Rules *rules);
};

}

#endif
