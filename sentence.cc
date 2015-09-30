#include "common.h"
#include "sentence.h"

namespace Parser {

using std::vector;
using std::string;

Sentence::Sentence(std::istream &is, Parameter *in_param, int sentence_count) {
    param = in_param;
    count = sentence_count;
    if (param->input_type == INPUT_IS_MORPHEME) {
        read_morphemes(is);
    }
    else if (param->input_type == INPUT_IS_PHRASE) {
        read_phrases(is);
    }

    // sent the sentence id
    set_id();
}

Sentence::~Sentence() {
    for (vector<Morpheme *>::iterator it = morphemes.begin(); it != morphemes.end(); it++) {
	delete *it;
    }
    morphemes.clear();
}

void Sentence::set_id() {
    std::string::size_type pos;
    if (comment_line.length() > 0 && // extract id from comment_line
        (pos = comment_line.find("S-ID:")) != std::string::npos) {
        pos += 5; // "S-ID:"
        std::string::size_type end_pos = comment_line.find(" ", pos);
        if (end_pos == std::string::npos) // end of the line
            id = comment_line.substr(pos, comment_line.length() - pos);
        else
            id = comment_line.substr(pos, end_pos - pos);
    }
    else {
        id = int2string(count);
    }
}

void Sentence::read_morphemes(std::istream &is) {
    string buffer;
    int token_end = 0, token_start = 0, byte_start = 0;

    while (getline(is, buffer)) {
	if (is_blank_line(buffer) || buffer.at(0) == '@') {
	    continue;
	}
	else if (buffer.at(0) == '#') { // comment line
	    comment_line += buffer;
	    continue;
	}
	else if (buffer == "EOS") { // End Of Sentence
	    break;
	}

	Morpheme *mrph_ptr = new Morpheme(buffer, byte_start, token_start);
	mid2index[mrph_ptr->get_id()] = morphemes.size();
	morphemes.push_back(mrph_ptr);
	if (token_end < mrph_ptr->get_token_end()) {
	    token_end = mrph_ptr->get_token_end();
	}
	token_start = mrph_ptr->get_token_start();
	byte_start = mrph_ptr->get_byte_start();
    }
    token_num = token_end + 1;
    set_forward_id_to_morphemes();
}

void Sentence::read_phrases(std::istream &is) {
    string buffer;

    while (getline(is, buffer)) {
	if (is_blank_line(buffer)) {
	    continue;
	}
	else if (buffer.at(0) == '#') { // comment line
	    comment_line += buffer;
	    continue;
	}
	else if (buffer == "EOS") { // End Of Sentence
	    break;
	}

	Phrase *phrase_ptr = new Phrase(buffer, get_phrase_num());
	phrases.push_back(phrase_ptr);
        token_num = phrase_ptr->get_token_end() + 1;
    }
}

bool Sentence::print() {
    for (vector<Morpheme *>::iterator it = morphemes.begin(); it != morphemes.end(); it++) {
	(*it)->print();
    }
    return true;
}

}
