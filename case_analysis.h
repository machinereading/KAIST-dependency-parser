#ifndef CASE_ANALYSIS_H
#define CASE_ANALYSIS_H

#include "common.h"
#include "caseframe.h"
#include "cky.h"

namespace Parser {

class CaseAnalysis {
    Caseframe cf;
  public:
    CaseAnalysis(CKY_cell *cky_ptr) {
	set_data_cf(cky_ptr);
    }
    bool set_data_cf(CKY_cell *cky_ptr);

    bool _make_data_from_feature_to_pp(Caseframe *cf_ptr, Phrase *b_ptr, bool adjacent_flag, bool clausal_modifier_flag);
    Phrase *_make_data_cframe_pp(Caseframe *cf_ptr, Phrase *b_ptr, bool adjacent_flag, bool clausal_modifier_flag);
    bool make_data_cframe_child(Caseframe *cf_ptr, Phrase *child_ptr, bool adjacent_flag);
    bool collect_arguments(CKY_cell *cky_ptr);
};

}

#endif
