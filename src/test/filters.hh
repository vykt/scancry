#pragma once

//external libraries
#include <doctest/doctest.h>


/*
 *  --- [FILTERS] ---
 */

//C++ interface opt class tests
inline const constexpr int test_cc_opt_subtests_num = 12;
inline const constexpr char * test_cc_opt_subtests[] = {
    "test_cc_opt",
    "test_cc_opt_file_path_out",
    "test_cc_opt_file_path_in",
    "test_cc_opt_sessions",
    "test_cc_opt_map",
    "test_cc_opt_alignment",
    "test_cc_opt_arch_byte_width",
    "test_cc_opt_omit_areas",
    "test_cc_opt_omit_objs",
    "test_cc_opt_exclusive_areas",
    "test_cc_opt_exclusive_objs",
    "test_cc_opt_addr_range",
};


//C interface opt class tests
inline const constexpr int test_c_opt_subtests_num = 12;
inline const constexpr char * test_c_opt_subtests[] = {
    "test_c_sc_opt",
    "test_c_sc_opt_file_path_out",
    "test_c_sc_opt_file_path_in",
    "test_c_sc_opt_sessions",
    "test_c_sc_opt_map",
    "test_c_sc_opt_alignment",
    "test_c_sc_opt_arch_byte_width",
    "test_c_sc_opt_omit_areas",
    "test_c_sc_opt_omit_objs",
    "test_c_sc_opt_exclusive_areas",
    "test_c_sc_opt_exclusive_objs",
    "test_c_sc_opt_addr_range",
};


//C++ interface scan_set class tests
inline const constexpr int test_cc_scan_set_subtests_num = 3;
inline const constexpr char * test_cc_scan_set_subtests[] = {
    "test_cc_scan_set",
    "test_cc_scan_set_update_scan_areas",
    "test_cc_scan_set_get_area_nodes"
};


//C interface opt class tests
inline const constexpr int test_c_scan_set_subtests_num = 3;
inline const constexpr char * test_c_scan_set_subtests[] = {
    "test_c_scan_set",
    "test_c_scan_set_update_scan_areas",
    "test_c_scan_set_get_area_nodes"
};



/*
 *  --- [FILTER FUNCTIONS] ---
 */

void add_cc_opt(doctest::Context & context);
void add_c_opt(doctest::Context & context);

void add_cc_scan_set(doctest::Context & context);
void add_c_scan_set(doctest::Context & context);
