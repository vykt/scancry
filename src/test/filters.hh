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
    "test_cc_opt_omit_areas",
    "test_cc_opt_omit_objs",
    "test_cc_opt_exclusive_areas",
    "test_cc_opt_exclusive_objs",
    "test_cc_opt_addr_range",
    "test_cc_opt_access",
    "test_cc_opt_reset"
};


//C interface opt class tests
inline const constexpr int test_c_opt_subtests_num = 13;
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
    "test_c_sc_opt_access",
    "test_c_sc_opt_reset"
};


//C++ interface opt_ptrscan class tests
inline const constexpr int test_cc_opt_ptrscan_subtests_num = 9;
inline const constexpr char * test_cc_opt_ptrscan_subtests[] = {
    "test_cc_sc_opt_ptrscan",
    "test_cc_sc_opt_ptr_target_addr",
    "test_cc_sc_opt_ptr_alignment",
    "test_cc_sc_opt_ptr_max_obj_sz",
    "test_cc_sc_opt_ptr_max_depth",
    "test_cc_sc_opt_ptr_static_areas",
    "test_cc_sc_opt_ptr_preset_offsets",
    "test_cc_sc_opt_ptr_smart_scan",
    "test_cc_sc_opt_ptr_reset"
};


//C interface opt_ptrscan class tests
inline const constexpr int test_c_opt_ptrscan_subtests_num = 9;
inline const constexpr char * test_c_opt_ptrscan_subtests[] = {
    "test_c_sc_opt_ptrscan",
    "test_c_sc_opt_ptr_target_addr",
    "test_c_sc_opt_ptr_alignment",
    "test_c_sc_opt_ptr_max_obj_sz",
    "test_c_sc_opt_ptr_max_depth",
    "test_c_sc_opt_ptr_static_areas",
    "test_c_sc_opt_ptr_preset_offsets",
    "test_c_sc_opt_ptr_smart_scan",
    "test_c_sc_opt_ptr_reset"
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


//C++ interface worker_pool tests
inline const constexpr int test_cc_worker_pool_subtests_num = 4;
inline const constexpr char * test_cc_worker_pool_subtests[] = {
    "test_cc_worker_pool",
    "test_cc_setup_free_workers",
    "test_scan",
    "test_multithread_scan"
};


//C interface worker_pool tests



/*
 *  --- [FILTER FUNCTIONS] ---
 */

void add_cc_opt(doctest::Context & context);
void add_c_opt(doctest::Context & context);

void add_cc_opt_ptrscan(doctest::Context & context);
void add_c_opt_ptrscan(doctest::Context & context);

void add_cc_map_area_set(doctest::Context & context);
void add_c_map_area_set(doctest::Context & context);
