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
#include "common.hh"
#include "error.hh"



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */
    
/*
 *  --- [OPT | PRIVATE] ---
 */

//perform a deep copy
void sc::opt::do_copy(const sc::opt & opts) noexcept {

    int ret;
    enum sc::addr_width addr_width;


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
    ret = opts.get_addr_width(addr_width);
    if (ret != 0) {
        opts._unlock();
        this->_set_ctor_failed(true);
        return;
    }
    this->addr_width = addr_width;

    //copy the scan set pointer
    this->scan_set = ((sc::opt &) opts).get_scan_set();

    //lock the scan set
    ret = this->scan_set->_lock_read();
    if (ret != 0) { this->_set_ctor_failed(true); return; }

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
   addr_width(sc::val_unset::addr_width),
   scan_set(nullptr) {

    //zero out the sessions vector
    std::memset(&this->sessions, 0, sizeof(this->sessions));

    return;
}


//copy constructor
sc::opt::opt(const sc::opt & opts) noexcept
 : _lockable(), _ctor_failable(),
   file_pathname_out(nullptr),
   file_pathname_in(nullptr),
   scan_set() {

    //zero out the sessions vector
    std::memset(&this->sessions, 0, sizeof(this->sessions));

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

    //unlock the scan set if one is present
    if (this->scan_set != nullptr) {
        this->scan_set->_unlock();
        this->scan_set = nullptr;
    }

    return;
}


//copy assignment operator
sc::opt & sc::opt::operator=(const sc::opt & opts) noexcept {

    if (this != &opts) this->do_copy(opts);    

    return *this;
}


//resetter
[[nodiscard]] int sc::opt::reset() noexcept {

    //reset file pathnames
    common::del_str_if_init(this->file_pathname_in);
    common::del_str_if_init(this->file_pathname_out);

    //reset sessions
    common::del_vct_if_init(this->sessions);

    //reset the map & address width
    this->map = nullptr;
    this->addr_width = sc::val_unset::addr_width;

    //reset the scan set
    if (this->scan_set != nullptr) {
        this->scan_set->_unlock();
        this->scan_set = nullptr;
    }
    
    return 0;
}


//setters & getters
_DEFINE_STR_SETTER(sc::opt, file_pathname_out)
_DEFINE_STR_GETTER(sc::opt, file_pathname_out)

_DEFINE_STR_SETTER(sc::opt, file_pathname_in)
_DEFINE_STR_GETTER(sc::opt, file_pathname_in)

_DEFINE_VCT_SETTER(sc::opt, sessions)
_DEFINE_VCT_GETTER(sc::opt, sessions)

_DEFINE_VALUE_SETTER(sc::opt, mc_vm_map *, map)
_DEFINE_VALUE_GETTER(sc::opt, mc_vm_map *, map, sc::val_bad::map)

_DEFINE_ENUM_SETTER(sc::opt, sc::addr_width, addr_width)
_DEFINE_ENUM_GETTER(sc::opt, sc::addr_width, addr_width)

_DEFINE_OBJ_SETTER(sc::opt, sc::map_area_set, scan_set)
_DEFINE_OBJ_GETTER(sc::opt, sc::map_area_set, scan_set)




/*
 *  --- [OPT_PTRSCAN | PRIVATE] ---
 */

//perform a deep copy
void sc::opt_ptrscan::do_copy(const sc::opt_ptrscan & opts_ptr) noexcept {

    int ret;
    enum sc::smart_scan smart_scan;


    //acquire a read lock on the source object
    ret = opts_ptr._lock_read();
    if (ret != 0) { this->_set_ctor_failed(true); return; }

    //call parent copy assignment operators
    _lockable::operator=(opts_ptr);
    _ctor_failable::operator=(opts_ptr);

    //copy trivial attributes
    this->target_addr = opts_ptr.get_target_addr();
    this->alignment   = opts_ptr.get_alignment();
    this->max_obj_sz  = opts_ptr.get_max_obj_sz();
    this->max_depth   = opts_ptr.get_max_depth();

    //copy the static area set
    this->static_set = ((sc::opt_ptrscan &) opts_ptr).get_static_set();

    //lock the scan set
    ret = this->static_set->_lock_read();
    if (ret != 0) { this->_set_ctor_failed(true); return; }

    //copy the preset offsets vector
    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->preset_offsets, opts_ptr.get_preset_offsets(), opts_ptr)

    //copy the smart scan toggle
    ret = opts_ptr.get_smart_scan(smart_scan);
    if (ret != 0) {
        opts_ptr._unlock();
        this->_set_ctor_failed(true);
        return;
    }
    this->smart_scan = smart_scan;

    //release the lock
    opts_ptr._unlock();

    return;
}



