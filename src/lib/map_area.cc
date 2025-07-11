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
[[nodiscard]] uintptr_t sc::addr_range::get_start_addr() noexcept {
    return this->start_addr;
}

[[nodiscard]] uintptr_t sc::addr_range::get_end_addr() noexcept {
    return this->end_addr;
}



/*
 *  --- [MAP_AREA_OPT | PUBLIC] ---
 */

//constructor
sc::map_area_opt::map_area_opt()
 : _lockable(), _ctor_failable(), access(sc::_access_unset) {}


//copy constructor
sc::map_area_opt::map_area_opt(
    sc::map_area_opt & ma_opts)
 : _lockable(), _ctor_failable(), access(ma_opts.access) {

    int ret;

    //acquire a write lock on the source object
    ret = ma_opts._lock_write();
    if (ret != 0) { this->_set_ctor_failed(true); return; }

    //copy constraint vectors
    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->omit_areas,
        ma_opts.get_omit_areas(), ma_opts)
    
    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->omit_objs,
        ma_opts.get_omit_objs(), ma_opts)

    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->exclusive_areas,
        ma_opts.get_exclusive_areas(), ma_opts)

    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->exclusive_objs,
        ma_opts.get_exclusive_objs(), ma_opts)

    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->omit_addr_ranges,
        ma_opts.get_omit_addr_ranges(), ma_opts)

    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->exclusive_addr_ranges,
        ma_opts.get_exclusive_addr_ranges(), ma_opts)

    //release the lock
    ma_opts._unlock();

    return;
}


//destructor
sc::map_area_opt::~map_area_opt() {

    //destroy all initialised vectors
    _CTOR_VCT_DELETE_IF_INIT(this->omit_areas)
    _CTOR_VCT_DELETE_IF_INIT(this->omit_objs)
    _CTOR_VCT_DELETE_IF_INIT(this->exclusive_areas)
    _CTOR_VCT_DELETE_IF_INIT(this->exclusive_objs)
    _CTOR_VCT_DELETE_IF_INIT(this->omit_addr_ranges)
    _CTOR_VCT_DELETE_IF_INIT(this->exclusive_addr_ranges)

    return;
}


//reset the area constraints
[[nodiscard]] int sc::map_area_opt::reset() noexcept {

    //acquire a write lock
    _LOCK_WRITE(-1);

    //destroy all initialised vectors
    _CTOR_VCT_DELETE_IF_INIT(this->omit_areas);
    _CTOR_VCT_DELETE_IF_INIT(this->omit_objs);
    _CTOR_VCT_DELETE_IF_INIT(this->exclusive_areas);
    _CTOR_VCT_DELETE_IF_INIT(this->exclusive_objs);
    _CTOR_VCT_DELETE_IF_INIT(this->omit_addr_ranges);
    _CTOR_VCT_DELETE_IF_INIT(this->exclusive_addr_ranges);

    //reset access
    this->access = sc::_access_unset;

    //release the lock
    _UNLOCK

    return 0;
}


//setters & getters
_DEFINE_VCT_SETTER(sc::map_area_opt, omit_areas)
_DEFINE_VCT_GETTER(sc::map_area_opt, omit_areas)
_DEFINE_VCT_GETTER_MUT(sc::map_area_opt, omit_areas)

_DEFINE_VCT_SETTER(sc::map_area_opt, omit_objs)
_DEFINE_VCT_GETTER(sc::map_area_opt, omit_objs)
_DEFINE_VCT_GETTER_MUT(sc::map_area_opt, omit_objs)

_DEFINE_VCT_SETTER(sc::map_area_opt, exclusive_areas)
_DEFINE_VCT_GETTER(sc::map_area_opt, exclusive_areas)
_DEFINE_VCT_GETTER_MUT(sc::map_area_opt, exclusive_areas)

_DEFINE_VCT_SETTER(sc::map_area_opt, exclusive_objs)
_DEFINE_VCT_GETTER(sc::map_area_opt, exclusive_objs)
_DEFINE_VCT_GETTER_MUT(sc::map_area_opt, exclusive_objs)

_DEFINE_VCT_SETTER(sc::map_area_opt, omit_addr_ranges)
_DEFINE_VCT_GETTER(sc::map_area_opt, omit_addr_ranges)
_DEFINE_VCT_GETTER_MUT(sc::map_area_opt, omit_addr_ranges)

_DEFINE_VCT_SETTER(sc::map_area_opt, exclusive_addr_ranges)
_DEFINE_VCT_GETTER(sc::map_area_opt, exclusive_addr_ranges)
_DEFINE_VCT_GETTER_MUT(sc::map_area_opt, exclusive_addr_ranges)

_DEFINE_PRIM_SETTER(sc::map_area_opt, cm_byte, access)
_DEFINE_PRIM_GETTER(sc::map_area_opt, cm_byte, CM_BYTE_MAX, access)


