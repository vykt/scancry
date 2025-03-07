//standard template library
#include <iterator>
#include <optional>
#include <unordered_set>
#include <string>
#include <algorithm>

//C standard library
#include <cstring>

//system headers
#include <unistd.h>
#include <linux/limits.h>

//external libraries
#include <cmore.h>
#include <memcry.h>

//local headers
#include "scancry.h"
#include "scan_set.hh"
#include "opt.hh"
#include "error.hh"



/*
 *  --- [INTERNAL] ---
 */

_SC_DBG_STATIC _SC_DBG_INLINE
bool is_blacklisted(const char * pathname) {

    //for every blacklisted pathname
    for (int i = 0; i < map_meta::pathname_blacklist_len; ++i) {

        //if pathname is blacklisted return `true`
        if(!strncmp(pathname, map_meta::pathname_blacklist[i], PATH_MAX))
            return true;
    }

    return false;
}


/* This function uses heuristics to determine whether an object should
 * be declared as "static" in a pointer scan. We're looking for .bss
 * segments (or their equivalents) in mmap'ed executables. Typically
 * for such objects, their pathname begins with a '/' and the object
 * contains multiple areas (r--, rw-, r-x, etc.). */
_SC_DBG_STATIC _SC_DBG_INLINE
bool is_static(const cm_lst_node * const obj_node) {

    //fetch object from node
    mc_vm_obj * obj = MC_GET_NODE_OBJ(obj_node);

    //object must have a path starting with '/'
    if (obj->pathname[0] != '/') return false;

    //object must have more than one area
    return (obj->vm_area_node_ps.len > 1) ? true : false;
}


_SC_DBG_STATIC _SC_DBG_INLINE
bool is_included(cm_lst_node * node,
                 const std::vector<cm_lst_node *> & node_set) {

    //search for this node in the node set
    auto iter = std::find(node_set.begin(), node_set.end(), node);

    //return `true` if found
    if (iter == node_set.end()) return false;
    return true;
}


_SC_DBG_STATIC _SC_DBG_INLINE
bool is_access(const cm_byte area_access, const cm_byte access_bitfield) {

    cm_byte area_access_bit, access_bitfield_bit, bitmask;


    //for every access bit
    bitmask = 0b1;
    do {

        //get relevant bits for this iteration
        area_access_bit &= bitmask;
        access_bitfield_bit &= bitmask;

        //if this access bit is not required
        if (area_access_bit == 0) goto is_access_continue;

        //if this area lacks this required access bit
        if ((area_access_bit & access_bitfield_bit) == 0) return false;        

        //shift bitmask a bit to the right
        is_access_continue:
        bitmask = bitmask << 1;
        
    } while (bitmask <= 0b1000);

    return true;
}


_SC_DBG_STATIC _SC_DBG_INLINE
bool in_addr_range(const mc_vm_area * area,
                   const std::pair<uintptr_t, uintptr_t> addr_range) {

    //if area falls outside the address range
    if (area->start_addr < addr_range.first
        || area->end_addr >= addr_range.second) return false;

    return true;
}


_SC_DBG_STATIC _SC_DBG_INLINE
cm_lst_node * get_last_obj_area(mc_vm_obj * obj) {

    cm_lst_node * area_node_ptr;

    //fetch last area node of this object
    area_node_ptr = obj->vm_area_node_ps.head;
    if (area_node_ptr->prev != NULL)
        area_node_ptr = area_node_ptr->prev;

    return MC_GET_NODE_PTR(area_node_ptr);
}



/*
 *  --- [PUBLIC]
 */

/* This method will build the selected set of areas for a scan. In
 * addition to the constraint vectors defined in `opt`, an `access_bitfield`
 * defines required permissions for every area. Set it to 0 to accept
 * any permissions. */
std::optional<int> scan_set::update_scan_areas(const cm_byte access_bitfield,
                                               const options & opt) {

    cm_lst_node * area_node;
    mc_vm_area * area;
    
    cm_lst_node * obj_node;
    mc_vm_obj * obj;

    /* If an area is included inside an exclusive objects set, do not check
     * if it is included in the exclusive areas set. */
    bool exclusive_included;


    //empty previous scan set
    this->area_nodes.clear();


    //fetch scan constraints
    const std::optional<std::vector<cm_lst_node *>> & omit_areas_set
        = opt.get_omit_areas_set();
    const std::optional<std::vector<cm_lst_node *>> & omit_objs_set
        = opt.get_omit_objs_set(); 
    const std::optional<std::vector<cm_lst_node *>> & exclusive_areas_set
        = opt.get_exclusive_areas_set();
    const std::optional<std::vector<cm_lst_node *>> & exclusive_objs_set
        = opt.get_exclusive_objs_set();
    const std::optional<std::pair<uintptr_t, uintptr_t>> addr_range
        = opt.get_addr_range();


    //fetch MemCry map
    mc_vm_map const * map = opt.get_map();
    if (map == nullptr) {
        sc_errno = SC_ERR_OPT_NOMAP;
        return std::nullopt;
    }


    //setup iteration over objects
    area_node = map->vm_areas.head;
    area = MC_GET_NODE_AREA(area_node);
    exclusive_included = false;

    //iterate through every area
    while (area_node->next != map->vm_areas.head && area_node->next != NULL) {

        // [object related checks]

        //perform object related checks if an object is present
        obj_node = area->obj_node_p;
        if (obj_node != nullptr) {

            //if an exclusive object set is used, object must be in it
            if (exclusive_objs_set.has_value()) {
                if(!is_included(obj_node, exclusive_objs_set.value())) {

                    //skip to last area of this object before continuing
                    area_node = get_last_obj_area(obj);

                    goto update_scan_areas_continue;
                }
                exclusive_included = true;
            }

            //if an omit object set is used, object must not be in it
            if (omit_objs_set.has_value()) {
                if (is_included(obj_node, omit_objs_set.value())) {

                    //skip to last area of this object before continuing
                    area_node = get_last_obj_area(obj);

                    goto update_scan_areas_continue;
                }
            }
            

            //object's pathname must not be blacklisted
            if (is_blacklisted(obj->pathname)) {

                //skip to last area of this object before continuing
                area_node = get_last_obj_area(obj);

                goto update_scan_areas_continue;
            }

        } //end object related checks


        // [area related checks]

        //if an exclusive area set is used, area must be in it
        if (exclusive_areas_set.has_value() && exclusive_included == false) {
            if (!is_included(area_node, exclusive_areas_set.value()))
                goto update_scan_areas_continue;
        }

        //if an omit area set is used, area must not be in it
        if (omit_areas_set.has_value()) {
            if (is_included(area_node, omit_areas_set.value()))
                goto update_scan_areas_continue;
        }

        //if access constraints are used, area must satisfy them 
        if (access_bitfield != 0) {
            if (!is_access(area->access, access_bitfield))
                goto update_scan_areas_continue;
        }

        //if an address range is provided, area must be inside it
        if (addr_range.has_value()) {
            if (!in_addr_range(area, addr_range.value()))
                goto update_scan_areas_continue;
        }


        //checks passed, add this area to the scan set
        this->area_nodes.insert(area_node);
        

        //advance iteration
        update_scan_areas_continue:
        area_node = area_node->next;
        area = MC_GET_NODE_AREA(area_node);
        exclusive_included = false;
        
    } //end iterate through every area

    if (this->area_nodes.empty() == true) {

        return std::nullopt;
    }

    return 0;
}
