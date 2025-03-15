#pragma once

//external libraries
#include <cmore.h>
#include <memcry.h>

//local headers
#include "scancry.h"



#ifdef __cplusplus
extern "C" {
#endif

//external
sc_opt new_sc_opt(const int arch_byte_width);
int del_sc_opt(sc_opt opts);

int sc_opt_set_file_path_out(sc_opt opts, const char * path);
const char * sc_opt_get_file_path_out(const sc_opt opts);

int sc_opt_set_file_path_in(sc_opt opts, const char * path);
const char * sc_opt_get_file_path_in(const sc_opt opts);

int sc_opt_set_sessions(sc_opt opts, const cm_vct * sessions);
int sc_opt_get_sessions(const sc_opt opts, cm_vct * sessions);

void sc_opt_set_map(sc_opt opts, const mc_vm_map * map);
mc_vm_map const * sc_opt_get_map(const sc_opt opts);

int sc_opt_set_alignment(sc_opt opts, const unsigned int alignment);
unsigned int sc_opt_get_alignment(const sc_opt opts);

unsigned int sc_opt_get_arch_byte_width(const sc_opt opts);

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

#ifdef __cplusplus
} //extern "C"
#endif
