#include "common.h"
#include "lexeme.h"

namespace Parser {

Lexeme::Lexeme(Parameter *in_param) {
    param = in_param;

    if (param->debug_flag)
        std::cerr << "opening " << param->rep2id_filename << "... ";
    if (rep2id.open(param->rep2id_filename.c_str()) == -1) {
        if (param->debug_flag)
            std::cerr << "failed." << endl;
        exists = false;
    }
    else {
        if (param->debug_flag) {
            std::cerr << "done." << endl;
            std::cerr << "opening " << param->id2lex_filename << "... ";
        }
        exists = true;
        id2lex = new Dbm(param->id2lex_filename); // keymap file or basename
        if (param->debug_flag)
            std::cerr << "done." << endl;
    }
}

Lexeme::~Lexeme() {
    if (exists) {
        rep2id.clear();
        delete id2lex;
    }
}

std::string Lexeme::get_lex(const int id) {
    std::ostringstream oss;
    oss << id;
    std::string key = oss.str();
    return id2lex->get(key);
}

// search the double array with a given str
size_t Lexeme::search_ld(char *str, size_t *result_lengths, size_t *result_values) {
    if (!exists)
        return 0;

    Darts::DoubleArray::result_pair_type result_pair[1024];
    size_t num = rep2id.commonPrefixSearch(str, result_pair, 1024);

    for (size_t i = 0; i < num; i++) { // hit num
        result_lengths[i] = result_pair[i].length;
        result_values[i] = result_pair[i].value; // size: result_pair[i].value & 0xff
                                                 // real value: result_pair[i].value >> 8
    }

    return num;
}

int Lexeme::traverse_ld(const char *str, size_t &node_pos, size_t key_pos, size_t key_length) {
    if (!exists)
        return 0;

    int value = rep2id.traverse(str, node_pos, key_pos, key_length);

    return value;
}

}
