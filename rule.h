#ifndef RULE_H
#define RULE_H

#include "knp_const.h"
#include "phrase.h"
#include "morpheme.h"
#include "parameter.h"

namespace Parser {

class CKY;
class CKY_cell;
class Rules;
class CaseFrames;

// a class for a set of rules
class GeneralRuleType {
    void	*RuleArray;
    int		CurRuleSize;
    const char	*filename;
    int		type;
    int		mode;
    int		breakmode;
    int		direction;
  public:
    GeneralRuleType(const char *in_filename, int in_type) { // for HomoRuleType
	RuleArray = NULL;
	CurRuleSize = 0;
	filename = in_filename;
	type = in_type;
	mode = 0;
	breakmode = 0;
	direction = 0;
	make_rules();
    }
    GeneralRuleType(const char *in_filename, int in_type, int in_mode, int in_breakmode, int in_direction) {
	RuleArray = NULL;
	CurRuleSize = 0;
	filename = in_filename;
	type = in_type;
	mode = in_mode;
	breakmode = in_breakmode;
	direction = in_direction;
	make_rules();
    }
    GeneralRuleType(const char *in_filename, int in_type, int in_mode) {
	RuleArray = NULL;
	CurRuleSize = 0;
	filename = in_filename;
	type = in_type;
	mode = in_mode;
	breakmode = 0;
	direction = 0;
	make_rules();
    }
    ~GeneralRuleType() {
	free(RuleArray);
    }
    
    bool make_rules() {
	switch (type) {
	case HomoRuleType:
	    RuleArray = (HomoRule *)malloc(sizeof(HomoRule) * GeneralRule_MAX); // HomoRule_MAX	(128)
	    break;
	case MorphRuleType:
	    RuleArray = (MrphRule *)malloc(sizeof(MrphRule) * GeneralRule_MAX);
	    break;
	case LatticeMorphRuleType:
	    RuleArray = (MrphRule *)malloc(sizeof(MrphRule) * GeneralRule_MAX);
	    break;
	case PhraseRuleType:
	    RuleArray = (PhraseRule *)malloc(sizeof(PhraseRule) * GeneralRule_MAX);
	    break;
	case LatticePhraseRuleType:
	    RuleArray = (PhraseRule *)malloc(sizeof(MrphRule) * GeneralRule_MAX);
	    break;
	case CFGRuleType:
	    RuleArray = (CFGRule *)malloc(sizeof(CFGRule) * GeneralRule_MAX); // CFGRule_MAX (512)
	    break;
	case DpndRuleType:
	    RuleArray = (DpndRule *)malloc(sizeof(DpndRule) * GeneralRule_MAX); // DpndRule_MAX (128)
	    break;
	default:
	    std::cerr << ";; Unknown rule type: " << type << "." << std::endl;
	    return false;
	}
	return true;
    }
    void *get_rule_array() {
	return RuleArray;
    }
    int get_rulesize() {
	return CurRuleSize;
    }
    int get_type() {
	return type;
    }
    int get_mode() {
	return mode;
    }
    int get_break_mode() {
	return breakmode;
    }
    int get_direction() {
	return direction;
    }
    bool increase_rulesize() {
	if (++CurRuleSize == GeneralRule_MAX) {
	    std::cerr << ";; Too many Rule for " << filename << "." << std::endl;
	    return false;
	}
	return true;
    }

    bool assign_mrph_feature(std::vector<Morpheme *>::iterator s_m_ptr, int m_length, int temp_assign_flag);
    bool assign_mrph_feature_for_lattice(CKY *cky, int temp_assign_flag);
    bool assign_phrase_feature(std::vector<Phrase *>::iterator s_b_ptr, int b_length,
			       int also_assign_flag, int temp_assign_flag);
    bool assign_phrase_feature_for_lattice(CKY *cky, int also_assign_flag, int temp_assign_flag);

    bool check_possibility_and_make_bp(CKY_cell *cky_ptr, CKY_cell *left_ptr, CKY_cell *right_ptr, Rules *rules, CaseFrames *caseframes);
    bool assign_dpnd_rule_to_bp(Phrase *bp_ptr);
};


class Rules {
    Parameter *param;
    std::vector<GeneralRuleType *> GeneralRuleArray;
  public:
    Rules(Parameter *in_param);
    ~Rules() {
	for (std::vector<GeneralRuleType *>::iterator it = GeneralRuleArray.begin(); it != GeneralRuleArray.end(); it++) {
	    delete *it;
	}
	GeneralRuleArray.clear();
    }

    int get_rule_num() {
	return GeneralRuleArray.size();
    }
    std::vector<GeneralRuleType *> &get_rule_array() {
	return GeneralRuleArray;
    }

