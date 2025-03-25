#pragma once

//external libraries
#include <doctest/doctest.h>


/*
 *  --- [FILTERS] ---
 */

//C++ interface opt class tests
inline const constexpr int test_cc_opt_subtests_num = 11;
inline const constexpr char * test_cc_opt_subtests[] = {
    "test_cc_opt",
    "test_cc_opt_file_path_out",
    "test_cc_opt_file_path_in",
    "test_cc_opt_sessions",
    "test_cc_opt_map",
    "test_cc_opt_omit_areas",
    "test_cc_opt_omit_objs",
    "test_cc_opt_exclusive_areas",
    "test_cc_opt_exclusive_objs",
    "test_cc_opt_addr_range",
    "test_cc_opt_access"
};


//C interface opt class tests
inline const constexpr int test_c_opt_subtests_num = 12;
inline const constexpr char * test_c_opt_subtests[] = {
    "test_c_sc_opt",
    "test_c_sc_opt_file_path_out",
    "test_c_sc_opt_file_path_in",
    "test_c_sc_opt_sessions",
    "test_c_sc_opt_map",
    "test_c_sc_opt_addr_width",
    "test_c_sc_opt_omit_areas",
    "test_c_sc_opt_omit_objs",
    "test_c_sc_opt_exclusive_areas",
    "test_c_sc_opt_exclusive_objs",
    "test_c_sc_opt_addr_range",
    "test_c_sc_opt_access"
};


//C++ interface map_area_set class tests
inline const constexpr int test_cc_map_area_set_subtests_num = 5;
inline const constexpr char * test_cc_map_area_set_subtests[] = {
    "test_cc_map_area_set",
    "test_cc_map_area_set_simple",
    "test_cc_map_area_set_access",
    "test_cc_map_area_set_addr_range",
    "test_cc_map_area_set_constraints"
};


//C interface opt class tests
inline const constexpr int test_c_map_area_set_subtests_num = 2;
inline const constexpr char * test_c_map_area_set_subtests[] = {
    "test_c_sc_map_area_set",
    "test_c_sc_map_area_set_wrapper"
};



/*
 *  --- [FILTER FUNCTIONS] ---
 */

void add_cc_opt(doctest::Context & context);
void add_c_opt(doctest::Context & context);

void add_cc_map_area_set(doctest::Context & context);
void add_c_map_area_set(doctest::Context & context);
