#include "common.h"
#include "sentence.h"
#include "rule.h"
#include "grammar.h"
#include "cky.h"
#include "caseframe.h"
#include "lexeme.h"
#include "parameter.h"
#include "cmdline.h"
#include "dic.h"
#include "parser.h"

void option_proc(cmdline::parser &option, int argc, char **argv) {
    option.add<std::string>("dic", 'd', "dictionary directory", false, "dic");
    option.add<std::string>("rule", 'r', "rule directory", false, "rule");
    option.add<int>("input", 'i', "input type", false, INPUT_IS_MORPHEME);
    option.add<std::string>("output", 'o', "output format", false, "tab");
    option.add<std::string>("language", 'l', "language", false, "generic");
    option.add<int>("beam", 'b', "beam width for search", false, 5);
    option.add("unlexicalized", '\0', "do not use lexical information");
    option.add("strict_pa_checking", '\0', "strictly check predicate-argument relations");
    option.add("debug", '\0', "debug mode");
    option.add("version", 'v', "print version");
    option.add("help", 'h', "print this message");
    option.parse_check(argc, argv);

    if (option.exist("version")) {
        std::cout << "KNP " << VERSION << std::endl;
        exit(0);
    }
}

void print_xml_header() {
    std::cout << "<?xml version=\"1.0\" encoding=\"utf8\"?>" << std::endl
              << "<StandardFormat>" << std::endl
              << "  <Text Type=\"default\">" << std::endl;
}

void print_xml_footer() {
    std::cout << "  </Text>" << std::endl
              << "</StandardFormat>" << std::endl;
}

int main(int argc, char** argv) {
    cmdline::parser option;
    option_proc(option, argc, argv);
    Parser::Parameter param(option.exist("debug"), option.get<std::string>("language"), option.get<int>("input"), option.get<std::string>("output"), option.get<int>("beam"), 
                                       option.get<std::string>("dic"), option.get<std::string>("rule"), 
                                       option.exist("strict_pa_checking"), option.exist("unlexicalized"));
    int sentence_count = 1;

    Parser::CaseFrames cfs(&param);
    // cfs.print();

    Parser::Lexeme lexeme(&param);

    Parser::Grammar g(&param);
    Parser::Rules r(&param);

    std::ifstream is(argv[1]); // input stream
    if (param.output_type == OUTPUT_IS_XML)
        print_xml_header();

    // sentence loop
    while (true) {
	Parser::Sentence sentence(is ? is : std::cin, &param, sentence_count);
        if (param.input_type == INPUT_IS_MORPHEME) {
            if (sentence.get_mrph_num() == 0)
                break;

            if (param.debug_flag) {
                std::cout << "--- Input ---" << std::endl;
                sentence.print();
            }
        }
        else if (param.input_type == INPUT_IS_PHRASE) {
            if (sentence.get_phrase_num() == 0)
                break;
        }

	Parser::CKY cky(&sentence, &r, &cfs, &lexeme, &param);
	cky.parse();

        if (param.debug_flag) {
            std::cout << "--- CKY Result ---" << std::endl;
            cky.print();
            std::cout << "--- Output ---" << std::endl;
        }

	cky.get_best_parse();
        sentence_count++;
    }

    if (param.output_type == OUTPUT_IS_XML)
        print_xml_footer();

    return 0;
}
