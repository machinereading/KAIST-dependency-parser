#include "common.h"
#include "morpheme.h"

namespace Parser {

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;

extern void assign_cfeature(FEATURE **fpp, const char *fname, int temp_assign_flag);
extern char *check_feature(FEATURE *fp, const char *fname);

void replace_string(std::string &str, const char *from, const char *to) {
    std::string::size_type pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, strlen(from), to);
        pos += strlen(to);
    }
}

std::string print_str_for_xml(const std::string &in_str) {
    std::string out_str = in_str;
    replace_string(out_str, "&", "&amp;");
    replace_string(out_str, "<", "&lt;");
    replace_string(out_str, ">", "&gt;");
    replace_string(out_str, "\"", "&quot;");
    replace_string(out_str, "'", "&apos;");
    return out_str;
}

bool Morpheme::set_backward_id(string &str) {
    vector<string> num_vector;
    split_string(str, ";", num_vector);
    for (vector<string>::iterator it = num_vector.begin(); it != num_vector.end(); it++) {
	backward_id.push_back(atoi(*it));
    }
    return true;
}

char *Morpheme::make_rep(FEATURE *fp) {
    // if (char *cp = check_feature(fp, "RF")) {
    if (char *cp = check_feature(fp, "代表表記")) {
	// rep = cp + strlen("RF:");
        rep = cp + strlen("代表表記:");
	return cp;
    }
    else {
	// make it?
	return NULL;
    }
}

Morpheme::Morpheme(string &line, int in_byte_start, int in_token_start) {
    vector<string> input;

    if (split_string(line, " ", input, 16) < 9) { // 16 for Japanese
	cerr << "Invalid input: " << line << endl;
	return;
    }

    id = atoi(input[0]);
    set_backward_id(input[1]);
    byte_start = atoi(input[2]);
    byte_end = atoi(input[3]);
    goi = input[4];
    yomi = input[5];
    genkei = input[6];
    f = NULL;
    if (input.size() >= 16) { // for Japanese
	token_start = byte_start / 3;
	token_end = byte_end / 3 - 1;
	token_num = token_end - token_start + 1;
	pos = atoi(input[8]);
	spos = atoi(input[10]);
	conj_type = atoi(input[12]);
	conj_form = atoi(input[14]);
	sense = input[15];
	sense2f();
	make_rep(f);
    }
    else { // for English
	token_start = byte_start == in_byte_start ? in_token_start : in_token_start + 1;
	token_end = token_start;
	token_num = 1;
	pos = atoi(input[8]);
	spos = atoi(input[10]);
	conj_type = 0;
	conj_form = 0;
    }
}

// Constructor for the phrase input
Morpheme::Morpheme(std::string &in_genkei, std::string &in_pos, int in_token_start, int in_token_end) {
    id = -1;

    genkei = in_genkei;
    goi = in_genkei;
    rep = in_genkei;

    // set POS
    if (POS2POSID.find(in_pos) == POS2POSID.end()) { // not found
        std::cerr << ";; Invalid POS: " << in_pos << endl;
        pos = 0;
    }
    else
        pos = POS2POSID[in_pos];
    spos = 0;
    conj_type = 0;
    conj_form = 0;

    token_start = in_token_start;
    token_end = in_token_end;
    f = NULL;
}

bool Morpheme::sense2f() {
    if (sense.empty() || sense == "NIL") {
	return false;
    }

    // skip double quotations at the beginning and the end
    int start = sense.at(0) == '"' ? 1 : 0;
    int end = sense.size() - 1;
    if (sense.at(end) == '"') {
	end -= 1;
    }

    vector<string> sense_vector;
    split_string(sense.substr(start, end), " ", sense_vector);
    for (vector<string>::iterator it = sense_vector.begin(); it != sense_vector.end(); it++) {
	assign_cfeature(&f, (char *)(*it).c_str(), FALSE);
    }
    return true;
}

void Morpheme::print() {
    cout << goi;
    if (yomi.size()) {
	cout << " " << yomi;
    }
    if (genkei.size()) {
	cout << " " << genkei;
    }
    cout << " " << get_pos_name() << " " << pos;
    cout << " " << get_spos_name() << " " << spos;
    cout << " " << get_conj_type_name() << " " << conj_type;
    cout << " " << get_conj_form_name() << " " << conj_form;
//     if (sense.size()) {
// 	cout << " " << sense;
//     }

    cout << " ";
    if (f)
        f->print();
    cout << endl;
}

void Morpheme::print_xml() {
    cout << "          <word";
    if (id >= 0)
        cout << " id=\"" << id << "\"";
    if (f) {
        cout << " content_p=\"";
        if (check_feature(f, "CONT"))
            cout << 1;
        else
            cout << 0;
        cout << "\"";
    }
        
    cout << " str=\"" << print_str_for_xml(goi) << "\" lem=\"" << print_str_for_xml(genkei) << "\"";

    if (yomi.length() > 0)
        cout << " read=\"" << print_str_for_xml(yomi) << "\"";

    cout << " pos=\"" << get_pos_name();
    if (spos != 0)
        cout << ":" << get_spos_name();
    cout << "\" repname=\"";
    if (rep.length() > 0)
        cout << print_str_for_xml(rep);
    cout << "\"";
    if (conj_type != 0) {
        cout << " conj=\"";
        cout << get_conj_type_name();
        if (conj_form != 0)
            cout << ":" << get_conj_form_name();
        cout << "\"";
    }
    if (f) {
        cout << " feature=\"";
        f->print_xml();
        cout << "\"";
    }
    cout << "/>" << endl;;
}

void Morpheme::short_print() {
    cout << goi;
    if (rep.length() > 0)
        cout << " (" << rep << ")";
    cout << " ";
    if (f)
        f->print();
    cout << endl;
}

}
