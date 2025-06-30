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


//add C++ interface `opt` class tests
void add_cc_opt(doctest::Context & context) {
    _add_filters(test_cc_opt_subtests, test_cc_opt_subtests_num, context);
    return;
}


//add C interface `opt` class tests
void add_c_opt(doctest::Context & context) {
    _add_filters(test_c_opt_subtests, test_c_opt_subtests_num, context);
    return;
}


//add C++ interface `opt_ptr` class tests
void add_cc_opt_ptr(doctest::Context & context) {
    _add_filters(test_cc_opt_ptr_subtests,
                 test_cc_opt_ptr_subtests_num, context);
    return;
}


//add C interface `opt_ptr` class tests
void add_c_opt_ptr(doctest::Context & context) {
    _add_filters(test_c_opt_ptr_subtests,
                 test_c_opt_ptr_subtests_num, context);
    return;
}


//add C++ interface `map_area_set` tests
void add_cc_map_area_set(doctest::Context & context) {
    _add_filters(test_cc_map_area_set_subtests,
                 test_cc_map_area_set_subtests_num, context);
    return;
}


//add C interface `map_area_set` tests
void add_c_map_area_set(doctest::Context & context) {
    _add_filters(test_c_map_area_set_subtests,
                 test_c_map_area_set_subtests_num, context);
    return;
}


//add C++ interface `worker_pool` tests
void add_cc_worker_pool(doctest::Context & context) {
    _add_filters(test_cc_worker_pool_subtests,
                 test_cc_worker_pool_subtests_num, context);
    return;
}


//add C interface `worker_pool` tests
void add_c_worker_pool(doctest::Context & context) {
    _add_filters(test_c_worker_pool_subtests,
                 test_c_worker_pool_subtests_num, context);
    return;
}


//add C++ interface `ptrscan` tests
void add_cc_ptrscan(doctest::Context & context) {
    _add_filters(test_cc_ptrscan_subtests,
                 test_cc_ptrscan_subtests_num, context);
    return;
}


//add C interface `ptrscan` tests
void add_c_ptrscan(doctest::Context & context) {
    _add_filters(test_c_ptrscan_subtests,
                 test_c_ptrscan_subtests_num, context);
    return;
}