    bool read_homo_rule(std::string filename);
    bool read_mrph_rule(std::string filename, int mode, int breakmode, int direction, bool lattice_flag);
    bool read_phrase_rule(std::string filename, int mode, int breakmode, int direction, bool lattice_flag);
    bool read_cfg_rule(std::string filename, int mode);
    bool read_dpnd_rule(std::string filename);

    bool assign_mrph_feature(std::vector<Morpheme *>::iterator s_m_ptr, int m_length, int temp_assign_flag) {
	for (std::vector<GeneralRuleType *>::iterator it = GeneralRuleArray.begin(); it != GeneralRuleArray.end(); it++) {
	    if ((*it)->get_type() == MorphRuleType) {
		(*it)->assign_mrph_feature(s_m_ptr, m_length, temp_assign_flag);
	    }
	}
	return true;
    }
    bool assign_mrph_feature(Morpheme *s_m_ptr, int m_length, int temp_assign_flag) {
	std::vector<Morpheme *> morphemes; // Morpheme pointer -> std::vector<Morpheme *>
	for (int i = 0; i < m_length; i++) {
	    morphemes.push_back(s_m_ptr + i);
	}
	return assign_mrph_feature(morphemes.begin(), morphemes.size(), temp_assign_flag);
    }

    bool assign_mrph_feature_for_lattice(CKY *cky, int temp_assign_flag) {
	for (std::vector<GeneralRuleType *>::iterator it = GeneralRuleArray.begin(); it != GeneralRuleArray.end(); it++) {
	    if ((*it)->get_type() == LatticeMorphRuleType) {
		(*it)->assign_mrph_feature_for_lattice(cky, temp_assign_flag);
	    }
	}
	return true;
    }

    bool assign_phrase_feature(std::vector<Phrase *>::iterator s_b_ptr, int b_length,
			       int also_assign_flag, int temp_assign_flag) {
	for (std::vector<GeneralRuleType *>::iterator it = GeneralRuleArray.begin(); it != GeneralRuleArray.end(); it++) {
	    if ((*it)->get_type() == PhraseRuleType) {
		(*it)->assign_phrase_feature(s_b_ptr, b_length, also_assign_flag, temp_assign_flag);
	    }
	}
	return true;
    }
    bool assign_phrase_feature(Phrase *s_b_ptr, int b_length,
			       int also_assign_flag, int temp_assign_flag) {
	std::vector<Phrase *> phrases; // Phrase pointer -> std::vector<Phrase *>
	for (int i = 0; i < b_length; i++) {
	    phrases.push_back(s_b_ptr + i);
	}
	return assign_phrase_feature(phrases.begin(), phrases.size(), also_assign_flag, temp_assign_flag);
    }
    bool assign_phrase_feature_for_lattice(CKY *cky, int also_assign_flag, int temp_assign_flag) {
	for (std::vector<GeneralRuleType *>::iterator it = GeneralRuleArray.begin(); it != GeneralRuleArray.end(); it++) {
	    if ((*it)->get_type() == LatticePhraseRuleType) {
		(*it)->assign_phrase_feature_for_lattice(cky, also_assign_flag, temp_assign_flag);
	    }
	}
	return true;
    }

    bool check_possibility_and_make_bp(CKY_cell *cky_ptr, CKY_cell *left_ptr, CKY_cell *right_ptr, CaseFrames *caseframes) {
	bool ret = false;
	for (std::vector<GeneralRuleType *>::iterator it = GeneralRuleArray.begin(); it != GeneralRuleArray.end(); it++) {
	    if ((*it)->get_type() == CFGRuleType) {
		ret = (*it)->check_possibility_and_make_bp(cky_ptr, left_ptr, right_ptr, this, caseframes);
	    }
	}
	return ret;
    }

    bool assign_dpnd_rule_to_bp(Phrase *bp_ptr) {
	bool ret = false;
	for (std::vector<GeneralRuleType *>::iterator it = GeneralRuleArray.begin(); it != GeneralRuleArray.end(); it++) {
	    if ((*it)->get_type() == DpndRuleType) {
		ret = (*it)->assign_dpnd_rule_to_bp(bp_ptr);
	    }
	}
	return ret;
    }

    bool check_predicate_argument_possibility(Phrase *k_ptr, Phrase *u_ptr);
    char check_dpnd_possibility(CKY_cell *left_ptr, CKY_cell *right_ptr);
    char match_dpnd_rule(Phrase *k_ptr, Phrase *u_ptr);
    int count_dpnd_possible_candidates(CKY_cell *dpnd_cky_ptr, CKY_cell *cky_ptr);
};

}

#endif
