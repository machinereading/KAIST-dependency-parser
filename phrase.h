#ifndef PHRASE_H
#define PHRASE_H

#include "common.h"
#include "knp_const.h"
#include "morpheme.h"

namespace Parser {

using std::vector;
using std::string;

class CKY_cell;

class Phrase {
    int		num;
    int		token_start; // i in CKY
    int		token_end;   // j in CKY
    int		token_num;
    vector<Morpheme *> morphemes;
    string str;
    string rep;
    int         dpnd_head; // the number of dpnd head (used in printing)
    int		dpnd_cand_num;
    // vector<string> f;
    FEATURE	*f;
    vector<DpndRule *> dpnd_rule;
    vector<Phrase *> children;
public:
    // Phrase();
    Phrase(CKY_cell *in_left, CKY_cell *in_right);
    Phrase(string &line, size_t in_phrase_num);
    ~Phrase();

    void print();
    void print_tab(char dpnd_type, int dpnd_head);
    void print_xml(char dpnd_type, int dpnd_head);

    int set_num(int in_num) {
	return num = in_num;
    }
    int get_num() {
	return num;
    }
    int get_token_start() {
	return token_start;
    }
    int get_token_end() {
	return token_end;
    }
    int get_token_num() {
	return token_num;
    }
    vector<Morpheme *> &get_morphemes() {
	return morphemes;
    }
    int get_mrph_num() { // the number of morphemes
	return morphemes.size();
    }
    string &get_str() {
	return str;
    }
    string &get_rep() {
	return rep;
    }
    FEATURE *get_f() {
	return f;
    }
    FEATURE **get_fp() {
	return &f;
    }
    int set_dpnd_head(int in_dpnd_head) {
	return dpnd_head = in_dpnd_head;
    }
    int get_dpnd_head() {
	return dpnd_head;
    }
    int set_dpnd_cand_num(int in_dpnd_cand_num) {
	return dpnd_cand_num = in_dpnd_cand_num;
    }
    int get_dpnd_cand_num() {
	return dpnd_cand_num;
    }
    vector<DpndRule *> &get_dpnd_rule() {
	return dpnd_rule;
    }
    int get_dpnd_rule_size() {
	return dpnd_rule.size();
    }
    int set_dpnd_rule(DpndRule *in_dpnd_rule) {
	dpnd_rule.push_back(in_dpnd_rule);
	return dpnd_rule.size();
    }
    void push_back_child(Phrase *in_phrase) {
        children.push_back(in_phrase);
    }
    void draw_tree(std::vector<bool> mark);
    void draw_tree() {
        std::vector<bool> mark;
        draw_tree(mark);
    }
};

}

#endif
