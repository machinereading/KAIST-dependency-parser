#ifndef PARSER_H
#define PARSER_H

#include "common.h"

namespace Parser {

class Param
{
  public:
    int  mode;
    int  f;
    bool children;
    bool lexical;
    bool phrase;
    bool distance;

    Param() : mode(MODE_TEST), f(6), children(true), lexical(true), phrase(true), distance(true) {};
};

class Parser
{
//    std::vector<Word *> input;     // INPUT is stored in reverse
//    std::vector<Word *> stack;
    std::vector<int>       rel;
//    std::vector<Word *> *org_input;

    Param param;
    Dic *dic;
//    SVM *svm;

    std::vector<std::string> features;

    int OK, ALL, ROOT_OK, ROOT_ALL;
  public:
    Parser();
    Parser(char *);
    ~Parser();

    bool extract_feature();
//    std::string &extract_lexical(Word *);
//    bool push_stack_feature(std::vector<Word *>::size_type);
//    bool push_input_feature(std::vector<Word *>::size_type);
    bool push_children_feature(int);
    bool push_distance_feature();

    void print_feature();
    void print_result();
    void print_accuracy();

    bool parse(Sentence &);
    std::string decide_operation();
    bool manipulate_stacks(const std::string &);
    bool check_rel();
};

}

#endif
