#ifndef HASH_H
#define HASH_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "cdbpp.h"

namespace Parser {

using std::string;

class Dbm {
    bool available;
    string dbname;
    cdbpp::cdbpp *db;
  public:
    Dbm(const string &in_dbname) {
	dbname = in_dbname;
	open();
    }
    Dbm() {
	;
    }
    bool open();
    bool open(const string &in_dbname) {
	dbname = in_dbname;
	return open();
    }
    bool is_open();
    string get(const string &key);
};

}

#endif
