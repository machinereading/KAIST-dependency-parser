#include "common.h"

using std::ifstream;
using std::string;
using std::vector;
using std::map;

using std::cerr;
using std::endl;

Dic::Dic() {
    ;
}

Dic::Dic(const char *base) {
    for (int i = 0; ; i++) {
	string filename(base);
	filename += "_";
	filename += int2string(i);
	filename += ".dic";

	ifstream dic_is(filename.c_str());
	if (dic_is == NULL) {
	    break; // end
	}

	cerr << "DEBUG: open " << filename << endl;

	map<string, int> *tmp_dic = new map<string, int>;
	string buffer;
	while (getline(dic_is, buffer)) {
	    vector<string> line;
	    split_string(buffer, " ", line);
	    (*tmp_dic)[line[0]] = atoi(static_cast<const char *>(line[1].c_str()));
	}
	dic.push_back(tmp_dic);

	dic_is.close();
    }
}

Dic::~Dic() {
    ;
}

string Dic::find(int n, string &key) {
    map<string, int>::iterator it;
    if ((it = dic[n]->find(key)) == dic[n]->end()) {
	it = dic[n]->find(UNKNOWN);
    }

    return int2string(it->second);
}
