#pragma once

//external libraries
#include <cmore.h>
#include <memcry.h>

//local headers
#include "scancry.h"



extern "C" {

//sc_opt - external
sc_opt sc_new_opt(const enum sc_addr_width addr_width);
sc_opt sc_copy_opt(const sc_opt opts);
int sc_del_opt(sc_opt opts);
int sc_opt_reset(sc_opt opts);

int sc_opt_set_file_path_out(sc_opt opts, const char * path);
const char * sc_opt_get_file_path_out(const sc_opt opts);

int sc_opt_set_file_path_in(sc_opt opts, const char * path);
const char * sc_opt_get_file_path_in(const sc_opt opts);

int sc_opt_set_sessions(sc_opt opts, const cm_vct * sessions);
int sc_opt_get_sessions(const sc_opt opts, cm_vct * sessions);

int sc_opt_set_map(sc_opt opts, const mc_vm_map * map);
mc_vm_map * sc_opt_get_map(const sc_opt opts);

enum sc_addr_width sc_opt_get_addr_width(const sc_opt opts);

int sc_opt_set_omit_areas(sc_opt opts, const cm_vct * omit_areas);
int sc_opt_get_omit_areas(const sc_opt opts, cm_vct * omit_areas);

int sc_opt_set_omit_objs(sc_opt opts, const cm_vct * omit_objs);
int sc_opt_get_omit_objs(const sc_opt opts, cm_vct * omit_objs);

int sc_opt_set_exclusive_areas(sc_opt opts, const cm_vct * exclusive_areas);
int sc_opt_get_exclusive_areas(const sc_opt opts, cm_vct * exclusive_areas);

int sc_opt_set_exclusive_objs(sc_opt opts, const cm_vct * exclusive_objs);
int sc_opt_get_exclusive_objs(const sc_opt opts, cm_vct * exclusive_objs);

int sc_opt_set_omit_addr_ranges(sc_opt opts, const cm_vct * addr_ranges);
int sc_opt_get_omit_addr_ranges(const sc_opt opts, cm_vct * addr_ranges);

int sc_opt_set_exclusive_addr_ranges(
        sc_opt opts, const cm_vct * addr_ranges);
int sc_opt_get_exclusive_addr_ranges(
        const sc_opt opts, cm_vct * addr_ranges);

int sc_opt_set_access(sc_opt opts, const cm_byte access);
cm_byte sc_opt_get_access(const sc_opt opts);


//sc_opt_ptr - external
sc_opt_ptr sc_new_opt_ptr();
sc_opt_ptr sc_copy_opt_ptr(const sc_opt_ptr opts_ptr);
int sc_del_opt_ptr(sc_opt_ptr opts_ptr);
int sc_opt_ptr_reset(sc_opt_ptr opts_ptr);

int sc_opt_ptr_set_target_addr(sc_opt_ptr opts_ptr,
                                   const uintptr_t target_addr);
uintptr_t sc_opt_ptr_get_target_addr(const sc_opt_ptr opts_ptr);

int sc_opt_ptr_set_alignment(sc_opt_ptr opts_ptr,
                                 const off_t alignment);
off_t sc_opt_ptr_get_alignment(const sc_opt_ptr opts_ptr);

int sc_opt_ptr_set_max_obj_sz(sc_opt_ptr opts_ptr,
                                  const off_t max_obj_sz);
off_t sc_opt_ptr_get_max_obj_sz(const sc_opt_ptr opts_ptr);

int sc_opt_ptr_set_max_depth(sc_opt_ptr opts_ptr,
                                 const int max_depth);
int sc_opt_ptr_get_max_depth(const sc_opt_ptr opts_ptr);

int sc_opt_ptr_set_static_areas(sc_opt_ptr opts_ptr,
                                    const cm_vct * static_areas);
int sc_opt_ptr_get_static_areas(const sc_opt_ptr opts_ptr,
                                    cm_vct * static_areas);

int sc_opt_ptr_set_preset_offsets(sc_opt_ptr opts_ptr,
                                      const cm_vct * preset_offsets);
int sc_opt_ptr_get_preset_offsets(const sc_opt_ptr opts_ptr,
                                      cm_vct * preset_offsets);

int sc_opt_ptr_set_smart_scan(sc_opt_ptr opts_ptr,
                                  const bool enable);
bool sc_opt_ptr_get_smart_scan(const sc_opt_ptr opts_ptr);

} //extern "C"
