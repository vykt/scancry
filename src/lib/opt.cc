//C standard library
#include <cstring>

//system headers
#include <unistd.h>
#include <linux/limits.h>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry.h"
#include "opt.hh"
#include "c_iface.hh"
#include "common.hh"
#include "error.hh"



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */
    
/*
 *  --- [OPT | PRIVATE] ---
 */

//perform a deep copy
void sc::opt::do_copy(sc::opt & opts) noexcept {

    int ret;


    //acquire a read lock on the source object
    ret = opts._lock_read();
    if (ret != 0) { this->_set_ctor_failed(true); return; }

    //call parent copy assignment operators
    _lockable::operator=(opts);
    _ctor_failable::operator=(opts);

    //copy file pathnames
    _CTOR_STR_COPY_IF_INIT(this->file_pathname_in,
                           opts.get_file_pathname_in());
    
    _CTOR_STR_COPY_IF_INIT(this->file_pathname_out,
                           opts.get_file_pathname_out());

    //copy sessions
    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->sessions, opts.get_sessions(), opts);

    //copy the map & address width
    this->map = opts.get_map();

    //copy the address width
    ret= opts.set_addr_width(this->addr_width);
    if (ret != 0) {
        opts._unlock();
        this->_set_ctor_failed(true);
        return;
    }

    //copy the scan set
    this->scan_set = opts._get_scan_set_mut();

    //release the lock
    opts._unlock();

    return;
}



/*
 *  --- [OPT | PUBLIC] ---
 */

//constructor
sc::opt::opt() noexcept
 : _lockable(), _ctor_failable(),
   file_pathname_out(nullptr),
   file_pathname_in(nullptr),
   map(nullptr),
   addr_width(sc::val_unset::addr_width) {

    //zero out the sessions vector & the scan set red-black tree
    std::memset(&this->sessions, 0, sizeof(this->sessions));
    std::memset(&this->scan_set, 0, sizeof(this->scan_set));

    return;
}


//copy constructor
sc::opt::opt(sc::opt & opts) noexcept {

    this->do_copy(opts);
    return;
}


//destructor
sc::opt::~opt() noexcept {

    //destroy initialised file pathnames
    _CTOR_STR_DELETE_IF_INIT(this->file_pathname_out);
    _CTOR_STR_DELETE_IF_INIT(this->file_pathname_in);

    //destroy sessions
    _CTOR_VCT_DELETE_IF_INIT(this->sessions);

    return;
}


//copy assignment operator
sc::opt & sc::opt::operator=(sc::opt & opts) noexcept {

    if (this != &opts) this->do_copy(opts);
    return *this;
}


//resetter
[[nodiscard]] int sc::opt::reset() noexcept {

    int ret;


    //reset file pathnames
    common::del_str_if_init(this->file_pathname_in);
    common::del_str_if_init(this->file_pathname_out);

    //reset sessions
    common::del_vct_if_init(this->sessions);

    //reset the map & address width
    this->map = nullptr;
    this->addr_width = sc::val_unset::addr_width;

    //reset the scan set
    ret = this->scan_set.reset();
    if (ret != 0) return -1;
}


//setters & getters
_DEFINE_STR_SETTER(sc::opt, file_pathname_out)
_DEFINE_STR_GETTER(sc::opt, file_pathname_out)

_DEFINE_STR_SETTER(sc::opt, file_pathname_in)
_DEFINE_STR_GETTER(sc::opt, file_pathname_in)

_DEFINE_VCT_SETTER(sc::opt, sessions)
_DEFINE_VCT_GETTER(sc::opt, sessions)

_DEFINE_PTR_SETTER(sc::opt, mc_vm_map, map)
_DEFINE_PTR_GETTER(sc::opt, mc_vm_map, map)

_DEFINE_ENUM_SETTER(sc::opt, sc::addr_width, addr_width)
_DEFINE_ENUM_GETTER(sc::opt, sc::addr_width, addr_width)

_DEFINE_OBJ_REF_SETTER(sc::opt, sc::map_area_set, scan_set)
_DEFINE_OBJ_REF_GETTER(sc::opt, sc::map_area_set, scan_set)
_DEFINE_OBJ_REF_GETTER_MUT(sc::opt, sc::map_area_set, scan_set)


/*
 *  --- [OPT_PTR | PRIVATE] ---
 */

