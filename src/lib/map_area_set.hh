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
    inline constexpr const int pathname_blacklist_len = 3;
    inline constexpr const char * pathname_blacklist[pathname_blacklist_len] = {
        "/dev",
        "/memfd",
        "/run"
        //TODO find others
    };
}



#ifdef __cplusplus
extern "C" {
#endif

//external
sc_map_area_set sc_new_map_area_set();
int sc_del_map_area_set(sc_map_area_set s_set);

int sc_update_set(sc_map_area_set s_set, const sc_opt opts);
int sc_map_area_set_get_area_nodes(const sc_map_area_set s_set, cm_vct * area_nodes);


#ifdef __cplusplus
} //extern "C"
#endif
