#ifndef SENTENCE_H
#define SENTENCE_H

#include "common.h"
#include "parameter.h"
#include "morpheme.h"
#include "phrase.h"

namespace Parser {

using std::vector;
using std::string;
using std::map;

class Sentence {
    Parameter *param;
    int count;
    vector<Morpheme *> morphemes;
    vector<Phrase *> phrases;
    map<int, int> mid2index; // index of mrph_id to mrph_num
    int token_num;
    string comment_line;
    string id;
  public:
    Sentence();
    Sentence(std::istream &is, Parameter *in_param, int sentence_count);
    ~Sentence();

    bool print();

    void read_morphemes(std::istream &is);
    void read_phrases(std::istream &is);

    vector<Morpheme *> &get_morphemes() {
	return morphemes;
    }
    vector<Phrase *> &get_phrases() {
	return phrases;
    }
    int get_mrph_num() { // the number of input morphemes
	return morphemes.size();
    }
    int get_phrase_num() { // the number of input phrases
	return phrases.size();
    }
    int get_token_num() {
	return token_num;
    }
    bool set_forward_id_to_morphemes() {
	for (vector<Morpheme *>::iterator m_it = morphemes.begin(); m_it != morphemes.end(); m_it++) {
	    vector<int> bid = (*m_it)->get_backward_id();
	    for (vector<int>::iterator bid_it = bid.begin(); bid_it != bid.end(); bid_it++) {
		if (*bid_it != 0) {
		    morphemes[mid2index[*bid_it]]->set_forward_id((*m_it)->get_id());
		}
	    }
	}
	return true;
    }
    string &get_comment_line() {
	return comment_line;
    }
    void set_id();
    string &get_id() {
	return id;
    }
};

}

#endif
