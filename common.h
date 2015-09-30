#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <ios>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <climits>
#include "lisp.h"
// #include "morpheme.h"
// #include "dic.h"
// #include "svm.h"

namespace Parser {

#define VERSION "5.0"

enum parser_data_type {
    NONE, 
    IS_TOKEN_DATA, 
    IS_MRPH_DATA, 
    IS_PHRASE_DATA, 
};
typedef enum parser_data_type parser_data_type;

enum cf_type {
    CF_PRED, 
    CF_NOUN, 
};
typedef enum cf_type cf_type;

enum cs_assigned_type {
    UNASSIGNED, 
    ASSIGNED, 
    NIL_ASSIGNED, 
};
typedef enum cs_assigned_type cs_assigned_type;

enum verbose_type {VERBOSE0, VERBOSE1, VERBOSE2, 
		   VERBOSE3, VERBOSE4, VERBOSE5};
typedef enum verbose_type verbose_type;

enum chr_type {CHR_UNKNOWN, CHR_HIRAGANA, CHR_KATAKANA, CHR_FIGURE, 
	       CHR_ALPHABET, CHR_KANJI, CHR_SYMBOL};
typedef enum chr_type chr_type;

const int MODE_TRAIN = 1;
const int MODE_TEST  = 2;

const std::string ACTION_SHIFT  = "SHIFT";
const std::string ACTION_LEFT   = "LEFT";
const std::string ACTION_RIGHT  = "RIGHT";
const std::string ACTION_REDUCE = "REDUCE";

// split function with split_num
template<class T>
inline int split_string(const std::string &src, const std::string &key, T &result, int split_num)
{
    result.clear();
    int len =  src.size();
    int i = 0, si = 0, count = 0;

    while(i < len) {
	while (i < len && key.find(src[i]) != std::string::npos) { si++; i++; } // skip beginning spaces
	while (i < len && key.find(src[i]) == std::string::npos) i++; // skip contents
	if (split_num && ++count >= split_num) { // reached the specified num
	    result.push_back(src.substr(si, len - si)); // push the remainder string
	    break;
	}
	result.push_back(src.substr(si, i - si));
	si = i;
    }

    return result.size();
}

// split function
template<class T>
inline int split_string(const std::string &src, const std::string &key, T &result)
{
    return split_string(src, key, result, 0);
}

// join function
inline std::string join_string(const std::vector<std::string> &str, const std::string &key)
{
    std::string dst;

    for (std::vector<std::string>::const_iterator i = str.begin(); i != str.end(); i++) {
        if (i == str.begin()) {
            dst += *i;
        }
        else {
            dst += key + *i;
        }
    }
    return dst;
}

inline bool is_blank_line(const std::string &str)
{
    for (std::string::size_type i = 0; i < str.size(); i++) {
	if (str[i] == '\n' || 
	    str[i] == '\r' || 
	    str[i] == ' ' || // space
	    str[i] == '	') { // tab
	    ;
	}
	else {
	    return 0;
	}
    }

    return 1;
}

extern "C" int atoi(const char *);
extern "C" double atof(const char *);

inline int atoi(const std::string &str) {
    return atoi(str.c_str());
}

inline double atof(const std::string &str) {
    return atof(str.c_str());
}

// from lisp.c
extern "C" int s_feof(FILE *fp);
extern "C" CELL *s_read(FILE *fp);
extern "C" CELL *car(CELL *cell);
extern "C" CELL *cdr(CELL *cell);
extern "C" CELL *cons(void *car, void *cdr);
extern "C" int length(CELL *list);
extern "C" void error_in_lisp(void);
extern "C" void *my_alloc(int n);

extern chr_type check_str_type(std::string &str);
extern size_t str_length(std::string &str);

}

// int to string
template<class T>
inline std::string int2string(const T i)
{
    std::ostringstream o;

    o << i;
    return o.str();

    /*
    string ret;
    try {
        ret = boost::lexical_cast<string>(i);
    }
    catch (boost::bad_lexical_cast &e) {
        cerr << "Bad cast: " << e.what() << endl;
    }
    return ret;
    */
}

// Convert an integer to its string representation.
// std::string int2str(int i) {
//     std::stringstream ss;
//     ss << std::setfill('0') << std::setw(6) << i;
//    return ss.str();
// }

#endif
