#include "caseframe.h"

namespace Parser {

extern char *check_feature(FEATURE *fp, const char *fname);

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;
using std::map;

bool CaseFrame::str2feature(string &str) {
    if (str.empty()) {
	return false;
    }

    vector<string> f_vector;
    split_string(str, " ", f_vector);
    for (vector<string>::iterator it = f_vector.begin(); it != f_vector.end(); it++) {
	feature.push_back(*it);
    }
    return true;
}

bool CaseSlot::set_pp(string &str) {
    pp.insert(str);
    return true;
}

bool CaseSlot::set_pp(const char *cp) {
    string in_str = cp;
    return set_pp(in_str);
}

// bool CaseSlot::set_pp(string &str, int id) {
//     pp[str] = id;
//     return true;
// }

bool CaseSlot::process_and_set_pp(string &str) {
    if (str.empty()) {
	return false;
    }

//    vector<string> pp_vector;
//     split_string(str, "／", pp_vector);
//     for (vector<string>::iterator it = pp_vector.begin(); it != pp_vector.end(); it++) {
// 	set_pp(*it, 0);
//     }

    string::size_type end = str.length();
    string::size_type pos = str.find("*"); // adjacent
    if (pos != string::npos) {
	adjacent = true;
	end = pos;
    }
    else {
	adjacent = false;
    }

    pos = str.find("＊"); // voluntary slot
    if (pos != string::npos) {
	oblig = false;
	end = pos;
    }
    else {
	oblig = true;
    }

    string modified_str = str.substr(0, end);
    set_pp(modified_str);
    return true;
}

bool CaseSlot::set_ex(string &str, int str_freq) {
    ex[str] = str_freq;
    freq += str_freq; // total
    return true;
}

bool CaseSlot::set_ex(const char *cp, int str_freq) {
    string str = cp;
    return set_ex(str, str_freq);
}

// for input bp_ptr
bool CaseSlot::set_ex(Phrase *bp_ptr) {
    if (check_feature(bp_ptr->get_f(), "SENT")) { // sentential complement
	return set_ex("<SENT>", 1); // frequency = 1
    }
    else if (check_feature(bp_ptr->get_f(), "AGENT")) { // agent
	return set_ex("<AGENT>", 1); // frequency = 1
    }
    else if (check_feature(bp_ptr->get_f(), "PER")) { // person
	return set_ex("<PER>", 1); // frequency = 1
    }
    else if (check_feature(bp_ptr->get_f(), "LOC")) { // location
	return set_ex("<LOC>", 1); // frequency = 1
    }
    else if (check_feature(bp_ptr->get_f(), "TIME")) { // time
	return set_ex("<TIME>", 1); // frequency = 1
    }
    else if (check_feature(bp_ptr->get_f(), "QUANT")) { // quantitiy
	return set_ex("<QUANT>", 1); // frequency = 1
    }
    else {
	return set_ex(bp_ptr->get_rep(), 1); // frequency = 1
    }
}

bool CaseSlot::set_gex(string &str, int str_freq) {
    ex[str] = str_freq;
    return true;
}

bool CaseSlot::set_gex(const char *cp, int str_freq) {
    string str = cp;
    return set_gex(str, str_freq);
}

// for input bp_ptr
bool CaseSlot::set_gex(Phrase *bp_ptr) {
    if (check_feature(bp_ptr->get_f(), "SENT")) { // sentential complement
	return set_gex("<SENT>", 1); // frequency = 1
    }
    else {
	return set_gex("", 0);
    }
}

bool CaseSlot::process_and_set_ex(string &str) {
    if (str.empty() || str == "nil") {
	return false;
    }

    vector<string> ex_vector;
    split_string(str, " ", ex_vector);
    for (vector<string>::iterator it = ex_vector.begin(); it != ex_vector.end(); it++) {
	string::size_type pos = (*it).find_last_of(":");
	if (pos != string::npos) {
	    string key = (*it).substr(0, pos);
	    set_ex(key, atoi((*it).substr(pos + 1)));
	}
    }
    return true;
}

bool CaseSlot::print() {
    if (assigned) {
	assigned->print();
	cout << " ";
    }

    // case
    for (set<string>::iterator it = pp.begin(); it != pp.end(); it++) {
	cout << *it << ": ";
    }

    cout << assigned->get_score() << " ";

    // arguments
    int count = 0;
    for (map<string, int>::reverse_iterator it = ex.rbegin(); it != ex.rend() && count < 5; it++, count++) {
	cout << (*it).first << ":" << (*it).second << " ";
    }
    cout << endl;
    return true;
}


CaseFrame::CaseFrame(cf_type in_type, string &in_cf_id, string &in_read, string &in_entry) {
    type = in_type;
    freq = 0;
    cf_id = in_cf_id;
    read = in_read;
    entry = in_entry;
    // voice
}

bool CaseFrame::print() {
    cout << "ID " << cf_id << endl;
    for (vector<CaseSlot *>::iterator it = cs.begin(); it != cs.end(); it++) {
	(*it)->print();
    }
    return true;
}

CaseFrame::~CaseFrame() {
    cf_id.erase();
    pred_type.erase();
    entry.erase();
    read.erase();
    for (vector<string>::iterator it = feature.begin(); it != feature.end(); it++) {
	(*it).erase();
    }
    for (vector<CaseSlot *>::iterator it = cs.begin(); it != cs.end(); it++) {
	delete *it;
    }
}

CaseFrames::CaseFrames(Parameter *in_param) {
    param = in_param;

    read_case_prob(param->case_prob_filename);
    init_db(&noun_co_prob);
    init_db(&pred_co_prob);

    if (param->unlexicalized_flag == false) {
        read_caseframes(param->cf_filename);
        open_db(param->noun_co_prob_filename, noun_co_prob);
        open_db(param->pred_co_prob_filename, pred_co_prob);
    }
}

bool CaseFrames::read_caseframes(const string &filename) {
    std::ifstream dic(filename.c_str());
    if (dic == NULL) {
	return false;
    }

    if (param->debug_flag)
        cerr << "reading " << filename << "... ";

    string buffer, cf_id, read;
    CaseFrame *cf_ptr;
    CaseSlot *cs_ptr;
    while (getline(dic, buffer)) {
	vector<string> line;
	split_string(buffer, " ", line, 2);
	if (line[0] == "ID") {
	    cf_id = line[1];
	    // cout << cf_id << endl;
	}
	else if (line[0] == "読み") {
	    read = line[1];
	}
	else if (line[0] == "表記") {
	    // store case frames for each predicate
	    cf_ptr = new CaseFrame(CF_PRED, cf_id, read, line[1]);
	    cf[line[1]].push_back(cf_ptr);
	}
	else if (line[0] == "素性") {
	    cf_ptr->str2feature(line[1]);
	}
	else if (line[0].substr(0, strlen("格")) == "格") {
	    cs_ptr = new CaseSlot(CF_PRED);
	    cf_ptr->set_cs(cs_ptr);
	    cs_ptr->process_and_set_pp(line[1]);
	}
	else if (line[0].substr(0, strlen("用例")) == "用例") {
	    cs_ptr->process_and_set_ex(line[1]);
	    cf_ptr->set_freq(cs_ptr->get_freq()); // frequency of this case frame == maximum frequency among case slots
	}
	else if (line[0].substr(0, strlen("意味")) == "意味") {
	    // cs_ptr->process_and_set_gex(line[1]);
	}
    }

    dic.close();
    if (param->debug_flag)
        cerr << "done." << endl;
    return true;
}

bool CaseFrames::read_case_prob(const string &filename) {
    std::ifstream dic(filename.c_str());
    if (dic == NULL) {
	return false;
    }

    if (param->debug_flag)
        cerr << "reading " << filename << "... ";

    string buffer;
    while (getline(dic, buffer)) {
	vector<string> line;
	split_string(buffer, " ", line, 2);
	case_prob[line[0]] = atof(line[1]);
    }

    dic.close();
    if (param->debug_flag)
        cerr << "done." << endl;
    return true;
}

void CaseFrames::init_db(Dbm **db) {
    *db = new Dbm();
}

bool CaseFrames::open_db(const string &filename, Dbm *db) {
    if (param->debug_flag)
        cerr << "opening " << filename << "... ";
    db->init(filename); // keymap file or basename
    if (db->is_open()) {
        if (param->debug_flag)
            cerr << "done." << endl;
	return true;
    }
    else {
        if (param->debug_flag)
            cerr << "failed." << endl;
	return false;
    }
}

bool CaseFrames::print() {
    for (map<string, vector<CaseFrame *> >::iterator pred_it = cf.begin(); pred_it != cf.end(); pred_it++) {
	cout << "Entry: " << (*pred_it).first << endl;
	for (vector<CaseFrame *>::iterator cf_it = (*pred_it).second.begin(); cf_it != (*pred_it).second.end(); cf_it++) {	
	    (*cf_it)->print();
	}
    }
    return true;
}

CaseFrames::~CaseFrames() {
    ;
}

vector<CaseFrame *> *CaseFrames::get_caseframes(string &key) {
    if (cf.find(key) != cf.end()) { // exist
	return &(cf[key]);
    }
    else {
	return NULL;
    }
}

}
