#include "cky.h"

using std::cout;
using std::cerr;
using std::endl;

namespace Parser {

extern char *check_feature(FEATURE *fp, const char *fname);
extern void assign_cfeature(FEATURE **fpp, const char *fname, int temp_assign_flag);

// string -> CKY_cell for diagonal cells (i, i)
bool CKY_cell::set(int in_i_j) {
    exist_flag = true;
    i = in_i_j;
    j = in_i_j;
    dpnd_type = '\0';
    left = NULL;
    right = NULL;
    next = NULL;
    score = 0;
    token_num = 1; // str->size() / BYTES4CHAR;
    mrph_ptr = NULL;
    bp_ptr = NULL;

    return true;
}

// morpheme -> CKY_cell
bool CKY_cell::set(Morpheme *in_mrph_ptr, Rules *rules) {
    exist_flag = true;
    i = in_mrph_ptr->get_token_start();
    j = in_mrph_ptr->get_token_end();
    dpnd_type = '\0';
    left = NULL;
    right = NULL;
    next = NULL;
    score = 0;
    token_num = j - i + 1;
    mrph_ptr = in_mrph_ptr;
    bp_ptr = NULL;

    // mrph_ptr->cky_ptr = cky_ptr;
    // cpm_ptr = NULL;

    // apply unary morpheme rules (to make phrases)
    rules->assign_mrph_feature(mrph_ptr, 1, FALSE);

    // unaryの基本句はここでは作らず、make_phrases_on_cky()で作る
    // cky_ptr->bp_ptr = NULL;

    return true;
}

// phrase -> CKY_cell
bool CKY_cell::set(Phrase *in_phrase_ptr, Rules *rules) {
    exist_flag = true;
    i = in_phrase_ptr->get_token_start();
    j = in_phrase_ptr->get_token_end();
    dpnd_type = '\0';
    left = NULL;
    right = NULL;
    next = NULL;
    score = 0;
    token_num = j - i + 1;
    mrph_ptr = NULL;
    bp_ptr = in_phrase_ptr;

    // apply unary phrase rules
    rules->assign_phrase_feature(bp_ptr, 1, FALSE, FALSE);

    return true;
}

// make an input case frame and do case analysis
bool CKY_cell::case_analysis(CaseFrames *caseframes, Rules *rules) {
    if (check_feature(bp_ptr->get_f(), "PRED")) { // verb, adjective, noun+copula
	cfd = new CaseFrame(CF_PRED, this, caseframes, rules);

	// case analysis
	if (caseframes->find_best_cf(cfd, this, false)) {
            if (caseframes->get_param()->debug_flag) {
                std::cout << "* SCORE: "<< score << " ";
                cfp->print();
            }
	}
	caseframes->pseudo_noun_analysis(cfd, this);
	// cfd->print();
    }
    else {
	cfd = new CaseFrame(CF_NOUN, this, caseframes, rules);
	caseframes->pseudo_noun_analysis(cfd, this);
	cfp = NULL;
    }
    return true;
}

// merge two CKY cells with a dependency relation of dpnd_type
bool CKY_cell::set(CKY_cell *in_left, CKY_cell *in_right, char in_dpnd_type, Rules *rules, CaseFrames *caseframes) {
    exist_flag = true;
    i = in_left->i;
    j = in_right->j;
    dpnd_type = in_dpnd_type;
    left = in_left;
    right = in_right;
    next = NULL;
    token_num = in_left->token_num + in_right->token_num;
    mrph_ptr = NULL;
    if (dpnd_type == 'R' || dpnd_type == 'r' || dpnd_type == 'P' || dpnd_type == 'I' || dpnd_type == 'A') {
	bp_ptr = right->bp_ptr; // right is the head
    }
    else if (dpnd_type == 'L' || dpnd_type == 'l') {
	bp_ptr = left->bp_ptr; // left is the head
    }
    else {
	cerr << ";; Unknown dpnd_type: " << dpnd_type << endl;
	exit(10);
    }

    // 対象以外のスコアを集める (rightをたどりながらleftのスコアを足す)
    score = 0;
    cs_score = 0;
    CKY_cell *cky_ptr = this;
    while (cky_ptr && cky_ptr->get_dpnd_type()) { // dependency
	if (cky_ptr->get_left() && cky_ptr->get_left()->get_bp_ptr()) {
	    score += cky_ptr->get_left()->get_score();
            if (cky_ptr->get_dpnd_type() == 'r') // penalty for relaxed dependency
                score += HARSH_PENALTY_SCORE;
	}
	cky_ptr = cky_ptr->get_right();
    }

    // cout << "(" << i << ", " << j << "): s=" << score << ", cs=" << cs_score << endl; // debug

    // make an input case frame and do case analysis
    case_analysis(caseframes, rules);
    score += caseframes->collect_vp_probs(this, rules);

    return true;
}

// merge two CKY cells by a rule
bool CKY_cell::set(CKY_cell *in_left, CKY_cell *in_right, FEATURE *fp, Rules *rules, CaseFrames *caseframes) {
    // do not make a phrase for the intermediate node (NT-...)
    if (check_feature(fp, "BP")) {
	if (!exist_flag) { // make cky_ptr with bp
	    exist_flag = true;
	    i = in_left->i;
	    j = in_right->j;
	    dpnd_type = '\0';
	    // para_flag = 0;
	    left = in_left;
	    right = in_right;
	    next = NULL;
	    score = 0;
	    token_num = in_left->token_num + in_right->token_num;
	    // chr_ptr = cky_ptr->left->chr_ptr;
	    mrph_ptr = NULL;
	    bp_ptr = new Phrase(in_left, in_right);
	    // apply unary phrase rules
	    rules->assign_phrase_feature(bp_ptr, 1, FALSE, FALSE);
	}
	else if (bp_ptr == NULL) { // cky_ptr with mrph_ptr exists
	    bp_ptr = new Phrase(in_left, in_right);
	    // apply unary phrase rules
	    rules->assign_phrase_feature(bp_ptr, 1, FALSE, FALSE);
	}
	else {
	    cerr << ";; Unknown error!" << endl;
	    exit(10);
	}
	assign_cfeature(bp_ptr->get_fp(), fp->cp, FALSE);
	return true;
    }
    else {
	return false;
    }
}

CKY::CKY(Sentence *in_sentence, Rules *in_rules, CaseFrames *in_caseframes, Lexeme *in_lexeme, Parameter *in_param) : phrase_num(0) {
    param = in_param;
    n = in_sentence->get_token_num();
    sentence = in_sentence;
    rules = in_rules;
    caseframes = in_caseframes;
    lexeme = in_lexeme;

    int table_size = n * (n + 1) / 2;
    cky_table = (CKY_cell **)malloc(sizeof(CKY_cell *) * table_size);
    memset(cky_table, 0, sizeof(CKY_cell *) * table_size);

    if (param->input_type == INPUT_IS_MORPHEME) {
        // project morphemes to CKY
        set_morphemes_to_cky();

        // ** implement me!!
        // lexical_disambiguation_for_all_cky_data();

        assign_eos_feature_to_mrph();

        // assign features of lexical database
        assign_lexical_features_to_morphemes();

        // assign morpheme features to morpheme lattice
        rules->assign_mrph_feature_for_lattice(this, FALSE);

        // make phrases using CFG rules
        make_phrases_on_cky();
    }
    else if (param->input_type == INPUT_IS_PHRASE) {
        // project phrases to CKY
        set_phrases_to_cky();

        assign_eos_feature_to_phrase();
    }

    // assign phrase features to phrase lattice
    rules->assign_phrase_feature_for_lattice(this, FALSE, FALSE);

    // do case analysis on each predicate with no arguments
    case_analysis_on_single_predicate();
}

CKY::~CKY() {
    for (int i = 0; i < n * (n + 1) / 2; i++) {
	if (cky_table[i]) {
	    CKY_cell *cky_ptr = cky_table[i];
	    while (cky_ptr) {
		CKY_cell *next_cky_ptr = cky_ptr->get_next();
		delete cky_ptr;
		cky_ptr = next_cky_ptr;
	    }
	}
    }
    free(cky_table);
}

int CKY::get_base_cky_pos(int i, int j) {
    if (i < n && j < n) {
	return (2 * n - i + 1) * i / 2 + (n - j - 1);
    }
    else {
	cerr << ";; Error in get_base_cky_pos(): called (" << i << ", " << j << ")." << endl;
	return -1;
    }
}

CKY_cell **CKY::get_base_cky_cell_pp(int i, int j) {
    int pos = get_base_cky_pos(i, j);
    if (pos >= 0) {
	return &(cky_table[pos]);
    }
    else {
	return NULL;
    }
}

CKY_cell *CKY::get_base_cky_cell(int i, int j) {
    int pos = get_base_cky_pos(i, j);
    if (pos >= 0) {
	return cky_table[pos];
    }
    else {
	return NULL;
    }
}

CKY_cell *CKY::make_base_cky_cell(int i, int j) {
    int pos = get_base_cky_pos(i, j);
    if (pos >= 0) {
	cky_table[pos] = new CKY_cell;
	return cky_table[pos];
    }
    else {
	return NULL;
    }
}

CKY_cell *CKY::make_new_cky_cell(int i, int j) {
    CKY_cell *cell = get_base_cky_cell(i, j);
    if (cell) {
	return cell->find_and_make_new_cky_cell(0);
    }
    else {
	return make_base_cky_cell(i, j);
    }
}

CKY_cell *CKY::make_new_cky_cell(int i, int j, int k_best_num) {
    CKY_cell *cell = get_base_cky_cell(i, j);
    if (cell) {
	return cell->find_and_make_new_cky_cell(k_best_num);
    }
    else {
	return make_base_cky_cell(i, j);
    }
}

void CKY::sort_cky_cells(int i, int j) {
    CKY_cell **start_cky_ptr_ptr = get_base_cky_cell_pp(i, j);
    CKY_cell *pre_cell, *best_cell = NULL, *best_pre_cell;
    CKY_cell *cell = *start_cky_ptr_ptr;

    double best_score;
    for (int i = 0; i < param->beam_width && cell; i++) { // sort cky cells in linked list by their scores
        best_score = INT_MIN;
        best_pre_cell = pre_cell = NULL;

        while (cell && cell->exist()) {
            if (cell->get_score() > best_score) {
                best_score = cell->get_score();
                best_cell = cell;
                best_pre_cell = pre_cell;
            }
            pre_cell = cell;
            cell = cell->get_next();
        }
        if (best_pre_cell) {
            best_pre_cell->set_next(best_cell->get_next());
            best_cell->set_next(*start_cky_ptr_ptr);
	    *start_cky_ptr_ptr = best_cell;
        }

	start_cky_ptr_ptr = best_cell->get_next_pp();
	cell = best_cell->get_next();
    }

    if (best_cell) {
        CKY_cell *next_cell;
        cell = best_cell->get_next();
        while (cell) { // delete the cells after beam width
            next_cell = cell->get_next();
            delete cell;
            cell = next_cell;
        }
        best_cell->set_next(NULL); // do not consider more candidates than beam width
    }
}

// assign morphemes to CKY cells
bool CKY::set_morphemes_to_cky() {
    for (vector<Morpheme *>::iterator i = sentence->get_morphemes().begin(); i != sentence->get_morphemes().end(); i++) {
	CKY_cell *cell = make_new_cky_cell((*i)->get_token_start(), (*i)->get_token_end());
	cell->set(*i, rules);
    }
    return true;
}

// assign phrases to CKY cells
bool CKY::set_phrases_to_cky() {
    for (vector<Phrase *>::iterator i = sentence->get_phrases().begin(); i != sentence->get_phrases().end(); i++) {
	CKY_cell *cell = make_new_cky_cell((*i)->get_token_start(), (*i)->get_token_end());
	cell->set(*i, rules);
    }
    return true;
}

// make phrases from morpheme cells
bool CKY::make_phrases_on_cky() {
    for (int j = 0; j < n; j++) { // left to right
	for (int i = j; i >= 0; i--) { // bottom to top
	    // unary
	    CKY_cell *cky_ptr = get_base_cky_cell(i, j);
	    while (cky_ptr && cky_ptr->exist()) {
		rules->check_possibility_and_make_bp(cky_ptr, cky_ptr, (CKY_cell *)NULL, caseframes);
		cky_ptr = cky_ptr->get_next();
	    }

	    // binary: try to merge (i, i + k) and (i + k + 1, j) to make a phrase
	    for (int k = 0; k < j - i; k++) {
		CKY_cell *left_ptr = get_base_cky_cell(i, i + k);
		while (left_ptr) {
		    CKY_cell *right_ptr = get_base_cky_cell(i + k + 1, j);
		    while (right_ptr) {
			// make a phrase if possible
			rules->check_possibility_and_make_bp(make_new_cky_cell(i, j), left_ptr, right_ptr, caseframes);
			right_ptr = right_ptr->get_next();
		    }
		    left_ptr = left_ptr->get_next();
		}
	    }
	}
    }
    return true;
}

bool CKY::print() {
    for (int j = 0; j < n; j++) { /* left to right */
	for (int i = j; i >= 0; i--) { /* bottom to top */
	    CKY_cell *cky_ptr = get_base_cky_cell(i, j);
            if (cky_ptr && cky_ptr->exist())
                cout << "(" << i << ", " << j << ")" << endl;
	    while (cky_ptr && cky_ptr->exist()) {
		if (cky_ptr->get_mrph_ptr()) { // morpheme
		    cout << "	M ";
		    cky_ptr->get_mrph_ptr()->short_print();
		}
		if (cky_ptr->get_bp_ptr()) {
		    
		    if (cky_ptr->get_dpnd_type()) { // dependency
			cout << "	" << cky_ptr->get_dpnd_type() << " " << cky_ptr->get_score() << " ";
			cout << "(" << cky_ptr->get_left()->get_token_start() << ", " << cky_ptr->get_left()->get_token_end() << ")" << " ";
			cout << "(" << cky_ptr->get_right()->get_token_start() << ", " << cky_ptr->get_right()->get_token_end() << ")" << endl;
		    }
		    else { // basic phrase
			cout << "	B " << cky_ptr->get_score() << " ";
			cky_ptr->get_bp_ptr()->print();
		    }
		}
		// cout << " " << cky_ptr->get_str();
		cky_ptr = cky_ptr->get_next();
	    }
	}
    }
    return true;
}

bool CKY::get_best_parse() {
    double best_score = INT_MIN;
    CKY_cell *best_cky_ptr = NULL;

    // get the cky_ptr of the whole sentence
    CKY_cell *cky_ptr = get_base_cky_cell(0, n - 1);
    while (cky_ptr && cky_ptr->exist()) {
	if (best_score < cky_ptr->get_score()) {
	    best_score = cky_ptr->get_score();
	    best_cky_ptr = cky_ptr;
	}
	cky_ptr = cky_ptr->get_next();
    }

    if (best_cky_ptr) {
        if (param->debug_flag)
            cerr << ";; best_score = " << best_score << endl;
	assign_phrase_num(best_cky_ptr);
	print_bp_dependencies(best_cky_ptr, best_score, true);
	return true;
    }
    else {
        if (param->debug_flag)
            cerr << ";; parse failed" << endl;
        print_bp_dependencies(best_cky_ptr, best_score, false);
	return false;
    }
}

bool CKY::parse() {
    for (int j = 0; j < n; j++) { // left to right
        for (int i = j; i >= 0; i--) { // bottom to top
	    // cerr << "(" << i << ", " << j << ")" << endl;
	    if (i == j) {
		// cky_ptr->set(i); // if the input is character
		continue;
	    }
	    else {
		// try to merge (i, i + k) and (i + k + 1, j) to make a dependency
                for (int k = j - i - 1; k >= 0; k--) {
		// for (int k = 0; k < j - i; k++) {
		    CKY_cell *left_ptr = get_base_cky_cell(i, i + k);
                    while (left_ptr) {
			CKY_cell *right_ptr = get_base_cky_cell(i + k + 1, j);
			while (right_ptr) {
			    if (char dpnd_type = rules->check_dpnd_possibility(left_ptr, right_ptr)) {
				// cerr << "\t" << dpnd_type << " (" << i << ", " << i + k << ") (" << i + k + 1 << ", " << j << ")" << endl;
				CKY_cell *new_ptr = make_new_cky_cell(i, j);
                                if (new_ptr == NULL) {
                                    goto OUT_OF_LEFT_LOOP;
                                }
				new_ptr->set(left_ptr, right_ptr, dpnd_type, rules, caseframes);
				// cerr << "\t\t => " << new_ptr->get_score() << endl;
			    }
			    right_ptr = right_ptr->get_next();
			}
			left_ptr = left_ptr->get_next();
		    }
                  OUT_OF_LEFT_LOOP:
                    ;
		}
                sort_cky_cells(i, j);
	    }
	}
    }
    return true;
}

// assign EOS feature to morphemes
bool CKY::assign_eos_feature_to_mrph() {
    for (int i = 0; i < n; i++) {
	CKY_cell *cky_ptr = get_base_cky_cell(i, n - 1);
	while (cky_ptr && cky_ptr->exist() && cky_ptr->get_mrph_ptr()) {
	    assign_cfeature(cky_ptr->get_mrph_ptr()->get_fp(), "EOS", FALSE);
	    cky_ptr = cky_ptr->get_next();
	}
    }
    return true;
}

// assign EOS feature to phrases
bool CKY::assign_eos_feature_to_phrase() {
    for (int i = 0; i < n; i++) {
	CKY_cell *cky_ptr = get_base_cky_cell(i, n - 1);
	while (cky_ptr && cky_ptr->exist() && cky_ptr->get_bp_ptr()) {
	    assign_cfeature(cky_ptr->get_bp_ptr()->get_fp(), "EOS", FALSE);
	    cky_ptr = cky_ptr->get_next();
	}
    }
    return true;
}

// do case analysis on each predicate with no arguments
bool CKY::case_analysis_on_single_predicate() {
    for (int j = 0; j < n; j++) { /* left to right */
	for (int i = j; i >= 0; i--) { /* bottom to top */
	    CKY_cell *cky_ptr = get_base_cky_cell(i, j);
	    while (cky_ptr && cky_ptr->exist() && cky_ptr->get_bp_ptr()) {
		// make an input case frame and do case analysis
		cky_ptr->case_analysis(caseframes, rules);
		cky_ptr = cky_ptr->get_next();
	    }
	}
    }
    return true;
}

// assign the phrase number
bool CKY::assign_phrase_num(CKY_cell *cky_ptr) {
    if (cky_ptr->get_left() && cky_ptr->get_right() && 
	cky_ptr->get_left()->get_bp_ptr() && cky_ptr->get_right()->get_bp_ptr()) {
	assign_phrase_num(cky_ptr->get_left());
	assign_phrase_num(cky_ptr->get_right());
    }
    else {
	cky_ptr->get_bp_ptr()->set_num(phrase_num++);
    }
    return true;
}

bool CKY::print_bp_dependencies(CKY_cell *cky_ptr, double score, bool is_success) {
    /*
    // if comment line does not exist
    if (!Show_Opt_tag[0]) {
	fprintf(Outfp, "# S-ID:%d\n", Sen_num);
    }
    */

    // print dependency structure
    if (param->output_type == OUTPUT_IS_XML) {
        cout << "    <S Id=\"" << sentence->get_id() << "\">" << endl;
        if (is_success)
            cout << "      <Annotation>" << endl;
    }
    else if (sentence->get_comment_line().length() > 0)
        cout << sentence->get_comment_line() << endl;

    if (is_success)
        print_bp_dependency(cky_ptr, 'R', -1);

    if (param->output_type == OUTPUT_IS_XML) {
        if (is_success)
            cout << "      </Annotation>" << endl;
        cout << "    </S>" << endl;
    }
    else if (param->output_type == OUTPUT_IS_TREE) {
        if (is_success)
            cky_ptr->get_bp_ptr()->draw_tree();
    }
    else
        cout << "EOS" << endl;

    // print case structure
    /*
    cout << "<Case Structure Analysis Data>" << endl;
    memset(printed_case_structure_flag, 0, TAG_MAX);
    print_case_structure(cky_ptr);
    cout << "</Case Structure Analysis Data>" << endl;
    */
    return true;
}

bool CKY::print_bp_dependency(CKY_cell *cky_ptr, char dpnd_type, int dpnd_head) {
    if (cky_ptr->get_left() && cky_ptr->get_right() && 
	cky_ptr->get_left()->get_bp_ptr() && cky_ptr->get_right()->get_bp_ptr() && 
        cky_ptr->get_dpnd_type()) { // check the existence of dependency for bp -> bp
	print_bp_dependency(cky_ptr->get_left(), cky_ptr->get_dpnd_type(), cky_ptr->get_right()->get_bp_ptr()->get_num());
	print_bp_dependency(cky_ptr->get_right(), dpnd_type, dpnd_head);
        cky_ptr->get_bp_ptr()->push_back_child(cky_ptr->get_left()->get_bp_ptr());
        // cerr << cky_ptr->get_left()->get_bp_ptr()->get_num() << " -> " << cky_ptr->get_bp_ptr()->get_num() << endl;
    }
    else {
	// phrase
	Phrase *bp_ptr = cky_ptr->get_bp_ptr();
        if (param->output_type == OUTPUT_IS_XML)
            bp_ptr->print_xml(dpnd_type, dpnd_head);
        else if (param->output_type == OUTPUT_IS_TAB)
            bp_ptr->print_tab(dpnd_type, dpnd_head);
    }
    return true;
}

void CKY::assign_features_of_ld_from_id(Morpheme *mrph_ptr, unsigned int id) {
    std::string value = lexeme->get_lex(id);
    if (value.size() > 0) { // key exists
        int offset;
        char fname[DATA_LEN];
        std::vector<std::string> fstr;
        split_string(value, "|", fstr);
        for (std::vector<std::string>::iterator it = fstr.begin(); it != fstr.end(); it++) {
            int match_num = sscanf((*it).c_str(), "f:%d:%s", &offset, fname);
            if (match_num == 2) {
                // sprintf(buf, "%s:%d-%d", fname, start_mrph_num, end_mrph_num);
                assign_cfeature(mrph_ptr->get_fp(), fname, FALSE);
            }
        }
    }
}

void CKY::ld_traverse_one_morpheme(Morpheme *mrph_ptr, size_t node_pos) {
    std::string key;
    if (node_pos == 0) // at ROOT
        key = mrph_ptr->get_rep();
    else
        key = "+" + mrph_ptr->get_rep();

    int value = lexeme->traverse_ld(key.c_str(), node_pos, 0, 0);
    if (value == -1 || value > 0) { // no entry here but intermediate node, or entries found at this position
        mrph_ptr->push_back_ld_node_pos(node_pos);
        if (value > 0) { // process the retrieved entries
            unsigned int entry_num = value & 0xff;
            unsigned int start_id = value >> 8;
            for (unsigned int i = 0; i < entry_num; i++) {
                assign_features_of_ld_from_id(mrph_ptr, start_id + i);
            }
        }
    }
}

// assign features of lexical database
void CKY::assign_lexical_features_to_morphemes() {
    for (int start = 0; start < n; start++) { // start position to look up TRIE
        for (int i = start; i < n; i++) {
            for (int j = i; j < n; j++) {
                CKY_cell *cky_ptr = get_base_cky_cell(i, j);
                while (cky_ptr && cky_ptr->exist()) {
                    Morpheme *mrph_ptr;
                    if ((mrph_ptr = cky_ptr->get_mrph_ptr())) { // a morpheme (i, j)
                        if (i == start)
                            ld_traverse_one_morpheme(mrph_ptr, 0); // from ROOT
                        else {
                            for (int k = i - 1; k >= start; k--) { // check the preceding morphemes
                                CKY_cell *pre_cky_ptr = get_base_cky_cell(k, i - 1);
                                while (pre_cky_ptr && pre_cky_ptr->exist()) {
                                    Morpheme *pre_mrph_ptr;
                                    if ((pre_mrph_ptr = pre_cky_ptr->get_mrph_ptr())) { // a pre-morpheme (k, i - 1)
                                        // pre_mrph_ptr's ld_node_pos loop -> node_pos
                                        std::vector<size_t> *pre_ld_node_pos = pre_mrph_ptr->get_ld_node_pos();
                                        for (std::vector<size_t>::iterator it = pre_ld_node_pos->begin(); it != pre_ld_node_pos->end(); it++) {
                                            ld_traverse_one_morpheme(mrph_ptr, *it);
                                        }
                                    }
                                    pre_cky_ptr = pre_cky_ptr->get_next();
                                }
                            }
                        }
                    }
                    cky_ptr = cky_ptr->get_next();
                }
            }
        }

        // clear node_pos
        for (int i = start; i < n; i++) {
            for (int j = i; j < n; j++) {
                CKY_cell *cky_ptr = get_base_cky_cell(i, j);
                while (cky_ptr && cky_ptr->exist()) {
                    Morpheme *mrph_ptr;
                    if ((mrph_ptr = cky_ptr->get_mrph_ptr())) { // a morpheme (i, j)
                        mrph_ptr->clear_ld_node_pos();
                    }
                    cky_ptr = cky_ptr->get_next();
                }
            }
        }
    }
}

}
