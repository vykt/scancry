#pragma once

//external libraries
#include <doctest/doctest.h>


/*
 *  --- [FILTERS] ---
 */

//C++ interface opt class tests
inline const constexpr int test_cc_opt_subtests_num = 13;
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
    "test_cc_opt_omit_addr_range",
    "test_cc_opt_exclusive_addr_range",
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
    "test_c_sc_opt_omit_addr_range",
    "test_c_sc_opt_exclusive_addr_range",
    "test_c_sc_opt_access",
    "test_c_sc_opt_reset"
};


//C++ interface opt_ptr class tests
inline const constexpr int test_cc_opt_ptr_subtests_num = 9;
inline const constexpr char * test_cc_opt_ptr_subtests[] = {
    "test_cc_opt_ptr",
    "test_cc_opt_ptr_target_addr",
    "test_cc_opt_ptr_alignment",
    "test_cc_opt_ptr_max_obj_sz",
    "test_cc_opt_ptr_max_depth",
    "test_cc_opt_ptr_static_areas",
    "test_cc_opt_ptr_preset_offsets",
    "test_cc_opt_ptr_smart_scan",
    "test_cc_opt_ptr_reset"
};


//C interface opt_ptr class tests
inline const constexpr int test_c_opt_ptr_subtests_num = 9;
inline const constexpr char * test_c_opt_ptr_subtests[] = {
    "test_c_sc_opt_ptr",
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
inline const constexpr int test_cc_map_area_set_subtests_num = 6;
inline const constexpr char * test_cc_map_area_set_subtests[] = {
    "test_cc_sc_map_area_set",
    "test_cc_sc_map_area_set_simple",
    "test_cc_sc_map_area_set_access",
    "test_cc_sc_map_area_set_addr_ranges",
    "test_cc_sc_map_area_set_constraints",
    "test_cc_sc_map_area_set_reset"
};


//C interface opt class tests
inline const constexpr int test_c_map_area_set_subtests_num = 3;
inline const constexpr char * test_c_map_area_set_subtests[] = {
    "test_c_sc_map_area_set",
    "test_c_sc_map_area_set_wrapper",
    "test_c_sc_map_area_set_reset"
};


//C++ interface worker_pool tests
inline const constexpr int test_cc_worker_pool_subtests_num = 7;
inline const constexpr char * test_cc_worker_pool_subtests[] = {
    "test_cc_worker_pool",
    "test_cc_worker_pool_setup_free_workers",
    "test_cc_worker_pool_setup_free_workers_threaded",
    "test_cc_worker_pool_flags",
    "test_cc_worker_pool_scan",
    "test_cc_worker_pool_scan_threaded",
    "test_cc_worker_pool_crash_recover"
};


//C interface worker_pool tests
inline const constexpr int test_c_worker_pool_subtests_num = 2;
inline const constexpr char * test_c_worker_pool_subtests[] = {
    "test_c_worker_pool",
    "test_c_worker_pool_free_workers"
};


//C++ interface serialiser tests
inline const constexpr int test_cc_serialiser_subtests_num = 4;
inline const constexpr char * test_cc_serialiser_subtests[] = {
    "test_cc_serialiser",
    "test_cc_serialiser_save_load_scan",
    "test_cc_serialiser_read_headers"
};


//C interface serialiser tests
inline const constexpr int test_c_serialiser_subtests_num = 4;
inline const constexpr char * test_c_serialiser_subtests[] = {
    "test_c_serialiser",
    "test_c_serialiser_save_load_scan",
    "test_c_serialiser_read_headers"
};


//C++ interface ptrscan tests
inline const constexpr int test_cc_ptrscan_subtests_num = 5;
inline const constexpr char * test_cc_ptrscan_subtests[] = {
    "test_cc_ptrscan",
    "test_cc_ptrscan_process_addr",
    "test_cc_save_load_body",
    "test_cc_ptrscan_scan",
    "test_cc_ptrscan_verify",
};


//C interface ptrscan tests
inline const constexpr int test_c_ptrscan_subtests_num = 3;
inline const constexpr char * test_c_ptrscan_subtests[] = {
    "test_c_ptrscan_scan",
    "test_c_ptrscan_verify",
    "test_c_ptrscan_get_chains"
};



/*
 *  --- [FILTER FUNCTIONS] ---
 */

void add_cc_opt(doctest::Context & context);
void add_c_opt(doctest::Context & context);

void add_cc_opt_ptr(doctest::Context & context);
void add_c_opt_ptr(doctest::Context & context);

void add_cc_map_area_set(doctest::Context & context);
void add_c_map_area_set(doctest::Context & context);

void add_cc_worker_pool(doctest::Context & context);
void add_c_worker_pool(doctest::Context & context);

void add_cc_serialiser(doctest::Context & context);
void add_c_serialiser(doctest::Context & context);
