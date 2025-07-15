//C standard library
#include <cstdlib>
#include <cstring>

//system headers
#include <unistd.h>
#include <linux/limits.h>

//external libraries
#include <cmore.h>
#include <memcry.h>

//local headers
#include "scancry.h"
#include "map_area.hh"
#include "opt.hh"
#include "c_iface.hh"
#include "common.hh"
#include "error.hh"



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

/*
 *  --- [INTERNAL] ---
 */

//return type for constraint check functions
enum _constraint_match {
    INCLUDED = 0,
    NOT_INCLUDED = 1,
    ERROR = -1
};


//comparison function for the map area set red-black tree
_SC_DBG_STATIC enum cm_rbt_side map_area_set_compare(
    const void * area_node_0_erased, const void * area_node_1_erased) {

    cm_lst_node * area_node_0 = (cm_lst_node *) area_node_0_erased;
    cm_lst_node * area_node_1 = (cm_lst_node *) area_node_1_erased;
    
    mc_vm_area * area_0 = MC_GET_NODE_AREA(area_node_0);
    mc_vm_area * area_1 = MC_GET_NODE_AREA(area_node_1);

    if (area_0->id > area_1->id) return MORE;
    if (area_0->id < area_1->id) return LESS;

    return EQUAL;
}


[[nodiscard]] _SC_DBG_STATIC _SC_DBG_INLINE
enum _constraint_match is_blacklisted(const char * pathname) {

    //for every blacklisted pathname
    for (int i = 0; i < map_meta::pathname_blacklist_len; ++i) {

        //if pathname is blacklisted return `true`
        if(!strncmp(pathname, map_meta::pathname_blacklist[i], PATH_MAX))
            return INCLUDED;
    }

    return NOT_INCLUDED;
}


[[nodiscard]] _SC_DBG_STATIC _SC_DBG_INLINE
enum _constraint_match is_included(cm_lst_node * node, const cm_vct
                                   /* <const cm_lst_node *> */ & node_vct) {

    int ret;
    const cm_lst_node * iter_node;


    //search for this node in the node set
    for (int i = 0; i < node_vct.len; ++i) {

        //fetch next node
        ret = cm_vct_get(&node_vct, i, &iter_node);
        if (ret != 0) {
            sc_errno = SC_ERR_CMORE;
            return ERROR;
        }

        //check if this node is a match
        if (node == iter_node) return INCLUDED;
    }

    //exhausted the node vector
    return NOT_INCLUDED;
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
enum _constraint_match in_addr_ranges(
    const mc_vm_area * area, sc::addr_range & match_range,
    const cm_vct /* <sc::addr_range> */ & addr_ranges) {

    sc::addr_range * iter_addr_range;


    //for every address range
    for (int i = 0; i < addr_ranges.len; ++i) {

        //get the next address range
        iter_addr_range = (sc::addr_range *) cm_vct_get_p(&addr_ranges, i);
        if (iter_addr_range == nullptr) {
            sc_errno = SC_ERR_CMORE;
            return ERROR;
        }

        //if area fits in this range
        if ((area->start_addr >= iter_addr_range->get_start_addr())
            && (area->end_addr <= iter_addr_range->get_end_addr())) {

            //populate matching address range
            match_range = sc::addr_range(iter_addr_range->get_start_addr(),
                                         iter_addr_range->get_end_addr());

            return INCLUDED;
        }
    }

    //exhausted address range vector
    return NOT_INCLUDED;
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
 *  --- [ADDR_RANGE | PUBLIC] ---
 */

//getters
[[nodiscard]] uintptr_t sc::addr_range::get_start_addr() const noexcept {
    return this->start_addr;
}

[[nodiscard]] uintptr_t sc::addr_range::get_end_addr() const noexcept {
    return this->end_addr;
}



/*
 *  --- [OPT_MAP_AREA | PRIVATE] ---
 */

//perform a deep copy
void sc::opt_map_area::do_copy(sc::opt_map_area & opts_ma) noexcept {

    int ret;


    //acquire a read lock on the source object
    ret = opts_ma._lock_read();
    if (ret != 0) { this->_set_ctor_failed(true); return; }

    //call parent copy assignment operators
    _lockable::operator=(opts_ma);
    _ctor_failable::operator=(opts_ma);

    //copy constraint vectors
    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->omit_areas,
        opts_ma.get_omit_areas(), opts_ma)
    
    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->omit_objs,
        opts_ma.get_omit_objs(), opts_ma)

    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->exclusive_areas,
        opts_ma.get_exclusive_areas(), opts_ma)

    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->exclusive_objs,
        opts_ma.get_exclusive_objs(), opts_ma)

    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->omit_addr_ranges,
        opts_ma.get_omit_addr_ranges(), opts_ma)

    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->exclusive_addr_ranges,
        opts_ma.get_exclusive_addr_ranges(), opts_ma)

    //copy access
    this->access = opts_ma.get_access();

    //release the lock
    opts_ma._unlock();

    return;
}



