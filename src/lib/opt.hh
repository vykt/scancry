#pragma once

//external libraries
#include <cmore.h>
#include <memcry.h>

//local headers
#include "scancry.h"



extern "C" {

//external
void * new_sc_opt(const int arch_byte_width);
void del_sc_opt(sc_opt opts);

int sc_opt_file_path_out(sc_opt opts, const char * path);
const char * sc_opt_get_file_path_out(sc_opt opts);

int sc_opt_file_path_in(sc_opt opts, const char * path);
const char * sc_opt_get_file_path_in(sc_opt opts);

void sc_opt_session(sc_opt opts, mc_session * session);
mc_session const * sc_opt_get_session(sc_opt opts);

void sc_opt_map(sc_opt opts, mc_vm_map * map);
mc_vm_map const * sc_opt_get_map(sc_opt opts);

void sc_opt_alignment(sc_opt opts, int alignment);
int sc_opt_get_alignment(sc_opt opts);

int sc_opt_get_arch_byte_width(sc_opt opts);

int sc_opt_omit_areas(sc_opt opts, cm_vct * omit_areas);
int sc_opt_get_omit_areas(sc_opt opts, cm_vct * omit_areas);

int sc_opt_omit_objs(sc_opt opts, cm_vct * omit_objs);
int sc_opt_get_omit_objs(sc_opt opts, cm_vct * omit_objs);

int sc_opt_exclusive_areas(sc_opt opts, cm_vct * exclusive_areas);
int sc_opt_get_exclusive_areas(sc_opt opts, cm_vct * exclusive_areas);

int sc_opt_exclusive_objs(sc_opt opts, cm_vct * exclusive_objs);
int sc_opt_get_exclusive_objs(sc_opt opts, cm_vct * exclusive_objs);

void sc_opt_addr_range(sc_opt opts, sc_addr_range * range);
sc_addr_range sc_opt_get_addr_range(sc_opt opts);

} //extern "C"
