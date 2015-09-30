#ifndef MORPHEME_H
#define MORPHEME_H

#include "common.h"
#include "knp_const.h"
#include "grammar.h"

namespace Parser {

using std::vector;
using std::string;

extern CLASS	Class[CLASSIFY_NO + 1][CLASSIFY_NO + 1];
extern std::map<std::string, int> POS2POSID;
extern TYPE	Type[TYPE_NO];
extern FORM	Form[TYPE_NO][FORM_NO];

class Morpheme {
    int		id;
    vector<int>	forward_id;
    vector<int>	backward_id;
    int		byte_start;
    int		byte_end;
    int		token_start; // i in CKY
    int		token_end;   // j in CKY
    int		token_num;
    string	goi;
    string	yomi;
    string	genkei;
    string	rep;
    int		pos;
    int		spos;
    int         dpnd_head;
    int		conj_type;
    int		conj_form;
    string	sense;
    // std::vector<std::string> f;
    FEATURE	*f;
    vector<size_t>	ld_node_pos;
    bool sense2f();
    bool set_backward_id(string &str);
public:
    Morpheme();
    Morpheme(std::string &in_genkei, std::string &in_pos, int in_byte_start, int in_token_start);
    Morpheme(std::string &line, int in_byte_start, int in_token_start);
    ~Morpheme() {
	;
    }

    void print();
    void print_xml();
    void short_print();

    int get_id() {
	return id;
    }
    vector<int>	&get_forward_id() {
	return forward_id;
    }
    vector<int>	&get_backward_id() {
	return backward_id;
    }
    bool set_forward_id(int fid) {
	forward_id.push_back(fid);
	return true;
    }
    bool check_id(vector<int> &valid_id) {
	if (valid_id.empty()) { // ANY_ID
	    return true;
	}
	else {
	    for (vector<int>::iterator it = valid_id.begin(); it != valid_id.end(); it++) {
		if (id == *it) {
		    return true;
		}
	    }
	}
	return false;
    }
    int get_byte_start() {
	return byte_start;
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
    std::string &get_goi() {
	return goi;
    }
    std::string &get_yomi() {
	return yomi;
    }
    std::string &get_genkei() {
	return genkei;
    }
    std::string &get_rep() {
	return rep;
    }
    int get_pos() {
	return pos;
    }
    int get_spos() {
	return spos;
    }
    int get_conj_type() {
	return conj_type;
    }
    int get_conj_form() {
	return conj_form;
    }
    U_CHAR *get_pos_name() {
        if (pos == 0)
            return (U_CHAR *)"*";
        else
            return Class[pos][0].id;
    }
    U_CHAR *get_spos_name() {
        if (spos == 0)
            return (U_CHAR *)"*";
        else
            return Class[pos][spos].id;
    }
    U_CHAR *get_conj_type_name() {
        if (conj_type == 0)
            return (U_CHAR *)"*";
        else
            return Type[conj_type].name;
    }
    U_CHAR *get_conj_form_name() {
        if (conj_type == 0 || conj_form == 0)
            return (U_CHAR *)"*";
        else
            return Form[conj_type][conj_form].name;
    }
    std::string &get_sense() {
	return sense;
    }
    FEATURE *get_f() {
	return f;
    }
    FEATURE **get_fp() {
	return &f;
    }
    char *make_rep(FEATURE *fp);

    std::vector<size_t> *get_ld_node_pos() {
        return &ld_node_pos;
    }
    void push_back_ld_node_pos(size_t in_node_pos) {
        ld_node_pos.push_back(in_node_pos);
    }
    void clear_ld_node_pos() {
	ld_node_pos.clear();
    }
};

}

#endif