/*
 *  --- [OPT_PTR | PUBLIC] ---
 */

//constructor
sc::opt_ptrscan::opt_ptrscan() noexcept
 : _opt_scan(),
   target_addr(sc::val_unset::target_addr),
   alignment(sc::val_default::alignment),
   max_obj_sz(sc::val_default::max_obj_sz),
   max_depth(sc::val_default::max_depth),
   static_set(nullptr),
   smart_scan(sc::val_default::smart_scan) {

    //zero out the preset offsets vector
    memset(&this->preset_offsets, 0, sizeof(this->preset_offsets));

    return;
}


//copy constructor
sc::opt_ptrscan::opt_ptrscan(const sc::opt_ptrscan & opts_ptr) noexcept
 : _opt_scan() {

    this->do_copy(opts_ptr);
    return;
}


//destructor
sc::opt_ptrscan::~opt_ptrscan() noexcept {

    //destroy preset offsets
    _CTOR_VCT_DELETE_IF_INIT(this->preset_offsets);

    //reset the static set
    if (this->static_set != nullptr) {
        this->static_set->_unlock();
        this->static_set = nullptr;
    }

    return;
}


//copy assignment operator
sc::opt_ptrscan & sc::opt_ptrscan::operator=(
    const sc::opt_ptrscan & opts_ptr) noexcept {

    if (this != &opts_ptr) this->do_copy(opts_ptr);
    return *this;
}


//resetter
[[nodiscard]] int sc::opt_ptrscan::reset() noexcept {

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

    //reset the static set
    if (this->static_set != nullptr) {
        this->static_set->_unlock();
        this->static_set = nullptr;
    }

    //release the lock
    _UNLOCK

    return 0;
}


//setters & getters
_DEFINE_VALUE_SETTER(sc::opt_ptrscan, uintptr_t, target_addr)
_DEFINE_VALUE_GETTER(sc::opt_ptrscan, uintptr_t,
                     target_addr, sc::val_bad::target_addr)

_DEFINE_VALUE_SETTER(sc::opt_ptrscan, off_t, alignment)
_DEFINE_VALUE_GETTER(sc::opt_ptrscan, off_t,
                     alignment, sc::val_bad::alignment)

_DEFINE_VALUE_SETTER(sc::opt_ptrscan, off_t, max_obj_sz)
_DEFINE_VALUE_GETTER(sc::opt_ptrscan, off_t,
                     max_obj_sz, sc::val_bad::max_obj_sz)

_DEFINE_VALUE_SETTER(sc::opt_ptrscan, int, max_depth)
_DEFINE_VALUE_GETTER(sc::opt_ptrscan, int,
                     max_depth, sc::val_bad::max_depth)

_DEFINE_OBJ_SETTER(sc::opt_ptrscan, sc::map_area_set, static_set)
_DEFINE_OBJ_GETTER(sc::opt_ptrscan, sc::map_area_set, static_set)

_DEFINE_VCT_SETTER(sc::opt_ptrscan, preset_offsets)
_DEFINE_VCT_GETTER(sc::opt_ptrscan, preset_offsets)