//perform a deep copy
void sc::opt_ptr::do_copy(sc::opt_ptr & opts_ptr) noexcept {

    int ret;


    //acquire a read lock on the source object
    ret = opts_ptr._lock_read();
    if (ret != 0) { this->_set_ctor_failed(true); return; }

    //call parent copy assignment operators
    _lockable::operator=(opts_ptr);
    _ctor_failable::operator=(opts_ptr);

    //copy trivial attributes
    this->target_addr = opts_ptr.get_target_addr();
    this->max_obj_sz = opts_ptr.get_max_obj_sz();
    this->max_depth = opts_ptr.get_max_depth();

    //copy the static area set
    this->static_set = opts_ptr._get_static_set_mut();
    if (this->_get_ctor_failed() == true) {
        opts_ptr._unlock();
        this->_set_ctor_failed(true);
        return;
    }

    //copy the preset offsets vector
    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->preset_offsets, opts_ptr.get_preset_offsets(), opts_ptr)

    //copy the smart scan toggle
    ret = opts_ptr.get_smart_scan(this->smart_scan);
    if (ret != 0) {
        opts_ptr._unlock();
        this->_set_ctor_failed(true);
        return;
    }

    //release the lock
    opts_ptr._unlock();

    return;
}



/*
 *  --- [OPT_PTR | PUBLIC] ---
 */

//constructor
sc::opt_ptr::opt_ptr() noexcept
 : _opt_scan(),
   target_addr(sc::val_unset::target_addr),
   alignment(sc::val_default::alignment),
   max_obj_sz(sc::val_default::max_obj_sz),
   smart_scan(sc::val_default::smart_scan) {

    //zero out the preset offsets vector
    memset(&this->preset_offsets, 0, sizeof(this->preset_offsets));

    return;
}


//copy constructor
sc::opt_ptr::opt_ptr(sc::opt_ptr & opts_ptr) noexcept {

    this->do_copy(opts_ptr);
    return;
}


//destructor
sc::opt_ptr::~opt_ptr() noexcept {

    //destroy preset offsets
    _CTOR_VCT_DELETE_IF_INIT(this->preset_offsets);

    return;
}


//copy assignment operator
sc::opt_ptr & sc::opt_ptr::operator=(sc::opt_ptr & opts_ptr) noexcept {

    if (this != &opts_ptr) this->do_copy(opts_ptr);
    return *this;
}


//resetter
[[nodiscard]] int sc::opt_ptr::reset() noexcept {

    int ret;


    //acquire a write lock
    _LOCK_WRITE(-1)

    //reset trivial attributes
    this->target_addr = sc::val_unset::target_addr;
    this->alignment   = sc::val_default::alignment;
    this->max_obj_sz  = sc::val_default::max_obj_sz;
    this->max_depth   = sc::val_default::max_depth;
    this->smart_scan  = sc::val_default::smart_scan;

    //reset preset offsets
    common::del_vct_if_init(this->preset_offsets);

    //reset the static areas set
    ret = this->static_set.reset();
    if (ret != 0) {
        _UNLOCK
        return -1;
    }

    //release the lock
    _UNLOCK

    return 0;
}


//setters & getters
_DEFINE_VALUE_SETTER(sc::opt_ptr, uintptr_t, target_addr)
_DEFINE_VALUE_GETTER(sc::opt_ptr, uintptr_t,
                     sc::val_bad::target_addr, target_addr)

_DEFINE_VALUE_SETTER(sc::opt_ptr, off_t, alignment)
_DEFINE_VALUE_GETTER(sc::opt_ptr, off_t, sc::val_bad::alignment, alignment)

_DEFINE_VALUE_SETTER(sc::opt_ptr, off_t, max_obj_sz)
_DEFINE_VALUE_GETTER(sc::opt_ptr, off_t,
                     sc::val_bad::max_obj_sz, max_obj_sz)

_DEFINE_VALUE_SETTER(sc::opt_ptr, int, max_depth)
_DEFINE_VALUE_GETTER(sc::opt_ptr, int, sc::val_bad::max_depth, max_depth)

_DEFINE_OBJ_REF_SETTER(sc::opt_ptr, sc::map_area_set, static_set)
_DEFINE_OBJ_REF_GETTER(sc::opt_ptr, sc::map_area_set, static_set)

_DEFINE_VCT_SETTER(sc::opt_ptr, preset_offsets)
_DEFINE_VCT_GETTER(sc::opt_ptr, preset_offsets)