/*
 *  --- [MAP_AREA_SET | PUBLIC] ---
 */

//constructor
sc::map_area_set::map_area_set()
 : _lockable(), _ctor_failable() {}


//copy constructor
sc::map_area_set::map_area_set(sc::map_area_set & ma_set)
 : _lockable(), _ctor_failable() {

    int ret;


    //acquire a write lock on the source object
    ret = ma_set._lock_write();
    if (ret != 0) { this->_set_ctor_failed(true); return; }

    //copy the set
    _CTOR_RBT_COPY_IF_INIT_UNLOCK(
        this->set,
        ma_set.get_set(), ma_set)
    
    //release the lock
    ma_set._unlock();

    return;
}


//destructor
sc::map_area_set::~map_area_set() {

    //destroy the set
    _CTOR_RBT_DELETE_IF_INIT(this->set)

    return;
}


//reset the map area set
[[nodiscard]] int sc::map_area_set::reset() noexcept {

    //acquire a write lock
    _LOCK_WRITE(-1);

    //destroy the set
    _CTOR_RBT_DELETE_IF_INIT(this->set);

    //release the lock
    _UNLOCK

    return 0;
}


[[nodiscard]] int sc::map_area_set::update_set(
    sc::map_area_opt & ma_opts, const mc_vm_map & map) noexcept {

    int ret, ret_val;
    cm_rbt_node * new_node;

    cm_lst_node * area_node;
    mc_vm_area * area;
    
    cm_lst_node * obj_node;
    mc_vm_obj * obj;

    bool exclusive_constraints_used;

    enum _constraint_match match;
    sc::addr_range addr_range(0x0, 0x0);
    uintptr_t end_addr;

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
    ret = ma_opts._lock_read();
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
        = ma_opts.get_omit_areas(); 
    const cm_vct /* <cm_lst_node *> */ & omit_objs
        = ma_opts.get_omit_objs(); 
    const cm_vct /* <cm_lst_node *> */ & exclusive_areas
        = ma_opts.get_exclusive_areas(); 
    const cm_vct /* <cm_lst_node *> */ & exclusive_objs
        = ma_opts.get_exclusive_objs(); 
    const cm_vct /* <cm_lst_node *> */ & omit_addr_ranges
        = ma_opts.get_omit_addr_ranges(); 
    const cm_vct /* <cm_lst_node *> */ & exclusive_addr_ranges
        = ma_opts.get_exclusive_addr_ranges(); 
    const cm_byte access = ma_opts.get_access();

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
        if (access != sc::_access_unset) {
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

    ma_opts._unlock();
    _UNLOCK
    return ret_val;

    _update_set_fail:
    _CTOR_RBT_DELETE_IF_INIT(this->set);
    ma_opts._unlock();
    _UNLOCK
    
    return -1;
}


//getters
_DEFINE_RBT_GETTER(sc::map_area_set, set);
_DEFINE_RBT_GETTER_MUT(sc::map_area_set, set);





      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

/*
 *  --- [MAP_AREA_OPT | EXTERNAL] ---
 */

//ctors & dtor
_DEFINE_C_CTOR(sc_map_area_opt, map_area_opt, ma_opt, sc)
_DEFINE_C_COPY_CTOR(sc_map_area_opt, map_area_opt, ma_opt, sc, ma_opts)
_DEFINE_C_DTOR(sc_map_area_opt, map_area_opt, ma_opt, sc, ma_opts)
_DEFINE_C_RESET(sc_map_area_opt, map_area_opt, ma_opt, sc, ma_opts)

//setters & getters
int sc_ma_opt_set_omit_areas(
    sc_map_area_opt * ma_opts, const cm_vct * omit_areas) {

    _C_SETTER_PRELUDE(map_area_opt, int, sc, ma_opts)
    ret = cc_ma_opts->set_omit_areas(*omit_areas);
    _C_SETTER_POSTLUDE(-1, -1, 0)
}


const cm_vct * sc_ma_opt_get_omit_areas(sc_map_area_opt * ma_opts) {

    _C_GETTER_PRELUDE(map_area_opt, sc, ma_opts)
    return &cc_ma_opts->get_omit_areas();
}


int sc_ma_opt_set_omit_objs(
    sc_map_area_opt * ma_opts, const cm_vct * omit_objs) {

    _C_SETTER_PRELUDE(map_area_opt, int, sc, ma_opts)
    ret = cc_ma_opts->set_omit_objs(*omit_objs);
    _C_SETTER_POSTLUDE(-1, -1, 0)
}


const cm_vct * sc_ma_opt_get_omit_objs(sc_map_area_opt * ma_opts) {
    
    _C_GETTER_PRELUDE(map_area_opt, sc, ma_opts)
    return &cc_ma_opts->get_omit_objs();
}


//exclusive areas
int sc_ma_opt_set_exclusive_areas(
    sc_map_area_opt * ma_opts, const cm_vct * exclusive_areas) {
        
    _C_SETTER_PRELUDE(map_area_opt, int, sc, ma_opts)
    ret = cc_ma_opts->set_omit_objs(*exclusive_areas);
    _C_SETTER_POSTLUDE(-1, -1, 0)
}
    
const cm_vct * sc_ma_opt_get_exclusive_areas(sc_map_area_opt * ma_opts) {
    
    _C_GETTER_PRELUDE(map_area_opt, sc, ma_opts)
    return &cc_ma_opts->get_exclusive_areas();
}


int sc_ma_opt_set_exclusive_objs(
    sc_map_area_opt * ma_opts, const cm_vct * exclusive_objs) {

    _C_SETTER_PRELUDE(map_area_opt, int, sc, ma_opts)
    ret = cc_ma_opts->set_omit_objs(*exclusive_objs);
    _C_SETTER_POSTLUDE(-1, -1, 0)
}


const cm_vct * sc_ma_opt_get_exclusive_objs(sc_map_area_opt * ma_opts) {
    
    _C_GETTER_PRELUDE(map_area_opt, sc, ma_opts)
    return &cc_ma_opts->get_exclusive_objs();
}


int sc_ma_opt_set_omit_addr_ranges(
    sc_map_area_opt * ma_opts, const cm_vct * omit_addr_ranges) {

    sc_addr_range * addr_range;
    sc::addr_range cc_addr_range(0x0, 0x0);
    cm_vct cc_omit_addr_ranges;

        
    _C_SETTER_PRELUDE(map_area_opt, int, sc, ma_opts)

    /*
     *  NOTE: Need to convert between C and C++ address ranges.
     */

    /*
     * TODO: All of this is way too excessive, create a separate
     *       function for converting between 2 vectors of C and C++
     *       structures using a callback. It should handle both C <-> C++.
     */

    //allocate a new C++ address ranges vector
    ret = cm_new_vct(&cc_omit_addr_ranges, sizeof(sc::addr_range));
    if (ret != 0) { sc_errno = SC_ERR_CMORE; return -1; }

    //for every C address range
    for (int i = 0; i < omit_addr_ranges->len; ++i) {

        //fetch the address range
        addr_range = (sc_addr_range *) cm_vct_get_p(omit_addr_ranges, i);
        if (addr_range == nullptr) goto _sc_ma_opt_set_omit_addr_ranges_fail;

        //construct a C++ address range
        cc_addr_range = sc::addr_range(addr_range->min, addr_range->max);
        ret = cm_vct_apd(&cc_omit_addr_ranges, &cc_addr_range);
        if (ret != 0) goto _sc_ma_opt_set_omit_addr_ranges_fail;
    }

    //set the 
    ret = cc_ma_opts->set_omit_objs(cc_omit_addr_ranges);
    if (ret != 0) {cm_del_vct( &cc_omit_addr_ranges); return -1; }
    
    _C_SETTER_POSTLUDE(-1, -1, 0)

    _sc_ma_opt_set_omit_addr_ranges_fail:
    sc_errno = SC_ERR_CMORE;
    cm_del_vct(&cc_omit_addr_ranges);
    return -1;
}

    
const cm_vct * sc_ma_opt_get_exclusive_objs(sc_map_area_opt * ma_opts);

//exclusive address ranges
int sc_ma_opt_set_exclusive_addr_ranges(
    sc_map_area_opt * ma_opts, const cm_vct * exclusive_addr_ranges);
const cm_vct * sc_ma_opt_get_exclusive_addr_ranges(
    sc_map_area_opt * ma_opts);

//access
//0 = success, CM_BYTE_MAX = error
int sc_ma_opt_set_access(
    sc_map_area_opt * ma_opts, const cm_byte access);
//CM_BYTE_MAX = error, SC_ACCESS_UNSET = not set, other = success
cm_byte sc_ma_opt_get_access(sc_map_area_opt * ma_opts);


//external - map area set

//pointer = success, NULL = error
sc_map_area_set * sc_new_ma_set();
sc_map_area_set * sc_copy_ma_set(sc_map_area_set * ma_set);
//0 = success, -1 = error
void sc_del_ma_set(sc_map_area_set * ma_set);
int sc_ma_set_reset(sc_map_area_set * ma_set);

//0 = success, -1 = error
int sc_ma_set_update_set(sc_map_area_set * ma_set,
                                sc_map_area_opt * ma_opts,
                                const mc_vm_map * map);
//pointer = success, -1 = error
const cm_rbt * sc_get_set(sc_map_area_set * ma_set);


/*
 *  --- [MAP_AREA_SET | EXTERNAL] ---
 */

sc_map_area_set sc_new_map_area_set() {

    void *         

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
