//standard template library
#include <optional>
#include <unordered_set>
#include <string>
#include <iterator>
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
#include "map_area_set.hh"
#include "opt.hh"
#include "c_iface.hh"
#include "error.hh"



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

/*
 *  --- [INTERNAL] ---
 */

[[nodiscard]] _SC_DBG_STATIC _SC_DBG_INLINE
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
[[nodiscard]] _SC_DBG_STATIC _SC_DBG_INLINE
bool is_static(const cm_lst_node * const obj_node) {

    //fetch object from node
    mc_vm_obj * obj = MC_GET_NODE_OBJ(obj_node);

    //object must have a path starting with '/'
    if (obj->pathname[0] != '/') return false;

    //object must have more than one area
    return (obj->vm_area_node_ps.len > 1) ? true : false;
}


[[nodiscard]] _SC_DBG_STATIC _SC_DBG_INLINE
bool is_included(cm_lst_node * node,
                 const std::vector<const cm_lst_node *> & node_set) {

    //search for this node in the node set
    auto iter = std::find(node_set.begin(), node_set.end(), node);

    //return `true` if found
    if (iter == node_set.end()) return false;
    return true;
}


[[nodiscard]] _SC_DBG_STATIC _SC_DBG_INLINE
bool is_access(const cm_byte area_access, const cm_byte access_bitfield) {

    cm_byte area_access_bit, access_bitfield_bit, bitmask;


    //for every access bit
    bitmask = 0b1;
    do {

        //get relevant bits for this iteration
        area_access_bit = area_access & bitmask;
        access_bitfield_bit = access_bitfield & bitmask;

        //if this access bit is not required
        if (access_bitfield_bit == 0) goto is_access_continue;

        //if this area lacks this required access bit
        if ((area_access_bit & access_bitfield_bit) == 0) return false;        

        //shift bitmask a bit to the right
        is_access_continue:
        bitmask = bitmask << 1;
        
    } while (bitmask <= 0b1000);

    return true;
}


[[nodiscard]] _SC_DBG_STATIC _SC_DBG_INLINE
std::optional<std::pair<uintptr_t, uintptr_t>> in_addr_ranges(
    const mc_vm_area * area, const std::vector<
        std::pair<uintptr_t, uintptr_t>> & addr_ranges) {

    //for every address range
    for (auto iter = addr_ranges.begin(); iter != addr_ranges.end(); ++iter) {

        //if area fits in this range
        if ((area->start_addr >= iter->first)
            && (area->end_addr <= iter->second)) { return *iter; }
    }

    return std::nullopt;
}


[[nodiscard]] _SC_DBG_STATIC _SC_DBG_INLINE
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

/*
 * NOTE: This method will build the selected set of areas for a scan. In
 *       addition to the constraint vectors defined in `opt`, an
 *       `access_bitfield` defines required permissions for every area.
 *       Set it to 0 to accept any permissions.
 */

sc::map_area_set::map_area_set() : _lockable() {}


sc::map_area_set::map_area_set(const map_area_set & ma_set)
 : _lockable(),
   area_nodes(ma_set.area_nodes) {}


sc::map_area_set::map_area_set(const map_area_set && ma_set)
 : _lockable(),
   area_nodes(ma_set.area_nodes) {}


