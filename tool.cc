#include "common.h"

namespace Parser {

chr_type check_chr_type(int code) {
    // HIRAGANA
    if (code > 0x303f && code < 0x30a0) {
	return CHR_HIRAGANA;
    }
    // KATAKANA and "ー"
    else if ((code > 0x309f && code < 0x3100) || code == 0x30fc) {
	return CHR_KATAKANA;
    }
    // FIGURE (only ０-９)
    else if (code > 0xff0f && code < 0xff1a) {
	return CHR_FIGURE;
    }
    // ALPHABET (A-Z, a-z)
    else if (code > 0xff20 && code < 0xff5b) {
	return CHR_ALPHABET;
    }
    // CJK Unified Ideographs and "々"
    else if ((code > 0x4dff && code < 0xa000) || code == 0x3005) {
	return CHR_KANJI;
    }
    else {
	return CHR_SYMBOL;
    }
}

size_t str_length(std::string &str) {
    size_t length = str.size();
    size_t i = 0, count = 0;

    while (i < length) {
	unsigned char c = *(str.substr(i, i + 1).c_str());
	if (c > 0xfb) { // 6 bytes
	    i += 6;
	}
	else if (c > 0xf7) { // 5 bytes
	    i += 5;
	}
	else if (c > 0xef) { // 4 bytes
	    i += 4;
	}
	else if (c > 0xdf) { // 3 bytes
	    i += 3;
	}
	else if (c > 0x7f) { // 2 bytes
	    i += 2;
	}
	else { // 1 byte
	    i++;
	}
	count++;
    }
    return count;
}

chr_type check_str_type(std::string &str) {
    size_t length = str.size();
    size_t i = 0, count = 0;
    std::vector<int> unicodes;
    int unicode;

    while (i < length) {
	unsigned char c = *(str.substr(i, i + 1).c_str());
	if (c > 0xfb) { // 6 bytes
	    i += 6;
	}
	else if (c > 0xf7) { // 5 bytes
	    i += 5;
	}
	else if (c > 0xef) { // 4 bytes
	    i += 4;
	}
	else if (c > 0xdf) { // 3 bytes
	    unicode = (c & 0x0f) << 12;
	    c = *(str.substr(i + 1, i + 2).c_str());
	    unicode += (c & 0x3f) << 6;
	    c = *(str.substr(i + 2, i + 3).c_str());
	    unicode += c & 0x3f;
	    unicodes.push_back(unicode);
	    i += 3;
	}
	else if (c > 0x7f) { // 2 bytes
	    unicode = (c & 0x1f) << 6;
	    c = *(str.substr(i + 1, i + 2).c_str());
	    unicode += c & 0x3f;
	    unicodes.push_back(unicode);
	    i += 2;
	}
	else { // 1 byte
	    unicodes.push_back(c);
	    i++;
	}
	count++;
    }

    chr_type this_type;
    chr_type pre_type = CHR_UNKNOWN;
    for (std::vector<int>::iterator it = unicodes.begin(); it != unicodes.end(); it++) {
	this_type = check_chr_type(*it);
	if (pre_type != CHR_UNKNOWN && pre_type != this_type) {
	    return CHR_UNKNOWN;
	}
	pre_type = this_type;
    }
    return this_type;
}

}
