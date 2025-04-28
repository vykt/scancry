#pragma once

//external libraries
#include <cmore.h>
#include <memcry.h>

//local headers
#include "scancry.h"



#ifdef __cplusplus
extern "C" {
#endif

//sc_opt - external
sc_opt new_sc_opt(const enum sc::addr_width addr_width);
int del_sc_opt(sc_opt opts);

int sc_opt_reset(sc_opt opts);

int sc_opt_set_file_path_out(sc_opt opts, const char * path);
const char * sc_opt_get_file_path_out(const sc_opt opts);

int sc_opt_set_file_path_in(sc_opt opts, const char * path);
const char * sc_opt_get_file_path_in(const sc_opt opts);

int sc_opt_set_sessions(sc_opt opts, const cm_vct * sessions);
int sc_opt_get_sessions(const sc_opt opts, cm_vct * sessions);

int sc_opt_set_map(sc_opt opts, const mc_vm_map * map);
const mc_vm_map * sc_opt_get_map(const sc_opt opts);

enum sc::addr_width sc_opt_get_arch_byte_width(const sc_opt opts);

int sc_opt_set_omit_areas(sc_opt opts, const cm_vct * omit_areas);
int sc_opt_get_omit_areas(const sc_opt opts, cm_vct * omit_areas);

int sc_opt_set_omit_objs(sc_opt opts, const cm_vct * omit_objs);
int sc_opt_get_omit_objs(const sc_opt opts, cm_vct * omit_objs);

int sc_opt_set_exclusive_areas(sc_opt opts, const cm_vct * exclusive_areas);
int sc_opt_get_exclusive_areas(const sc_opt opts, cm_vct * exclusive_areas);

int sc_opt_set_exclusive_objs(sc_opt opts, const cm_vct * exclusive_objs);
int sc_opt_get_exclusive_objs(const sc_opt opts, cm_vct * exclusive_objs);

int sc_opt_set_addr_range(sc_opt opts, const sc_addr_range * range);
int sc_opt_get_addr_range(const sc_opt opts, sc_addr_range * range);

int sc_opt_set_access(sc_opt opts, const cm_byte access);
cm_byte sc_opt_get_access(const sc_opt opts);


//sc_opt_ptrscan - external
sc_opt_ptrscan sc_new_opt_ptrscan();
int sc_del_opt_ptrscan(sc_opt_ptrscan opts_ptrscan);

int sc_opt_ptrscan_reset(sc_opt_ptrscan opts);

int sc_opt_ptrscan_set_target_addr(sc_opt_ptrscan opts_ptrscan,
                                   const uintptr_t target_addr);
uintptr_t sc_opt_ptrscan_get_target_addr(const sc_opt_ptrscan opts_ptrscan);

int sc_opt_ptrscan_set_alignment(sc_opt_ptrscan opts_ptrscan,
                                 const off_t alignment);
uintptr_t sc_opt_ptrscan_get_alignment(const sc_opt_ptrscan opts_ptrscan);

int sc_opt_ptrscan_set_max_obj_sz(sc_opt_ptrscan opts_ptrscan,
                                  const off_t max_obj_sz);
uintptr_t sc_opt_ptrscan_get_max_obj_sz(const sc_opt_ptrscan opts_ptrscan);

int sc_opt_ptrscan_set_static_areas(sc_opt_ptrscan opts_ptrscan,
                                    const cm_vct * static_areas);
int sc_opt_ptrscan_get_static_areas(const sc_opt_ptrscan opts_ptrscan,
                                    cm_vct * static_areas);

int sc_opt_ptrscan_set_preset_offsets(sc_opt_ptrscan opts_ptrscan,
                                      const cm_vct * preset_offsets);
int sc_opt_ptrscan_get_preset_offsets(const sc_opt_ptrscan opts_ptrscan,
                                      cm_vct * preset_offsets);

int sc_opt_ptrscan_set_smart_scan(sc_opt_ptrscan opts_ptrscan, bool enable);
bool sc_opt_ptrscan_get_smart_scan(const sc_opt_ptrscan opts_ptrscan);

#ifdef __cplusplus
} //extern "C"
#endif