/*
 *  --- [OPT_MAP_AREA | PUBLIC] ---
 */

//constructor
sc::opt_map_area::opt_map_area() noexcept
 : _lockable(), _ctor_failable(), access(sc::val_unset::access) {

    //zero out vectors
    std::memset(&this->omit_areas, 0, sizeof(this->omit_areas));
    std::memset(&this->omit_areas, 0, sizeof(this->omit_objs));
    std::memset(&this->omit_areas, 0, sizeof(this->exclusive_areas));
    std::memset(&this->omit_areas, 0, sizeof(this->exclusive_objs));
    std::memset(&this->omit_areas, 0, sizeof(this->omit_addr_ranges));
    std::memset(&this->omit_areas, 0, sizeof(this->exclusive_addr_ranges));

    return;
}


//copy constructor
sc::opt_map_area::opt_map_area(sc::opt_map_area & opts_ma) noexcept
 : _lockable(), _ctor_failable() {

    this->do_copy(opts_ma);
    return;
}


//destructor
sc::opt_map_area::~opt_map_area() noexcept {

    //destroy all initialised vectors
    _CTOR_VCT_DELETE_IF_INIT(this->omit_areas)
    _CTOR_VCT_DELETE_IF_INIT(this->omit_objs)
    _CTOR_VCT_DELETE_IF_INIT(this->exclusive_areas)
    _CTOR_VCT_DELETE_IF_INIT(this->exclusive_objs)
    _CTOR_VCT_DELETE_IF_INIT(this->omit_addr_ranges)
    _CTOR_VCT_DELETE_IF_INIT(this->exclusive_addr_ranges)

    return;
}


//copy assignment operator
sc::opt_map_area & sc::opt_map_area::operator=(
    sc::opt_map_area & opts_ma) noexcept {

    if (this != &opts_ma) this->do_copy(opts_ma);
    return *this;
}


//reset the area constraints
[[nodiscard]] int sc::opt_map_area::reset() noexcept {

    //acquire a write lock
    _LOCK_WRITE(-1);

    //destroy all initialised vectors
    common::del_vct_if_init(this->omit_areas);
    common::del_vct_if_init(this->omit_objs);
    common::del_vct_if_init(this->exclusive_areas);
    common::del_vct_if_init(this->exclusive_objs);
    common::del_vct_if_init(this->omit_addr_ranges);
    common::del_vct_if_init(this->exclusive_addr_ranges);

    //reset access
    this->access = sc::val_unset::access;

    //release the lock
    _UNLOCK

    return 0;
}


//setters & getters
_DEFINE_VCT_SETTER(sc::opt_map_area, omit_areas)
_DEFINE_VCT_GETTER(sc::opt_map_area, omit_areas)

_DEFINE_VCT_SETTER(sc::opt_map_area, omit_objs)
_DEFINE_VCT_GETTER(sc::opt_map_area, omit_objs)

_DEFINE_VCT_SETTER(sc::opt_map_area, exclusive_areas)
_DEFINE_VCT_GETTER(sc::opt_map_area, exclusive_areas)

_DEFINE_VCT_SETTER(sc::opt_map_area, exclusive_objs)
_DEFINE_VCT_GETTER(sc::opt_map_area, exclusive_objs)

_DEFINE_VCT_SETTER(sc::opt_map_area, omit_addr_ranges)
_DEFINE_VCT_GETTER(sc::opt_map_area, omit_addr_ranges)

_DEFINE_VCT_SETTER(sc::opt_map_area, exclusive_addr_ranges)
_DEFINE_VCT_GETTER(sc::opt_map_area, exclusive_addr_ranges)

_DEFINE_VALUE_SETTER(sc::opt_map_area, cm_byte, access)
_DEFINE_VALUE_GETTER(sc::opt_map_area, cm_byte, access, sc::val_bad::access)



