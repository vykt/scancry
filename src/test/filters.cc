//external libraries
#include <doctest/doctest.h>

//local headers
#include "filters.hh"



//generic doctest filter applier
void _add_filters(const char * const filters[],
                  int len, doctest::Context & context) {

    //add filters
    for (int i = 0; i < len; ++i) {
        context.addFilter("test-case", filters[i]);
    }

    return;
}


//add C++ interface opt class tests
void add_cc_opt(doctest::Context & context) {

    _add_filters(test_cc_opt_subtests, test_cc_opt_subtests_num, context);
    return;
}


//add C interface opt class tests
void add_c_opt(doctest::Context & context) {

    _add_filters(test_c_opt_subtests, test_c_opt_subtests_num, context);
    return;
}


//add C++ interface scan_set tests
void add_cc_scan_set(doctest::Context & context) {

    _add_filters(test_cc_scan_set_subtests,
                 test_cc_scan_set_subtests_num, context);
    return;
}


//add C interface scan_set tests
void add_c_scan_set(doctest::Context & context) {

    _add_filters(test_c_scan_set_subtests,
                 test_c_scan_set_subtests_num, context);
    return;
}
