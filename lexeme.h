#ifndef LEXEME_H
#define LEXEME_H

#include "common.h"
#include "parameter.h"
#include "darts.h"
#include "dbm.h"

namespace Parser {

class Lexeme {
    Parameter *param;
    bool exists;
    Darts::DoubleArray rep2id;
    Dbm *id2lex;
  public:
    Lexeme(Parameter *in_param);
    ~Lexeme();

    string get_lex(const int id);

    size_t search_ld(char *str, size_t *result_lengths, size_t *result_values);
    int traverse_ld(const char *str, size_t &node_pos, size_t key_pos, size_t key_length);
};

}

#endif
