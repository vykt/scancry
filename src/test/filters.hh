#pragma once

//external libraries
#include <doctest/doctest.h>


/*
 *  --- [FILTERS] ---
 */

//C++ interface option classes tests
inline const constexpr int test_cc_opt_subtests_num = 21;
inline const constexpr char * test_cc_opt_subtests[] = {
    "test_cc_opt_ctor_dtor",
    "test_cc_opt_file_path_out",
    "test_cc_opt_file_path_in",
    "test_cc_opt_sessions",
    "test_cc_opt_map",
    "test_cc_opt_addr_width",
    "test_cc_opt_scan_set",
    "test_cc_opt_copy_ctor",
    "test_cc_opt_copy_assign",
    "test_cc_opt_reset",

    "test_cc_opt_ptrscan_ctor_dtor",
    "test_cc_opt_ptrscan_target_addr",
    "test_cc_opt_ptrscan_alignment",
    "test_cc_opt_ptrscan_max_obj_sz",
    "test_cc_opt_ptrscan_max_depth",
    "test_cc_opt_ptrscan_static_set",
    "test_cc_opt_ptrscan_preset_offsets",
    "test_cc_opt_ptrscan_smart_scan",
    "test_cc_opt_ptrscan_copy_ctor",
    "test_cc_opt_ptrscan_copy_assign",
    "test_cc_opt_ptrscan_reset"
};


//C interface option classes tests
inline const constexpr int test_c_opt_subtests_num = 21;
inline const constexpr char * test_c_opt_subtests[] = {
    "test_c_sc_opt_ctor_dtor",
    "test_c_sc_opt_file_path_out",
    "test_c_sc_opt_file_path_in",
    "test_c_sc_opt_sessions",
    "test_c_sc_opt_map",
    "test_c_sc_opt_addr_width",
    "test_c_sc_opt_scan_set",
    "test_c_sc_opt_copy_ctor",
    "test_c_sc_opt_copy_assign",
    "test_c_sc_opt_reset",

    "test_c_sc_opt_ptrscan_ctor_dtor",
    "test_c_sc_opt_ptrscan_target_addr",
    "test_c_sc_opt_ptrscan_alignment",
    "test_c_sc_opt_ptrscan_max_obj_sz",
    "test_c_sc_opt_ptrscan_max_depth",
    "test_c_sc_opt_ptrscan_static_set",
    "test_c_sc_opt_ptrscan_preset_offsets",
    "test_c_sc_opt_ptrscan_smart_scan",
    "test_c_sc_opt_ptrscan_copy_ctor",
    "test_c_sc_opt_ptrscan_copy_assign",
    "test_c_sc_opt_ptrscan_reset"
};


//C++ interface map area tests
inline const constexpr int test_cc_map_area_subtests_num = 18;
inline const constexpr char * test_cc_map_area_subtests[] = {
    "test_cc_sc_opt_map_area_ctor_dtor",
    "test_cc_sc_opt_map_area_omit_areas",
    "test_cc_sc_opt_map_area_omit_objs",
    "test_cc_sc_opt_map_area_exclusive_areas",
    "test_cc_sc_opt_map_area_exclusive_objs",
    "test_cc_sc_opt_map_area_omit_addr_ranges",
    "test_cc_sc_opt_map_area_exclusive_addr_ranges",
    "test_cc_sc_opt_map_area_access",
    "test_cc_sc_opt_map_area_copy_ctor",
    "test_cc_sc_opt_map_area_copy_assign",
    "test_cc_sc_opt_map_area_reset",

    "test_cc_sc_map_area_set_ctor_dtor",
    "test_cc_sc_map_area_set_update_set",
    "test_cc_sc_map_area_set_to_addr_ord_vct",
    "test_cc_sc_map_area_set_to_size_ord_vct",
    "test_cc_sc_map_area_set_copy_ctor",
    "test_cc_sc_map_area_set_copy_assign",
    "test_cc_sc_map_area_set_reset"
};


//C interface map area tests
inline const constexpr int test_c_map_area_subtests_num = 18;
inline const constexpr char * test_c_map_area_subtests[] = {
    "test_c_sc_opt_map_area_ctor_dtor",
    "test_c_sc_opt_map_area_omit_areas",
    "test_c_sc_opt_map_area_omit_objs",
    "test_c_sc_opt_map_area_exclusive_areas",
    "test_c_sc_opt_map_area_exclusive_objs",
    "test_c_sc_opt_map_area_omit_addr_ranges",
    "test_c_sc_opt_map_area_exclusive_addr_ranges",
    "test_c_sc_opt_map_area_access",
    "test_c_sc_opt_map_area_copy_ctor",
    "test_c_sc_opt_map_area_copy_assign",
    "test_c_sc_opt_map_area_reset",

    "test_c_sc_map_area_set_ctor_dtor",
    "test_c_sc_map_area_set_update_set",
    "test_c_sc_map_area_set_to_addr_ord_vct",
    "test_c_sc_map_area_set_to_size_ord_vct",
    "test_c_sc_map_area_set_copy_ctor",
    "test_c_sc_map_area_set_copy_assign",
    "test_c_sc_map_area_set_reset"
};


//C++ interface worker_pool tests
inline const constexpr int test_cc_worker_pool_subtests_num = 8;
inline const constexpr char * test_cc_worker_pool_subtests[] = {
    "test_cc_worker_pool",
    "test_cc_worker_pool_setup",
    "test_cc_worker_pool_workers",

    "test_cc_worker_pool_setup_flags",
    "test_cc_worker_pool_scan",
    "test_cc_worker_pool_scan_threaded",
    "test_cc_worker_pool_scan_terminated",
    "test_cc_worker_pool_reset"
};


//C interface worker_pool tests
inline const constexpr int test_c_worker_pool_subtests_num = 8;
inline const constexpr char * test_c_worker_pool_subtests[] = {
    "test_c_worker_pool",
    "test_c_worker_pool_setup_free_workers",
    "test_c_worker_pool_setup_free_workers_threaded",
    "test_c_worker_pool_setup_flags",
    "test_c_worker_pool_scan",
    "test_c_worker_pool_scan_threaded",
    "test_c_worker_pool_scan_terminated",
    "test_c_worker_pool_reset"
};


//C++ interface serialiser tests
inline const constexpr int test_cc_serialiser_subtests_num = 3;
inline const constexpr char * test_cc_serialiser_subtests[] = {
    "test_cc_serialiser",
    "test_cc_serialiser_save_load_scan",
    "test_cc_serialiser_read_headers"
};


//C interface serialiser tests
inline const constexpr int test_c_serialiser_subtests_num = 3;
inline const constexpr char * test_c_serialiser_subtests[] = {
    "test_c_serialiser",
    "test_c_serialiser_save_load_scan",
    "test_c_serialiser_read_headers"
};


//C++ interface ptrscan tests
inline const constexpr int test_cc_ptrscan_subtests_num = 5;
inline const constexpr char * test_cc_ptrscan_subtests[] = {
    "test_cc_ptrscan",
    "test_cc_ptrscan_scan",
    "test_cc_ptrscan_scan_threaded",
    "test_cc_ptrscan_save_load",
    "test_cc_ptrscan_verify",
};


//C interface ptrscan tests
inline const constexpr int test_c_ptrscan_subtests_num = 4;
inline const constexpr char * test_c_ptrscan_subtests[] = {
    "test_c_ptrscan_scan",
    "test_c_ptrscan_save_load",
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

void add_cc_ptrscan(doctest::Context & context);
void add_c_ptrscan(doctest::Context & context);
