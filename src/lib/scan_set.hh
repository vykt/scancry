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


/* Abstraction above mc_vm_map; uses constraints from the options
 * class to arrive at a final set of areas to scan. */
class scan_set {

    private:
        //[attributes]
        std::unordered_set<cm_lst_node *> area_nodes;

    public:
        //[methods]
        std::optional<int> update_scan_areas(const cm_byte access_mask,
                                             const options & opt);

        //getters & setters
        const std::unordered_set<cm_lst_node *> &
                                get_area_nodes() const {
            return area_nodes;
        }
};