/*
 *  --- [MAP_AREA_SET | PRIVATE] ---
 */

void sc::map_area_set::do_copy(sc::map_area_set & ma_set) noexcept {

    int ret;


    //acquire a read lock on the source object
    ret = ma_set._lock_read();
    if (ret != 0) { this->_set_ctor_failed(true); return; }

    //call parent copy assignment operators
    _lockable::operator=(ma_set);
    _ctor_failable::operator=(ma_set);

    //copy the set
    _CTOR_RBT_COPY_IF_INIT_UNLOCK(
        this->set,
        ma_set.get_set(), ma_set)
    
    //release the lock
    ma_set._unlock();

    return;
}



/*
 *  --- [MAP_AREA_SET | PUBLIC] ---
 */

//constructor
sc::map_area_set::map_area_set() noexcept
 : _lockable(), _ctor_failable() {

    //zero out the set
    std::memset(&this->set, 0, sizeof(this->set));

    return;
}


//copy constructor
sc::map_area_set::map_area_set(sc::map_area_set & ma_set) noexcept
 : _lockable(), _ctor_failable() {

    this->do_copy(ma_set);
    return;
}


//destructor
sc::map_area_set::~map_area_set() noexcept {

    //destroy the set
    _CTOR_RBT_DELETE_IF_INIT(this->set)

    return;
}


//copy assignment operator
sc::map_area_set & sc::map_area_set::operator=(
    sc::map_area_set & ma_set) noexcept {

    if (this != &ma_set) this->do_copy(ma_set);
    return *this;
}


//reset the map area set
[[nodiscard]] int sc::map_area_set::reset() noexcept {

    //acquire a write lock
    _LOCK_WRITE(-1);

    //destroy the set
    common::del_rbt_if_init(this->set);

    //release the lock
    _UNLOCK

    return 0;
}


