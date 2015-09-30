#ifndef PARAMETER_H
#define PARAMETER_H

#include "knp_const.h"
#include "common.h"

namespace Parser {

class Parameter {
  public:
    bool debug_flag;
    bool strict_pa_checking_flag;
    bool unlexicalized_flag;
    int beam_width;

    int language;
    bool head_final_flag;

    int input_type;
    int output_type;

    std::string dic_dirname;
    std::string rule_dirname;

    std::string cf_filename;
    std::string case_prob_filename;
    std::string noun_co_prob_filename;
    std::string pred_co_prob_filename;
    std::string rep2id_filename;
    std::string id2lex_filename;

    std::string juman_grammar_filename;
    std::string juman_katuyou_filename;

    Parameter(const bool in_debug, const std::string &in_language, const int in_input, const std::string &in_output, const int in_beam, 
              const std::string &in_dic, const std::string &in_rule, 
              const bool in_strict_pa_checking, const bool in_unlexicalized) {
        debug_flag = in_debug;
        strict_pa_checking_flag = in_strict_pa_checking;
        unlexicalized_flag = in_unlexicalized;
        beam_width = in_beam;

        language = in_language == "japanese" ? LANGUAGE_IS_JAPANESE : LANGUAGE_IS_GENERIC;
        head_final_flag = (in_language == "japanese" || in_language == "korean") ? true : false;

        input_type = in_input;
        output_type = in_output == "xml" ? OUTPUT_IS_XML : in_output == "tree" ? OUTPUT_IS_TREE : OUTPUT_IS_TAB;

        dic_dirname = in_dic;
        rule_dirname = in_rule;

        cf_filename = dic_dirname + "/cf.knpdict";
        case_prob_filename = dic_dirname + "/case.prob";
        noun_co_prob_filename = dic_dirname + "/noun_co.cdb";
        pred_co_prob_filename = dic_dirname + "/pred_co.cdb";
        rep2id_filename = dic_dirname + "/rep2id.da";
        id2lex_filename = dic_dirname + "/id2lex.db";

        juman_grammar_filename = dic_dirname + "/JUMAN.grammar";
        juman_katuyou_filename = dic_dirname + "/JUMAN.katuyou";
    }
};

}

#endif