_DEFINE_ENUM_SETTER(sc::opt_ptrscan, sc::smart_scan, smart_scan)
_DEFINE_ENUM_GETTER(sc::opt_ptrscan, sc::smart_scan, smart_scan)



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

/*
 *  --- [OPT | EXTERNAL] ---
 */

//ctors & dtor
_DEFINE_C_CTOR(opt, opt, sc)
_DEFINE_C_COPY_CTOR(opt, opt, sc, opts)
_DEFINE_C_COPY_ASSIGN(opt, opt, sc, dst_opts, src_opts)
_DEFINE_C_DTOR(opt, opt, sc, opts)
_DEFINE_C_RESET(opt, opt, sc, opts)


//setters & getters
_DEFINE_C_STR_SETTER(opt, opt, sc, opts, file_pathname_out)
_DEFINE_C_STR_GETTER(opt, opt, sc, opts, file_pathname_out)

_DEFINE_C_STR_SETTER(opt, opt, sc, opts, file_pathname_in)
_DEFINE_C_STR_GETTER(opt, opt, sc, opts, file_pathname_in)

_DEFINE_C_VCT_SETTER(opt, opt, sc, opts, sessions)
_DEFINE_C_VCT_GETTER(opt, opt, sc, opts, sessions)

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
_DEFINE_C_CTOR(opt_ptrscan, opt_ptr, sc)
_DEFINE_C_COPY_CTOR(opt_ptrscan, opt_ptr, sc, opts_ptr)
_DEFINE_C_COPY_ASSIGN(opt_ptrscan, opt_ptr, sc, dst_opts_ptr, src_opts_tr)
_DEFINE_C_DTOR(opt_ptrscan, opt_ptr, sc, opts_ptr)
_DEFINE_C_RESET(opt_ptrscan, opt_ptr, sc, opts_ptr)


//setters & getters
_DEFINE_C_VALUE_SETTER(opt_ptrscan, opt_ptr, uintptr_t,
                       sc, opts_ptr, target_addr)
_DEFINE_C_VALUE_GETTER(opt_ptrscan, opt_ptr, uintptr_t,
                       sc, opts_ptr, target_addr)

_DEFINE_C_VALUE_SETTER(opt_ptrscan, opt_ptr, off_t,
                       sc, opts_ptr, alignment)
_DEFINE_C_VALUE_GETTER(opt_ptrscan, opt_ptr, off_t,
                       sc, opts_ptr, alignment)
                    
_DEFINE_C_VALUE_SETTER(opt_ptrscan, opt_ptr, off_t,
                       sc, opts_ptr, max_obj_sz)
_DEFINE_C_VALUE_GETTER(opt_ptrscan, opt_ptr, off_t,
                       sc, opts_ptr, max_obj_sz)
                    
_DEFINE_C_VALUE_SETTER(opt_ptrscan, opt_ptr, int,
                       sc, opts_ptr, max_depth)
_DEFINE_C_VALUE_GETTER(opt_ptrscan, opt_ptr, int,
                       sc, opts_ptr, max_depth)

_DEFINE_C_OBJ_SETTER(opt_ptrscan, opt_ptr, map_area_set,
                     sc, opts_ptr, static_set)
_DEFINE_C_OBJ_GETTER(opt_ptrscan, opt_ptr, map_area_set,
                     sc, opts_ptr, static_set)

_DEFINE_C_VCT_SETTER(opt_ptrscan, opt_ptr, sc,
                     opts_ptr, preset_offsets)
_DEFINE_C_VCT_GETTER(opt_ptrscan, opt_ptr, sc,
                     opts_ptr, preset_offsets)

_DEFINE_C_ENUM_SETTER(opt_ptrscan, opt_ptr, smart_scan,
                      sc, opts_ptr, smart_scan)
_DEFINE_C_ENUM_GETTER(opt_ptrscan, opt_ptr, smart_scan,
                      sc, opts_ptr, smart_scan)