[[nodiscard]] int sc::map_area_set::update_set(
    sc::opt_map_area & opts_ma, const mc_vm_map & map) noexcept {

    int ret, ret_val;
    cm_rbt_node * new_node;

    cm_lst_node * area_node;
    mc_vm_area * area;
    
    cm_lst_node * obj_node;
    mc_vm_obj * obj;

    bool exclusive_constraints_used;

    enum _constraint_match match;
    sc::addr_range addr_range(0x0, 0x0);

    /*
     *  NOTE: If an area is not included in an exclusive set, continue
     *        evaluation in case it is included in another exclusive set.
     *
     *        If an area is included in any omit set, do not include it.
     *
     *        Omitting sets take precedence over exclusive sets.
     */

    //acquire a write lock on self
    _LOCK_WRITE(-1);

    //acquire a read lock on the map area options
    ret = opts_ma._lock_read();
    if (ret != 0) {
        _UNLOCK
        return -1;
    }

    //destroy any existing set
    _CTOR_RBT_DELETE_IF_INIT(this->set);

    //create a new set
    cm_new_rbt(&this->set, sizeof(const cm_lst_node *),
               sizeof(const mc_vm_area *), map_area_set_compare);


    //fetch scan constraints
    const cm_vct /* <cm_lst_node *> */ & omit_areas
        = opts_ma.get_omit_areas(); 
    const cm_vct /* <cm_lst_node *> */ & omit_objs
        = opts_ma.get_omit_objs(); 
    const cm_vct /* <cm_lst_node *> */ & exclusive_areas
        = opts_ma.get_exclusive_areas(); 
    const cm_vct /* <cm_lst_node *> */ & exclusive_objs
        = opts_ma.get_exclusive_objs(); 
    const cm_vct /* <cm_lst_node *> */ & omit_addr_ranges
        = opts_ma.get_omit_addr_ranges(); 
    const cm_vct /* <cm_lst_node *> */ & exclusive_addr_ranges
        = opts_ma.get_exclusive_addr_ranges(); 
    const cm_byte access = opts_ma.get_access();

    //determine if exclusive constraints are provided
    exclusive_constraints_used = (exclusive_areas.is_init == true)
                                 || (exclusive_objs.is_init == true)
                                 || (exclusive_addr_ranges.is_init == true);

    //setup iteration over map areas
    area_node = map.vm_areas.head;
    ret_val = 0;

    //iterate through every area
    for (int i = 0; i < map.vm_areas.len; ++i) {

        //fetch the area of this node
        area = MC_GET_NODE_AREA(area_node);


        // -- omission checks

        //check if permissions do not match
        if (access != sc::val_unset::access) {
            if (!is_access(area->access, access))
                goto _update_set_continue;
        }

        //check if this object is omitted or blacklisted
        obj_node = area->obj_node_p;
        if (obj_node != nullptr) {
            obj = MC_GET_NODE_OBJ(obj_node);

            //if the omit object vector is provided
            if (omit_objs.is_init == true) {

                //check if this object is included in the omit object vector
                match = is_included(obj_node, omit_objs);
                if (match == ERROR) goto _update_set_fail;

                //if included, skip to the end of the current object
                if (match == INCLUDED) {
                    area_node = get_last_obj_area(obj);
                    goto _update_set_continue;
                }
            }

            //skip to the end of the current object if blacklisted
            match = is_blacklisted(obj->pathname);
            if (match == INCLUDED) {
                area_node = get_last_obj_area(obj);
                goto _update_set_continue;
            }
            
        } //end check if this object is omitted


        //check if this area is omitted
        if (omit_areas.is_init == true) {

            //check if this area is included in the omit object vector
            match = is_included(area_node, omit_areas);
            if (match == ERROR) goto _update_set_fail;

            //if included, skip this area
            if (match == INCLUDED) {
                goto _update_set_continue;
            }

        } //end check if this area is omitted


        //check if this address range is omitted
        if (omit_addr_ranges.is_init == true) {

            //check if this area is included in the omit address ranges
            match = in_addr_ranges(area, addr_range, omit_addr_ranges);
            if (match == ERROR) goto _update_set_fail;

            //if included, skip all areas that still fit in this range
            if (match == INCLUDED) {

                //skip all areas that still fit in this range
                while (area->end_addr <= addr_range.get_end_addr()) {

                    area_node = area_node->next;
                    if (((area_node == map.vm_areas.head) && (i != 0))
                        || (area_node == nullptr)) break;
                    area = MC_GET_NODE_AREA(area_node);
                }
                /* this raw `continue` is intentional */
                continue;
            }
        } //end check if this address range is omitted


        // -- exclusive checks

        //check if this object is in the exclusive objects vector
        if (obj != nullptr && (exclusive_objs.is_init == true)) {

            //check if object is included in the exclusive objects vector
            match = is_included(obj_node, exclusive_objs);
            if (match == ERROR) goto _update_set_fail;

            //if included, add this area to the set            
            if (match == INCLUDED) goto _update_set_add;
        }

        //check if area is in the exclusive areas set
        if (exclusive_areas.is_init == true) {

            //check if this area is in the exclusive areas vector
            match = is_included(area_node, exclusive_areas);            
            if (match == ERROR) goto _update_set_fail;

            //if included, add this area to the set
            if (match == INCLUDED) goto _update_set_add;
        }

        //check if this address range is included
        if (exclusive_addr_ranges.is_init == true) {

            //check if this area is in the exclusive address ranges vector
            match = in_addr_ranges(area, addr_range, exclusive_addr_ranges);
            if (match == ERROR) goto _update_set_fail;

            //if included, add this area to the set
            if (match == INCLUDED) goto _update_set_add;
        }

        /*
         *  NOTE: Areas that reach here are not omitted, but are not in
         *        any exclusive constraint. If exclusive constraints are
         *        used, this area should not be included.
         */

        if (exclusive_constraints_used == true) goto _update_set_continue;

        //add to the scan set
        _update_set_add:
        new_node = cm_rbt_set(&this->set, area_node,
                              MC_GET_NODE_AREA(area_node));
        if (new_node == nullptr) {
            sc_errno = SC_ERR_CMORE;
            goto _update_set_fail;
        }

        //advance iteration
        _update_set_continue:
        area_node = area_node->next;
        
    } //end iterate through every area

    //return an error if the resulting set is empty
    ret_val = 0;
    if (this->set.size == 0) {
        sc_errno = SC_ERR_SCAN_EMPTY;
        ret_val = -1;
    }

    opts_ma._unlock();
    _UNLOCK
    return ret_val;

    _update_set_fail:
    _CTOR_RBT_DELETE_IF_INIT(this->set);
    opts_ma._unlock();
    _UNLOCK
    
    return -1;
}


//getters
_DEFINE_RBT_GETTER(sc::map_area_set, set);





      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

/*
 *  --- [INTERNAL] ---
 */

