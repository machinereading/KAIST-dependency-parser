#include "common.h"
#include "phrase.h"
#include "cky.h"

namespace Parser {

using std::cout;
using std::cerr;
using std::endl;
using std::vector;

extern char *check_feature(FEATURE *fp, const char *fname);
extern void assign_cfeature(FEATURE **fpp, const char *fname, int temp_assign_flag);

void Phrase::print() {
    if (str.size()) {
	cout << str;
    }
    if (rep.size()) {
	cout << " (" << rep << ")";
    }

    if (f) { // features
        cout << " ";
	f->print();
    }
    cout << endl;
}

void Phrase::print_tab(char dpnd_type, int dpnd_head) {
    cout << "+ " << dpnd_head << dpnd_type;
    if (f) { // features
        cout << " ";
        f->print();
    }
    cout << endl;

    // morphemes
    if (get_mrph_num() > 0)
        for (vector<Morpheme *>::iterator it = morphemes.begin(); it != morphemes.end(); it++)
            (*it)->print();
    else
        cout << rep << endl;
}

void Phrase::print_xml(char dpnd_type, int dpnd_head) {
    cout << "        <phrase id=\"" << num << "\" head=\"" << dpnd_head << "\" category=\"NONE\" feature=\"";
    if (f) // features
	f->print_xml();
    cout << "\" dpndtype=\"" << dpnd_type << "\">";

    // morphemes
    if (get_mrph_num() > 0) {
        cout << endl;
        for (vector<Morpheme *>::iterator it = morphemes.begin(); it != morphemes.end(); it++)
            (*it)->print_xml();
        cout << "        </phrase>" << endl;
    }
    else {
        cout << rep << "</phrase>" << endl;
    }
}

Phrase::Phrase(CKY_cell *in_left, CKY_cell *in_right) : dpnd_head(-1), dpnd_cand_num(0) {
    token_num = in_left->get_token_num();
    token_start = in_left->get_token_start();

    // check left
    if (Phrase *left_bp_ptr = in_left->get_bp_ptr()) { // if left is bp
	for (vector<Morpheme *>::iterator it = left_bp_ptr->get_morphemes().begin(); it != left_bp_ptr->get_morphemes().end(); it++) {
	    morphemes.push_back(*it);
	    str += (*it)->get_goi();
	}
	rep = left_bp_ptr->get_rep();
    }
    else if (Morpheme *left_mrph_ptr = in_left->get_mrph_ptr()) { // if left is mrph
	morphemes.push_back(left_mrph_ptr);
	str += left_mrph_ptr->get_goi();
	if (check_feature(left_mrph_ptr->get_f(), "CONT")) {
	    rep = left_mrph_ptr->get_rep();
	}
    }

    if (in_right) { // check right if available
	token_num += in_right->get_token_num();
	token_end = in_right->get_token_end();
	if (Phrase *right_bp_ptr = in_right->get_bp_ptr()) { // if right is bp
	    for (vector<Morpheme *>::iterator it = right_bp_ptr->get_morphemes().begin(); it != right_bp_ptr->get_morphemes().end(); it++) {
		morphemes.push_back(*it);
		str += (*it)->get_goi();
	    }
	    rep = right_bp_ptr->get_rep();
	}
	else if (Morpheme *right_mrph_ptr = in_right->get_mrph_ptr()) { // if right is mrph
	    morphemes.push_back(right_mrph_ptr);
	    str += right_mrph_ptr->get_goi();
	    if (check_feature(right_mrph_ptr->get_f(), "CONT")) {
		rep = right_mrph_ptr->get_rep();
	    }
	}
    }
    else {
	token_end = in_left->get_token_end();
    }

    if (rep.size() == 0) {
	// cerr << ";; Failed to set rep." << endl;
	rep = morphemes.back()->get_rep();
    }

    f = NULL;
    // assign <EOS> feature if the last morpheme has <EOS>
    if (check_feature(morphemes.back()->get_f(), "EOS")) {
	assign_cfeature(&f, "EOS", FALSE);
    }
}

void Phrase::draw_tree(std::vector<bool> mark) {
    if (children.size() > 0) {
        for (std::vector<Phrase *>::reverse_iterator it = children.rbegin(); it != children.rend(); it++) { // to the second last child
            mark.push_back(it == children.rbegin() ? true : false); // the farthest child is true
            (*it)->draw_tree(mark);
            mark.pop_back();
        }
    }

    if (mark.size() > 0) {
        bool last_mark = mark.back();
        mark.pop_back();
        for (std::vector<bool>::iterator it = mark.begin(); it != mark.end(); it++) {
            if (*it == true)
                std::cout << "　　";
            else
                std::cout << "│　";
        }
        if (last_mark == true) // mark is true if it is the farthest child
            std::cout << "┌─";
        else
            std::cout << "├─";
    }

    std::cout << str << std::endl;
}

Phrase::Phrase(string &line, size_t in_phrase_num) : dpnd_head(-1), dpnd_cand_num(0) {
    vector<string> input, wstring, fstring, input_str_mrph, input_pos_mrph;

    int input_num = split_string(line, " ", input);
    if (input_num < 4 || input_num > 5) {
	cerr << "Invalid input: " << line << endl;
	return;
    }

    num = in_phrase_num;
    token_start = atoi(input[0]);
    token_end = atoi(input[1]) - 1;
    token_num = token_end - token_start + 1;

    int wnum = split_string(input[2], "+", wstring);
    rep = wnum == 0 ? input[2] : wstring[0]; // use the first word as rep

    f = NULL;
    std::string fstr = "BP_TYPE:" + input[3]; // POS sequence
    assign_cfeature(&f, const_cast<char *>(fstr.c_str()), FALSE);

    int mrph_str_num = split_string(input[2], "+", input_str_mrph);
    int mrph_pos_num = split_string(input[3], "+", input_pos_mrph);
    if (mrph_str_num == mrph_pos_num) { // same number of str/pos
        for (vector<string>::iterator str_it = input_str_mrph.begin(), pos_it = input_pos_mrph.begin(); str_it != input_str_mrph.end(); str_it++, pos_it++) {
            morphemes.push_back(new Morpheme(*str_it, *pos_it, token_start, token_end));
        }
    }

    if (input_num == 5) {
        int fnum = split_string(input[4], ",", fstring);
        for (int i = 0; i < fnum; i++) {
            assign_cfeature(&f, const_cast<char *>(fstring[i].c_str()), FALSE);
        }
    }
}

Phrase::~Phrase() {
    for (vector<Morpheme *>::iterator it = morphemes.begin(); it != morphemes.end(); it++) {
	delete *it;
    }
    morphemes.clear();
}

}
