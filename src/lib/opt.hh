#pragma once

//external libraries
#include <cmore.h>
#include <memcry.h>

//local headers
#include "scancry.h"



extern "C" {

// -- sc_opt

//opaque handle = success, NULL = error
sc_opt * sc_new_opt();
sc_opt * sc_copy_opt(const sc_opt * opts);
//void return
void sc_del_opt(sc_opt * opts);
//0 = success, -1 = error
int sc_opt_reset(sc_opt * opts);

//0 = success, -1 = error
int sc_opt_set_file_pathname_out(sc_opt * opts, const char * path);
//pointer to a private string (can't fail)
const char ** sc_opt_get_filename_pathname_out(const sc_opt * opts);

//0 = success, -1 = error
int sc_opt_set_file_pathname_in(sc_opt * opts, const char * path);
//pointer to a private string (can't fail)
const char ** sc_opt_get_file_pathname_in(const sc_opt * opts);

/*
 *  NOTE: The following setter requires an initialised vector. The
 *        getter requires an unitialised vector which will be
 *        initialised and populated by the call. On success, the
 *        returned vector be manually destroyed. 
 */

//0 = success, -1 = error
int sc_opt_set_sessions(sc_opt * opts,
                               const cm_vct /* <mc_session> */ * sessions);
int sc_opt_get_sessions(const sc_opt * opts,
                               cm_vct /* <mc_session> */ * sessions);

//0 = success, -1 = error
int sc_opt_set_map(sc_opt * opts, const mc_vm_map * map);
//pointer = success, NULL = error
mc_vm_map * sc_opt_get_map(const sc_opt * opts);

//0 = success, -1 = error
int sc_opt_set_alignment(sc_opt * opts, const off_t alignment);
//alignment = success, SC_ALIGNMENT_BAD = error
off_t sc_opt_get_alignment(const sc_opt opts);

//0 = success. -1 = error
int sc_opt_set_addr_width(sc_opt * opts,
                                 const sc_addr_width addr_width);
//address width = success, SC_ADDR_WIDTH_BAD = error
enum sc_addr_width sc_opt_get_addr_width(const sc_opt * opts);

//0 = success, -1 = error
int sc_opt_set_scan_set(sc_opt * opts,
                               const sc_map_area_opt * ma_opts);
//map area set attribute pointer = success, NULL = error
const sc_map_area_set * sc_opt_get_scan_set(const sc_opt * opts);


// -- sc_opt_ptr

//opaque handle = success, NULL = error
sc_opt_ptr * sc_new_opt_ptr();
sc_opt_ptr * sc_copy_opt_ptr(const sc_opt_ptr * opts_ptr);
//0 = success, -1 = error
void sc_del_opt_ptr(sc_opt_ptr * opts_ptr);
int sc_opt_ptr_reset(sc_opt_ptr * opts_ptr);

//0 = success, -1 = error
int sc_opt_ptr_set_target_addr(sc_opt_ptr * opts_ptr,
                                      const uintptr_t target_addr);
//target address = success, SC_TARGET_ADDR_BAD = error
uintptr_t sc_opt_ptr_get_target_addr(const sc_opt_ptr * opts_ptr);

//0 = success, -1 = error
int sc_opt_ptr_set_alignment(sc_opt_ptr * opts_ptr,
                                    const off_t alignment);
//alignment = success, SC_ALIGNMENT_BAD = error
off_t sc_opt_ptr_get_alignment(const sc_opt_ptr * opts_ptr);

//0 = success. -1 = error
int sc_opt_ptr_set_max_obj_sz(sc_opt_ptr * opts_ptr,
                                     const off_t max_obj_sz);
//max object size = success, SC_MAX_OBJ_SZ_BAD = error
off_t sc_opt_ptr_get_max_obj_sz(const sc_opt_ptr * opts_ptr);

//0 = success, -1 = error
int sc_opt_ptr_set_max_depth(sc_opt_ptr * opts_ptr,
                                    const int max_depth);
//max depth = succeess, SC_MAX_DEPTH_BAD = error
int sc_opt_ptr_get_max_depth(const sc_opt_ptr * opts_ptr);

//0 = success, -1 = error
int sc_opt_ptr_set_static_set(sc_opt_ptr * opts_ptr,
                                     const sc_map_area_set * static_set);
//pointer to a private map area set (can't fail)
const sc_map_area_set *
    sc_opt_ptr_get_static_set(const sc_opt_ptr * opts_ptr);

//0 = success, -1 = error
int sc_opt_ptr_set_preset_offsets(
    sc_opt_ptr * opts_ptr, const cm_vct * preset_offsets);
//pointer to a private vector (can't fail)
const cm_vct *
    sc_opt_ptr_get_preset_offsets(const sc_opt_ptr * opts_ptr);

//0 = success, -1 = fail
int sc_opt_ptr_set_smart_scan(sc_opt_ptr * opts_ptr,
                                     const enum sc_smart_scan smart_scan);
//smart scan enum = success, SC_SMART_SCAN_BAD = errorr
enum sc_smart_scan
    sc_opt_ptr_get_smart_scan(const sc_opt_ptr * opts_ptr);

} //"C"