//convert a C address range to a C++ address range
_SC_DBG_STATIC int _to_cc_addr_range(
    void * void_dst, const void * void_src) {

    //restore types
    sc::addr_range * dst = static_cast<sc::addr_range *>(void_dst);
    const sc_addr_range * src
        = static_cast<const sc_addr_range *>(void_src);

    //construct a new C++ address range
    *dst = sc::addr_range(src->start_addr, src->end_addr);

    return 0;
}


//convert a C++ address range to a C address range
_SC_DBG_STATIC int _from_cc_addr_range(
    void * void_dst, const void * void_src) {

    //restore types
    sc_addr_range * dst = static_cast<sc_addr_range *>(void_dst);
    const sc::addr_range * src
        = static_cast<const sc::addr_range *>(void_src);

    //build a new C address range
    dst->start_addr = src->get_start_addr();
    dst->end_addr = src->get_end_addr();

    return 0;
}


/*
 *  --- [OPT_map_area | EXTERNAL] ---
 */

//ctors & dtor
_DEFINE_C_CTOR(opt_map_area, opt_ma, sc)
_DEFINE_C_COPY_CTOR(opt_map_area, opt_ma, sc, opts_ma)
_DEFINE_C_DTOR(opt_map_area, opt_ma, sc, opts_ma)
_DEFINE_C_RESET(opt_map_area, opt_ma, sc, opts_ma)


//setters & getters
_DEFINE_C_PTR_SETTER(opt_map_area, opt_ma, cm_vct, sc, opts_ma, omit_areas)
_DEFINE_C_PTR_GETTER(opt_map_area, opt_ma, cm_vct, sc, opts_ma, omit_areas)

_DEFINE_C_PTR_SETTER(opt_map_area, opt_ma, cm_vct, sc, opts_ma, omit_objs)
_DEFINE_C_PTR_GETTER(opt_map_area, opt_ma, cm_vct, sc, opts_ma, omit_objs)

_DEFINE_C_PTR_SETTER(opt_map_area, opt_ma, cm_vct, sc, opts_ma, exclusive_areas)
_DEFINE_C_PTR_GETTER(opt_map_area, opt_ma, cm_vct, sc, opts_ma, exclusive_areas)

_DEFINE_C_PTR_SETTER(opt_map_area, opt_ma, cm_vct,sc, opts_ma,  exclusive_objs)
_DEFINE_C_PTR_GETTER(opt_map_area, opt_ma, cm_vct, sc, opts_ma, exclusive_objs)

_DEFINE_C_VCT_CONV_SETTER(opt_map_area, opt_ma, sc::addr_range, sc,
                          opts_ma, omit_addr_ranges, _to_cc_addr_range)
_DEFINE_C_VCT_CONV_GETTER(opt_map_area, opt_ma, sc_addr_range, sc,
                          opts_ma, omit_addr_ranges, _from_cc_addr_range)

_DEFINE_C_VCT_CONV_SETTER(opt_map_area, opt_ma, sc::addr_range, sc,
                          opts_ma, exclusive_addr_ranges, _to_cc_addr_range)
_DEFINE_C_VCT_CONV_GETTER(opt_map_area, opt_ma, sc_addr_range, sc,
                          opts_ma, exclusive_addr_ranges, _from_cc_addr_range)

_DEFINE_C_VALUE_SETTER(opt_map_area, opt_ma, cm_byte, sc, ma_set, access)
_DEFINE_C_VALUE_GETTER(opt_map_area, opt_ma, cm_byte, sc, ma_set, access)



//external - map area set

//ctors & dtor
_DEFINE_C_CTOR(map_area_set, ma_set, sc)
_DEFINE_C_COPY_CTOR(map_area_set, ma_set, sc, ma_set)
_DEFINE_C_DTOR(map_area_set, ma_set, sc, ma_set)
_DEFINE_C_RESET(map_area_set, ma_set, sc, ma_set)


int sc_ma_set_update_set(sc_map_area_set * ma_set,
                         sc_opt_map_area * opts_ma,
                         const mc_vm_map * map) {

    sc::map_area_set * cc_ma_set  = (sc::map_area_set *) ma_set;
    sc::opt_map_area * cc_opts_ma = (sc::opt_map_area *) opts_ma;
    
    return cc_ma_set->update_set(*cc_opts_ma, *map);
}


//setters & getters
_DEFINE_C_PTR_GETTER(map_area_set, ma_set, cm_rbt, sc, ma_set, set)
