#ifndef DIC_H
#define DIC_H

#include "common.h"

class Dic {
    std::vector<std::map<std::string, int> *> dic;
  public:
    Dic();
    Dic(const char *);
    ~Dic();

    std::string find(int, std::string &);
};

#endif
