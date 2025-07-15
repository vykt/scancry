#pragma once

//standard template library
#include <optional>
#include <unordered_set>
#include <string>

//external libraries
#include <cmore.h>
#include <memcry.h>

//local includes
#include "scancry.h"
#include "opt.hh"



namespace map_meta {

    //trying to read/write from some backing objects can hang
    inline constexpr const int pathname_blacklist_len = 4;
    inline constexpr const char * pathname_blacklist[pathname_blacklist_len] = {
        "[vvar]",
        "/dev",
        "/memfd",
        "/run"
        //TODO find others
    };
}



#ifdef __cplusplus
extern "C" {
#endif

//external - map area options

//pointer = success, -1 = error
sc_opt_map_area * sc_new_opt_ma();
sc_opt_map_area * sc_copy_opt_ma(sc_opt_map_area * opts_ma);
//0 = success, -1 = error
void sc_del_opt_ma(sc_opt_map_area * opts_ma);
int sc_opt_ma_reset(sc_opt_map_area * opts_ma);

//setters: 0 = success, -1 = error
//getters: pointer = success, NULL = error

//omit areas
int sc_opt_ma_set_omit_areas(
    sc_opt_map_area * opts_ma, const cm_vct * omit_areas);
const cm_vct * sc_opt_ma_get_omit_areas(sc_opt_map_area * opts_ma);

//omit objects
int sc_opt_ma_set_omit_objs(
    sc_opt_map_area * opts_ma, const cm_vct * omit_objs);
const cm_vct * sc_opt_ma_get_omit_objs(sc_opt_map_area * opts_ma);

//exclusive areas
int sc_opt_ma_set_exclusive_areas(
    sc_opt_map_area * opts_ma, const cm_vct * exclusive_areas);
const cm_vct * sc_opt_ma_get_exclusive_areas(sc_opt_map_area * opts_ma);

//exclusive objects
int sc_opt_ma_set_exclusive_objs(
    sc_opt_map_area * opts_ma, const cm_vct * exclusive_objs);
const cm_vct * sc_opt_ma_get_exclusive_objs(sc_opt_map_area * opts_ma);

//omit address ranges
int sc_opt_ma_set_omit_addr_ranges(
    sc_opt_map_area * opts_ma, const cm_vct * omit_addr_ranges);
//only for this getter: 0 = success, -1 = fail, deallocate vector manually
int sc_opt_ma_get_omit_addr_ranges(
    sc_opt_map_area * opts_ma, cm_vct * addr_ranges);

//exclusive address ranges
int sc_opt_ma_set_exclusive_addr_ranges(
    sc_opt_map_area * opts_ma, const cm_vct * exclusive_addr_ranges);
//only for this getter: 0 = success, -1 = fail, deallocate vector manually
int sc_opt_ma_get_exclusive_addr_ranges(
    sc_opt_map_area * opts_ma, cm_vct * addr_range);

//access
//0 = success, CM_BYTE_MAX = error
int sc_opt_ma_set_access(
    sc_opt_map_area * opts_ma, const cm_byte access);
//CM_BYTE_MAX = error, SC_ACCESS_UNSET = not set, other = success
cm_byte sc_opt_ma_get_access(sc_opt_map_area * opts_ma);


//external - map area set

//pointer = success, NULL = error
sc_map_area_set * sc_new_ma_set();
sc_map_area_set * sc_copy_ma_set(sc_map_area_set * ma_set);
//0 = success, -1 = error
void sc_del_ma_set(sc_map_area_set * ma_set);
int sc_ma_set_reset(sc_map_area_set * ma_set);

//0 = success, -1 = error
int sc_ma_set_update_set(sc_map_area_set * ma_set,
                                sc_opt_map_area * opts_ma,
                                const mc_vm_map * map);
//pointer = success, -1 = error
const cm_rbt * sc_get_set(sc_map_area_set * ma_set);


#ifdef __cplusplus
} //"C"
#endif