[[nodiscard]] int sc::map_area_set::reset() {

    _LOCK(-1)
    this->area_nodes.clear();
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] int sc::map_area_set::update_set(sc::opt & opts) {

    cm_lst_node * area_node;
    mc_vm_area * area;
    
    cm_lst_node * obj_node;
    mc_vm_obj * obj;

    /*
     *  NOTE: If an area is not included in an exclusive set, continue
     *        evaluation in case it is included in another exclusive set.
     *
     *        If an area is included in any omit set, do not include it.
     *
     *        Omitting sets take precedence over exclusive sets.
     */
    
    bool first_iter;
    bool exclusive_used;

    std::optional<std::pair<uintptr_t, uintptr_t>> addr_range;

    //apply lock
    _LOCK(-1)

    //empty previous scan set
    this->area_nodes.clear();


    //fetch scan constraints
    const std::optional<std::vector<const cm_lst_node *>> &
        omit_areas_set = opts.get_omit_areas();
    const std::optional<std::vector<const cm_lst_node *>> &
        omit_objs_set = opts.get_omit_objs(); 
    const std::optional<std::vector<const cm_lst_node *>> &
        exclusive_areas_set = opts.get_exclusive_areas();
    const std::optional<std::vector<const cm_lst_node *>> &
        exclusive_objs_set = opts.get_exclusive_objs();
    const std::optional<std::vector<std::pair<uintptr_t, uintptr_t>>> &
        omit_addr_ranges = opts.get_omit_addr_ranges();
    const std::optional<std::vector<std::pair<uintptr_t, uintptr_t>>> &
        exclusive_addr_ranges = opts.get_exclusive_addr_ranges();
    const std::optional<cm_byte> access = opts.get_access();

    //fetch MemCry map
    mc_vm_map const * map = opts.get_map();
    if (map == nullptr || map->vm_areas.len == 0) {
        sc_errno = SC_ERR_OPT_NOMAP;
        _UNLOCK(-1)
        return -1;
    }


    //setup iteration over objects
    area_node = map->vm_areas.head;
    first_iter = true;
    exclusive_used = ((exclusive_objs_set.has_value() == true)
                      || (exclusive_areas_set.has_value() == true)
                      || (exclusive_addr_ranges.has_value() == true))
                      ? true : false;

    //iterate through every area
    while ((first_iter == true)
           || ((area_node != map->vm_areas.head) && (area_node != NULL))) {

        //prepare this iteration
        first_iter = false;
        area = MC_GET_NODE_AREA(area_node);


        // -- omission checks

        //check if permissions do not match
        if (access.has_value() && access.value() != 0) {
            if (!is_access(area->access, access.value()))
                goto _update_set_continue;
        }

        //check if this object is omitted or blacklisted
        obj_node = area->obj_node_p;
        if (obj_node != nullptr) {
            obj = MC_GET_NODE_OBJ(obj_node);

            //if the omit object set is provided
            if (omit_objs_set.has_value() == true) {

                //if this object is included in the omit object set
                if (is_included(obj_node, omit_objs_set.value()) == true) {

                    //can skip to the end of this object
                    area_node = get_last_obj_area(obj);
                    goto _update_set_continue;
                }
            }

            //object's pathname must not be blacklisted
            if (is_blacklisted(obj->pathname)) {

                //skip to last area of this object before continuing
                area_node = get_last_obj_area(obj);

                goto _update_set_continue;
            }
            
        } //end check if this object is omitted


        //check if this area is omitted
        if (omit_areas_set.has_value() == true) {

            if (is_included(area_node, omit_areas_set.value()) == true) {
                goto _update_set_continue;
            }
        } //end check if this area is omitted


        //check if this address range is omitted
        if (omit_addr_ranges.has_value() == true) {

            //if area is in an omit area set
            addr_range = in_addr_ranges(area, omit_addr_ranges.value());
            if (addr_range.has_value() == true) {

                //skip all areas that still fit in this range
                while (area->end_addr <= addr_range->second) {

                    area_node = area_node->next;
                    if (((area_node == map->vm_areas.head)
                        && (first_iter == false)) || (area_node == nullptr))
                        break;
                    area = MC_GET_NODE_AREA(area_node);
                }
                continue;
            }
        } //end check if this address range is omitted


        // -- exclusive checks

        //check if object is in the exclusive objects set
        if (obj != nullptr && (exclusive_objs_set.has_value() == true)) {
            if (is_included(obj_node, exclusive_objs_set.value()) == true)
                goto _update_set_add;
        }

        //check if area is in the exclusive areas set
        if (exclusive_areas_set.has_value() == true) {
            if (is_included(area_node, exclusive_areas_set.value()) == true)
                goto _update_set_add;
        }

        //check if this address range is included
        if (exclusive_addr_ranges.has_value() == true) {
            addr_range = in_addr_ranges(area,
                                        exclusive_addr_ranges.value());
            if (addr_range.has_value() == true) goto _update_set_add;
        }

        //if exclusive set(s) are used, do not include this area
        if (exclusive_used == true) goto _update_set_continue;

        //add to the scan set
        _update_set_add:
        this->area_nodes.insert(area_node);

        //advance iteration
        _update_set_continue:
        area_node = area_node->next;
        
    } //end iterate through every area

    if (this->area_nodes.empty() == true) {
        sc_errno = SC_ERR_SCAN_EMPTY;
        _UNLOCK(-1)
        return -1;
    }

    _UNLOCK(-1)
    return 0;
}


[[nodiscard]] const std::unordered_set<const cm_lst_node *> &
        sc::map_area_set::get_area_nodes() const noexcept {

    return this->area_nodes;
}



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

/*
 *  --- [EXTERNAL] ---
 */

sc_map_area_set sc_new_map_area_set() {

    try {
        return new sc::map_area_set();    

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return nullptr;
    }
}


int sc_del_map_area_set(sc_map_area_set ma_set) {
    
    //cast opaque handle into class
    sc::map_area_set * s = static_cast<sc::map_area_set *>(ma_set);

    try {
        delete s;
        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_reset_set(sc_map_area_set ma_set) {

    int ret;


    //cast opaque handle into class
    sc::map_area_set * s = static_cast<sc::map_area_set *>(ma_set);

    try {
        ret = s->reset();
        return (ret != 0) ? -1 : 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_update_set(sc_map_area_set ma_set, const sc_opt opts) {

    //cast opaque handles into classes
    sc::map_area_set * s = static_cast<sc::map_area_set *>(ma_set);
    sc::opt * o = static_cast<sc::opt *>(opts);

    try {

        //update the scan set
        std::optional<int> ret = s->update_set(*o);
        if (ret.has_value() == false) return -1;
        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_get_set(const sc_map_area_set ma_set, cm_vct * area_nodes) {

    int ret;


    //cast opaque handle into class
    sc::map_area_set * s = static_cast<sc::map_area_set *>(ma_set);

    try {
        //get the STL unordered set
        const std::unordered_set<const cm_lst_node *>
        & area_set = s->get_area_nodes();

        //convert the STL unordered set to a CMore vector
        ret = c_iface::uset_to_cmore_vct<const cm_lst_node *,
                                         const cm_lst_node *>(
                            area_nodes,area_set, std::nullopt);
        if (ret != 0) return -1;

        //sort the CMore vector
        c_iface::sort_area_vct(area_nodes);

        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}
