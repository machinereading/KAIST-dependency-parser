#include "common.h"
#include "hash.h"

namespace Parser {

using std::cout;
using std::cerr;
using std::endl;

bool Dbm::open() {
    // Open the database file for reading (with binary mode)
    std::ifstream ifs(dbname.c_str(), std::ios_base::binary);
    if (ifs.fail()) {
        cerr << "ERROR: Failed to open a database file for reading." << endl;
	available = false;
        return false;
    }

    try {
        // Open the database from the input stream
        db = new cdbpp::cdbpp(ifs);
        if (!db->is_open()) {
            cerr << "ERROR: Failed to read a database file." << endl;
	    available = false;
        }
    } catch (const cdbpp::cdbpp_exception& e) {
        // Abort if something went wrong...
        cerr << "ERROR: " << e.what() << endl;
        available = false;
    }
    available = true;
    return available;
}

bool Dbm::is_open() {
    return available;
}

string Dbm::get(const string &key) {
    size_t vsize;
    string ret_value;
    const char *value = (const char *)db->get(key.c_str(), key.length(), &vsize);
    if (value == NULL) {
	cerr << "ERROR: The key <" << key << "> is not found." << endl;
    }
    else {
	ret_value = value;
	cerr << "FOUND: The key <" << key << "> is found: " << ret_value << endl;
    }
    return ret_value;
}

}
