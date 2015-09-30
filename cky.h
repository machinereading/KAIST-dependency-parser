#ifndef CKY_H
#define CKY_H

#include "common.h"
#include "parameter.h"
#include "sentence.h"
#include "morpheme.h"
#include "rule.h"
#include "caseframe.h"
#include "lexeme.h"

namespace Parser {

class Rules;
class GeneralRuleType;

class CKY_cell {
    bool	exist_flag;
    int		i;
    int		j;
    int		para_flag;
    int		dpnd_head;
    char	dpnd_type;
    double	score;
    double	cs_score;	// score of case structure
    double	vp_score;
    double	para_score;
    int		token_num;
    Morpheme	*mrph_ptr;
    Phrase	*bp_ptr;
    int		direction;	/* direction of dependency */
    CKY_cell	*left;		/* pointer to the left child */
    CKY_cell	*right;		/* pointer to the right child */
    CKY_cell	*next;		/* pointer to the next CKY cell at this point */
    CaseFrame	*cfd;		/* input case frame */
    CaseFrame	*cfp;		/* corresponding case frame */
    vector<AssignedInfo *> cfp_assigned;
  public:
    CKY_cell() : exist_flag(false), mrph_ptr(NULL), bp_ptr(NULL), left(NULL), right(NULL), next(NULL), cfd(NULL), cfp(NULL) {}
    bool set(int in_i_j);
    bool set(Morpheme *in_mrph_ptr, Rules *rules);
    bool set(Phrase *in_phrase_ptr, Rules *rules);
    bool set(CKY_cell *in_left, CKY_cell *in_right, char dpnd_type, Rules *rules, CaseFrames *caseframes);
    bool set(CKY_cell *in_left, CKY_cell *in_right, FEATURE *fp, Rules *rules, CaseFrames *caseframes);
    bool case_analysis(CaseFrames *caseframes, Rules *rules);

    int get_token_start() {
	return i;
    }
    int get_token_end() {
	return j;
    }
    int get_dpnd_head() {
	return dpnd_head;
    }
    char get_dpnd_type() {
	return dpnd_type;
    }
    int get_token_num() {
	return token_num;
    }
    double get_score() {
	return score;
    }
    double set_score(double in_score) {
        score = in_score;
	return score;
    }
    double add_score(double in_score) {
	score += in_score;
	return score;
    }
    double get_cs_score() {
	return cs_score;
    }
    double set_cs_score(double in_cs_score) {
	cs_score = in_cs_score;
        return cs_score;
    }
    bool exist() {
	if (exist_flag) {
	    return true;
	}
	else {
	    return false;
	}
    }
    string &get_str() {
	if (mrph_ptr) {
	    return mrph_ptr->get_goi();
	}
	else { // if (bp_ptr) {
	    return bp_ptr->get_str();
	}
    }
    bool print() {
	if (mrph_ptr) {
	    mrph_ptr->print();
	}
	if (bp_ptr) {
	    bp_ptr->print();
	}
	return true;
    }
    Morpheme *get_mrph_ptr() {
	return mrph_ptr;
    }
    Phrase *get_bp_ptr() {
	return bp_ptr;
    }
    CKY_cell *get_left() {
	return left;
    }
    CKY_cell *get_right() {
	return right;
    }
    CKY_cell *get_next() {
	return next;
    }
    CKY_cell **get_next_pp() {
	return &next;
    }
    void set_next(CKY_cell *in_cell) {
        next = in_cell;
    }
    CKY_cell *find_and_make_new_cky_cell(int k_best_num) {
        int count = 0;
	CKY_cell *cell = this;
	CKY_cell *pre_cell = this;
	while (cell && cell->exist()) {
	    pre_cell = cell;
	    cell = cell->next;
            count++;
	}
	if (cell) {
	    return cell;
	}
	else {
            if (k_best_num == 0 || count < k_best_num) { // up to k_best_num if specified
                pre_cell->next = new CKY_cell;
                return pre_cell->next;
            }
            else {
                return NULL;
            }
	}
    }
    CaseFrame *get_cfd() {
	return cfd;
    }
    CaseFrame *set_cfp(CaseFrame *in_cfp) {
	return cfp = in_cfp;
    }
    vector<AssignedInfo *> &get_cfp_assigned() {
	return cfp_assigned;
    }
};

class CKY {
    int n;
    int phrase_num;
    Parameter *param;
    Sentence *sentence;
    Rules *rules;
    CaseFrames *caseframes;
    CKY_cell **cky_table;
    Lexeme *lexeme;
  public:
    CKY(Sentence *in_sentence, Rules *in_rules, CaseFrames *in_caseframes, Lexeme *in_lexeme, Parameter *in_param);
    ~CKY();

    int get_token_num() {
	return n;
    }
    int get_base_cky_pos(int i, int j);
    CKY_cell **get_base_cky_cell_pp(int i, int j);
    CKY_cell *get_base_cky_cell(int i, int j);
    CKY_cell *make_base_cky_cell(int i, int j);
    CKY_cell *make_new_cky_cell(int i, int j);
    CKY_cell *make_new_cky_cell(int i, int j, int k_best_num);
    bool set_morphemes_to_cky();
    bool set_phrases_to_cky();
    bool make_phrases_on_cky();
    void sort_cky_cells(int i, int j);
    bool parse();
    bool get_best_parse();
    bool print();
    bool assign_eos_feature_to_mrph();
    bool assign_eos_feature_to_phrase();
    bool case_analysis_on_single_predicate();

    bool assign_phrase_num(CKY_cell *cky_ptr);
    bool print_bp_dependencies(CKY_cell *cky_ptr, double score, bool is_success);
    bool print_bp_dependency(CKY_cell *cky_ptr, char dpnd_type, int dpnd_head);

    void assign_features_of_ld_from_id(Morpheme *mrph_ptr, unsigned int id);
    void assign_lexical_features_to_morphemes();
    void ld_traverse_one_morpheme(Morpheme *mrph_ptr, size_t node_pos);
};

}

#endif