_DEFINE_ENUM_SETTER(sc::opt_ptr, sc::smart_scan, smart_scan)
_DEFINE_ENUM_GETTER(sc::opt_ptr, sc::smart_scan, smart_scan)



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

/*
 *  --- [OPT | EXTERNAL] ---
 */

//ctors & dtor
_DEFINE_C_CTOR(sc_opt, opt, opt, sc)
_DEFINE_C_COPY_CTOR(sc_opt, opt, opt, sc, opts)
_DEFINE_C_DTOR(sc_opt, opt, opt, sc, opts)
_DEFINE_C_RESET(sc_opt, opt, opt, sc, opts)


//setters & getters
_DEFINE_C_VALUE_SETTER(opt, opt, char *, sc, opts, file_pathname_out)
_DEFINE_C_PTR_GETTER(opt, opt, char *, sc, opts, file_pathname_out)

_DEFINE_C_VALUE_SETTER(opt, opt, char *, sc, opts, file_pathname_in)
_DEFINE_C_PTR_GETTER(opt, opt, char *, sc, opts, file_pathname_in)

_DEFINE_C_PTR_SETTER(opt, opt, cm_vct, sc, opts, sessions)
_DEFINE_C_PTR_GETTER(opt, opt, cm_vct, sc, opts, sessions)

_DEFINE_C_VALUE_SETTER(opt, opt, mc_vm_map *, sc, opts, map)
_DEFINE_C_VALUE_GETTER(opt, opt, mc_vm_map *, sc, opts, map)

_DEFINE_C_ENUM_SETTER(opt, opt, addr_width, sc, opts, addr_width)
_DEFINE_C_ENUM_GETTER(opt, opt, addr_width, sc, opts, addr_width)

_DEFINE_C_OBJ_SETTER(opt, opt, map_area_set, sc, opts, scan_set)
_DEFINE_C_OBJ_GETTER(opt, opt, map_area_set, sc, opts, scan_set)


/*
 *  --- [OPT_PTR | EXTERNAL] ---
 */

//ctors & dtor
_DEFINE_C_CTOR(sc_opt_ptr, opt_ptr, opt_ptr, sc)
_DEFINE_C_COPY_CTOR(sc_opt_ptr, opt_ptr, opt_ptr, sc, opts_ptr)
_DEFINE_C_DTOR(sc_opt_ptr, opt_ptr, opt_ptr, sc, opts_ptr)
_DEFINE_C_RESET(sc_opt_ptr, opt_ptr, opt_ptr, sc, opts_ptr)


//setters & getters
_DEFINE_C_VALUE_SETTER(opt_ptr, opt_ptr, uintptr_t,
                       sc, opts_ptr, target_addr)
_DEFINE_C_VALUE_GETTER(opt_ptr, opt_ptr, uintptr_t,
                       sc, opts_ptr, target_addr)

_DEFINE_C_VALUE_SETTER(opt_ptr, opt_ptr, off_t,
                       sc, opts_ptr, alignment)
_DEFINE_C_VALUE_GETTER(opt_ptr, opt_ptr, off_t,
                       sc, opts_ptr, alignment)
                    
_DEFINE_C_VALUE_SETTER(opt_ptr, opt_ptr, off_t,
                       sc, opts_ptr, max_obj_sz)
_DEFINE_C_VALUE_GETTER(opt_ptr, opt_ptr, off_t,
                       sc, opts_ptr, max_obj_sz)
                    
_DEFINE_C_VALUE_SETTER(opt_ptr, opt_ptr, int,
                       sc, opts_ptr, max_depth)
_DEFINE_C_VALUE_GETTER(opt_ptr, opt_ptr, int,
                       sc, opts_ptr, max_depth)

_DEFINE_C_OBJ_SETTER(opt_ptr, opt_ptr, map_area_set,
                     sc, opts_ptr, static_set)
_DEFINE_C_OBJ_GETTER(opt_ptr, opt_ptr, map_area_set,
                     sc, opts_ptr, static_set)

_DEFINE_C_ENUM_SETTER(opt_ptr, opt_ptr, smart_scan,
                      sc, opts_ptr, smart_scan)
_DEFINE_C_ENUM_GETTER(opt_ptr, opt_ptr, smart_scan,
                      sc, opts_ptr, smart_scan)
