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
    this->addr_width = opts.get_addr_width();

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

_DEFINE_VALUE_SETTER(sc::opt, enum sc::addr_width, addr_width)
_DEFINE_VALUE_GETTER(sc::opt, enum sc::addr_width,
                     sc::val_unset::addr_width, addr_width)

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
    this->smart_scan = opts_ptr.get_smart_scan();

    //copy the static area set
    this->static_set = opts_ptr._get_static_set_mut();
    if (this->_get_ctor_failed() == true) {
        opts_ptr._unlock();
        return;
    }

    //copy the preset offsets vector
    _CTOR_VCT_COPY_IF_INIT_UNLOCK(
        this->preset_offsets, opts_ptr.get_preset_offsets(), opts_ptr)

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



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

/*
 *  --- [INTERNAL] ---
 */

//wrapper for setting a STL vector in C
template <typename O, typename T, typename T_c>
[[nodiscard]] _SC_DBG_STATIC int _vector_setter(
                    void * opts_X, const cm_vct * cmore_vct,
                    int (O::*set)(const std::optional<std::vector<T>> &),
                    std::optional<std::function<int(T *, T_c *)>> convert_cb) {

    int ret;

    
    //cast opaque handle into class
    O * o = static_cast<O *>(opts_X);

    //call the setter with the STL vector
    try {
        if (cmore_vct == nullptr) {
            (o->*set)(std::nullopt);
            
        } else {
            //create a STL vector
            std::vector<T> stl_vct;
            ret = c_iface::vct_from_cmore_vct<T, T_c>(
                cmore_vct, stl_vct, convert_cb);
            if (ret != 0) return -1;

            //perform the set
            (o->*set)(stl_vct);
        }
        return 0;

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


//wrapper for getting a STL vector in C
template <typename O, typename T, typename T_c>
_SC_DBG_STATIC int _vector_getter(
                    void * opts_X, cm_vct * cmore_vct,
                    const std::optional<std::vector<T>> & (O::*get)() const,
                    std::optional<std::function<int(T_c *, T *)>> convert_cb) {

    int ret;

    
    //cast opaque handle into class
    O * o = static_cast<O *>(opts_X);

    //copy contents of the STL vector into the CMore vector.
    try {
        //get the STL vector
        const std::optional<std::vector<T>> & stl_vct = (o->*get)();

        //if this constraint doesn't have a value, fail
        if (stl_vct.has_value() == false) {
            sc_errno = SC_ERR_OPT_EMPTY;
            return -1;
        }

        //perform the copy
        ret = c_iface::vct_to_cmore_vct<T, T_c>(
            cmore_vct, stl_vct.value(), convert_cb);
        if (ret != 0) return -1;

        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}



//generic getter of constraints
template <typename O, typename T, typename T_c>
_SC_DBG_STATIC int _unordered_set_getter(
                void * opts_X, cm_vct * cmore_vct,
                const std::optional<std::unordered_set<T>> & (O::*get)() const,
                std::optional<std::function<int(T_c *, T *)>> convert_cb) {

    int ret;

    
    //cast opaque handle into class
    O * o = static_cast<O *>(opts_X);

    //copy contents of the STL vector into the CMore vector.
    try {
        //get the STL unordered set
        const std::optional<std::unordered_set<T>> & stl_uset = (o->*get)();

        //if this constraint doesn't have a value, fail
        if (stl_uset.has_value() == false) {
            sc_errno = SC_ERR_OPT_EMPTY;
            return -1;
        }

        //perform the copy
        ret = c_iface::uset_to_cmore_vct<T, T_c>(
            cmore_vct, stl_uset.value(), convert_cb);
        if (ret != 0) return -1;

        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}



/*
 *  --- [OPT | EXTERNAL] ---
 */

//new class opt
sc_opt sc_new_opt(const enum sc_addr_width addr_width) {

    //convert C enum to C++
    enum sc::addr_width width = (addr_width == AW64) ? sc::AW64 : sc::AW32;

    try {
        return new sc::opt(width);

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return nullptr;
    }
};


sc_opt sc_copy_opt(const sc_opt opts) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    try {
        return new sc::opt(*o);
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return nullptr;
    }
}


//delete class opt
int sc_del_opt(sc_opt opts) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    try {
        delete o;
        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


//reset class opt
int sc_opt_reset(sc_opt opts) {

    int ret;

    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    try {
        ret = o->reset();
        return (ret != 0) ? -1 : 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


//set class opt->file_path_out
int sc_opt_set_file_path_out(sc_opt opts, const char * path) {

    int ret;


    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    try {
        if (path == nullptr) ret = o->set_file_path_out(std::nullopt);
        else ret = o->set_file_path_out(path);
        return (ret != 0) ? -1 : 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


const char * sc_opt_get_file_path_out(const sc_opt opts) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //return NULL if optional is not set or there is an error
    try {
        const std::optional<std::string> & file_path_out
            = o->get_file_path_out();

        if (file_path_out.has_value() && !file_path_out.value().empty()) {
            return file_path_out.value().c_str();
        } else {
            sc_errno = SC_ERR_OPT_EMPTY;
            return nullptr;
        }
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return nullptr;
    }
}


int sc_opt_set_file_path_in(sc_opt opts, const char * path) {

    int ret;


    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);
    
    try {
        if (path == nullptr) ret = o->set_file_path_in(std::nullopt);
        else ret = o->set_file_path_in(path);
        return (ret != 0) ? -1 : 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


const char * sc_opt_get_file_path_in(const sc_opt opts) {
    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //return NULL if optional is not set or there is an error
    try {
        const std::optional<std::string> & file_path_in
            = o->get_file_path_in();

        if (file_path_in.has_value() && !file_path_in.value().empty()) {
            return file_path_in.value().c_str();
        } else {
            sc_errno = SC_ERR_OPT_EMPTY;
            return nullptr;
        }
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return nullptr;
    }
}


int sc_opt_set_sessions(sc_opt opts, const cm_vct * sessions) {

    int ret;


    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //create a STL vector
    std::vector<const mc_session *> v;
    v.resize(sessions->len);
    std::memcpy(v.data(), sessions->data,
                sessions->data_sz * sessions->len);

    //call the setter with the STL vector
    try {
        ret = o->set_sessions(v);
        return (ret != 0) ? -1 : 0; 

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_opt_get_sessions(const sc_opt opts, cm_vct * sessions) {
    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //initialise the CMore vector
    int ret = cm_new_vct(sessions, sizeof(cm_lst_node *));
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        return -1;
    }

    //copy contents of the STL vector into the CMore vector.
    try {
        //get the STL vector
        const std::vector<const mc_session *> & v = o->get_sessions();

        //copy the STL vector into the CMore vector
        ret = cm_vct_rsz(sessions, v.size());
        if (ret != 0) {
            sc_errno = SC_ERR_CMORE;
            return -1;
        }
        std::memcpy(sessions->data, v.data(),
                    v.size() * sizeof(cm_lst_node *));
        return 0;
        
    } catch (const std::exception & excp) {
        cm_del_vct(sessions);
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_opt_set_map(sc_opt opts, const mc_vm_map * map) {

    int ret;


    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    ret = o->set_map(map);
    return (ret != 0) ? -1 : 0;
}


mc_vm_map * sc_opt_get_map(const sc_opt opts) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    return o->get_map();
}


enum sc_addr_width sc_opt_get_addr_width(const sc_opt opts) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //convert C++ enum to C
    return (o->addr_width == sc::AW64) ? AW64 : AW32;
}


/*
 *  NOTE: In these setter functions, the C++ setter is passed a STL vector
 *        constructed from a CMore vector
 */

int sc_opt_set_omit_areas(sc_opt opts, const cm_vct * omit_areas) {

    //call generic setter
    return _vector_setter<sc::opt, const cm_lst_node *, const cm_lst_node *>(
        opts, omit_areas, &sc::opt::set_omit_areas, std::nullopt);
}


int sc_opt_get_omit_areas(const sc_opt opts, cm_vct * omit_areas) {

    //call generic getter
    return _vector_getter<sc::opt, const cm_lst_node *, const cm_lst_node *>(
        opts, omit_areas, &sc::opt::get_omit_areas, std::nullopt);
}


int sc_opt_set_omit_objs(sc_opt opts, const cm_vct * omit_objs) {

    //call generic setter
    return _vector_setter<sc::opt, const cm_lst_node *, const cm_lst_node *>(
        opts, omit_objs, &sc::opt::set_omit_objs, std::nullopt);
}


int sc_opt_get_omit_objs(const sc_opt opts, cm_vct * omit_objs) {

    //call generic getter
    return _vector_getter<sc::opt, const cm_lst_node *, const cm_lst_node *>(
        opts, omit_objs, &sc::opt::get_omit_objs, std::nullopt);
}


int sc_opt_set_exclusive_areas(sc_opt opts, const cm_vct * exclusive_areas) {

    //call generic setter
    return _vector_setter<sc::opt, const cm_lst_node *, const cm_lst_node *>(
        opts, exclusive_areas, &sc::opt::set_exclusive_areas, std::nullopt);
}


int sc_opt_get_exclusive_areas(const sc_opt opts, cm_vct * exclusive_areas) {
    
    //call generic getter
    return _vector_getter<sc::opt, const cm_lst_node *, const cm_lst_node *>(
        opts, exclusive_areas, &sc::opt::get_exclusive_areas, std::nullopt);
}


int sc_opt_set_exclusive_objs(sc_opt opts, const cm_vct * exclusive_objs) {

    //call generic setter
    return _vector_setter<sc::opt, const cm_lst_node *, const cm_lst_node *>(
        opts, exclusive_objs, &sc::opt::set_exclusive_objs, std::nullopt);
}


int sc_opt_get_exclusive_objs(const sc_opt opts, cm_vct * exclusive_objs) {

    //call generic getter
    return _vector_getter<sc::opt, const cm_lst_node *, const cm_lst_node *>(
        opts, exclusive_objs, &sc::opt::get_exclusive_objs, std::nullopt);
}


int sc_opt_set_omit_addr_ranges(
        sc_opt opts, const cm_vct * addr_ranges) {

    //call generic setter
    return _vector_setter<sc::opt,
                          std::pair<uintptr_t, uintptr_t>,
                          sc_addr_range>(
        opts, addr_ranges, &sc::opt::set_omit_addr_ranges,
            //start lambda
            [](std::pair<uintptr_t, uintptr_t> * cc_addr_range,
            sc_addr_range * c_addr_range) -> int {

                *cc_addr_range = std::pair<uintptr_t, uintptr_t>(
                    c_addr_range->min, c_addr_range->max);
                return 0;
        }); //end lambda
}


int sc_opt_get_omit_addr_ranges(
        const sc_opt opts, cm_vct * addr_ranges) {

    //call generic getter
    return _vector_getter<sc::opt,
                          std::pair<uintptr_t, uintptr_t>,
                          sc_addr_range>(
        opts, addr_ranges, &sc::opt::get_omit_addr_ranges,
            //start lambda
            [](sc_addr_range * c_addr_range,
            std::pair<uintptr_t, uintptr_t> * cc_addr_range) -> int {

                c_addr_range->min = cc_addr_range->first;
                c_addr_range->max = cc_addr_range->second;
                return 0;
        }); //end lambda
}


int sc_opt_set_exclusive_addr_ranges(
        sc_opt opts, const cm_vct * addr_ranges) {

    //call generic setter
    return _vector_setter<sc::opt,
                          std::pair<uintptr_t, uintptr_t>,
                          sc_addr_range>(
        opts, addr_ranges, &sc::opt::set_exclusive_addr_ranges,
            //start lambda
            [](std::pair<uintptr_t, uintptr_t> * cc_addr_range,
            sc_addr_range * c_addr_range) -> int {

                *cc_addr_range = std::pair<uintptr_t, uintptr_t>(
                    c_addr_range->min, c_addr_range->max);
                return 0;
        }); //end lambda
}


int sc_opt_get_exclusive_addr_ranges(
        const sc_opt opts, cm_vct * addr_ranges) {

    //call generic getter
    return _vector_getter<sc::opt,
                          std::pair<uintptr_t, uintptr_t>,
                          sc_addr_range>(
        opts, addr_ranges, &sc::opt::get_exclusive_addr_ranges,
            //start lambda
            [](sc_addr_range * c_addr_range,
            std::pair<uintptr_t, uintptr_t> * cc_addr_range) -> int {

                c_addr_range->min = cc_addr_range->first;
                c_addr_range->max = cc_addr_range->second;
                return 0;
        }); //end lambda
}


int sc_opt_set_access(sc_opt opts, const cm_byte access) {

    int ret;

    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //perform the set
    if (access == (cm_byte) -1) ret =  o->set_access(std::nullopt);
    else ret = o->set_access(access);
    return (ret != 0) ? -1 : 0;
}

    
cm_byte sc_opt_get_access(const sc_opt opts) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //return NULL if optional is not set or there is an error
    std::optional<cm_byte> access = o->get_access();

    if (access.has_value()) {
        return access.value();
    } else {
        sc_errno = SC_ERR_OPT_EMPTY;
        return -1;
    }
}



/*
 *  --- [OPT_PTR | EXTERNAL] ---
 */

//new class opt_ptr
sc_opt_ptr sc_new_opt_ptr() {

    try {
        return new sc::opt_ptr();

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return nullptr;
    }
}


sc_opt_ptr sc_copy_opt_ptr(const sc_opt_ptr opts_ptr) {

    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);

    try {
        return new sc::opt_ptr(*o);
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return nullptr;
    }
}


//delete class opt
int sc_del_opt_ptr(sc_opt_ptr opts_ptr) {

    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);

    try {
        delete o;
        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


//reset class opt_ptr
int sc_opt_ptr_reset(sc_opt_ptr opts_ptr) {

    int ret;

    
    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);

    try {
        ret = o->reset();
        return (ret != 0) ? -1 : 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_opt_ptr_set_target_addr(sc_opt_ptr opts_ptr,
                                   const uintptr_t target_addr) {

    int ret;


    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);

    //perform the set
    if (target_addr == 0x0) ret = o->set_target_addr(std::nullopt);
    else ret = o->set_target_addr(target_addr);
    return (ret != 0) ? -1 : 0;
}


uintptr_t sc_opt_ptr_get_target_addr(const sc_opt_ptr opts_ptr) {
    
    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);

    //return NULL if optional is not set or there is an error
    const std::optional<uintptr_t> & target_addr
        = o->get_target_addr();

    if (target_addr.has_value()) {
        return target_addr.value();
    } else {
        sc_errno = SC_ERR_OPT_EMPTY;
        return 0x0;
    }
}


int sc_opt_ptr_set_alignment(sc_opt_ptr opts_ptr,
                                 const off_t alignment) {

    int ret;


    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);

    //perform the set
    if (alignment == 0x0) ret = o->set_alignment(std::nullopt);
    else ret = o->set_alignment(alignment);
    return (ret != 0) ? -1 : 0;
}


off_t sc_opt_ptr_get_alignment(const sc_opt_ptr opts_ptr) {
    
    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);

    //return NULL if optional is not set or there is an error
    const std::optional<off_t> & alignment
        = o->get_alignment();

    if (alignment.has_value()) {
        return alignment.value();
    } else {
        sc_errno = SC_ERR_OPT_EMPTY;
        return 0x0;
    }
}


int sc_opt_ptr_set_max_obj_sz(sc_opt_ptr opts_ptr,
                                  const off_t max_obj_sz) {

    int ret;


    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);
    
    //perform the set
    if (max_obj_sz == 0x0) ret = o->set_max_obj_sz(std::nullopt);
    else ret = o->set_max_obj_sz(max_obj_sz);
    return (ret != 0) ? -1 : 0;
}


off_t sc_opt_ptr_get_max_obj_sz(const sc_opt_ptr opts_ptr) {
    
    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);

    //return NULL if optional is not set or there is an error
    const std::optional<off_t> & max_obj_sz
        = o->get_max_obj_sz();

    if (max_obj_sz.has_value()) {
        return max_obj_sz.value();
    } else {
        sc_errno = SC_ERR_OPT_EMPTY;
        return 0x0;
    }
}


int sc_opt_ptr_set_max_depth(sc_opt_ptr opts_ptr,
                                  const int max_depth) {

    int ret;


    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);
    
    //perform the set
    if (max_depth == 0x0) ret = o->set_max_depth(std::nullopt);
    else ret = o->set_max_depth(max_depth);
    return (ret != 0) ? -1 : 0;
}


int sc_opt_ptr_get_max_depth(const sc_opt_ptr opts_ptr) {
    
    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);

    //return NULL if optional is not set or there is an error
    const std::optional<int> & max_depth
        = o->get_max_depth();

    if (max_depth.has_value()) {
        return max_depth.value();
    } else {
        sc_errno = SC_ERR_OPT_EMPTY;
        return 0x0;
    }
}


int sc_opt_ptr_set_static_areas(sc_opt_ptr opts_ptr,
                                    const cm_vct * static_areas) {

    //call generic setter
    return _vector_setter<sc::opt_ptr,
                          const cm_lst_node *,
                          const cm_lst_node *>(
        opts_ptr, static_areas,
        &sc::opt_ptr::set_static_areas, std::nullopt);
}


int sc_opt_ptr_get_static_areas(const sc_opt_ptr opts_ptr,
                                    cm_vct * static_areas) {

    /*
     *  NOTE: C++ returns a std::unordered_set. It only returns an
     *        unordered set because that is the container used by the
     *        internal implementation. Since we can't simply memcpy()
     *        the container and must move it an element at a time, it
     *        makes sense to move it to a more appropriate container.
     */

    int ret;

    //call generic setter
    ret = _unordered_set_getter<sc::opt_ptr,
                                const cm_lst_node *,
                                const cm_lst_node *>(
        opts_ptr, static_areas,
        &sc::opt_ptr::get_static_areas, std::nullopt);
    if (ret != 0) return -1;

    //sort the returned vector
    try {
        c_iface::sort_area_vct(static_areas);
    } catch (const std::exception & excp) {
        cm_del_vct(static_areas);
        exception_sc_errno(excp);
        return -1;
    }

    return 0;
}


int sc_opt_ptr_set_preset_offsets(sc_opt_ptr opts_ptr,
                                      const cm_vct * preset_offsets) {

    //call generic setter
    return _vector_setter<sc::opt_ptr, off_t, off_t>(
        opts_ptr, preset_offsets,
        &sc::opt_ptr::set_preset_offsets, std::nullopt);
}


int sc_opt_ptr_get_preset_offsets(const sc_opt_ptr opts_ptr,
                                      cm_vct * preset_offsets) {

    //call generic getter
    return _vector_getter<sc::opt_ptr, off_t, off_t>(
        opts_ptr, preset_offsets,
        &sc::opt_ptr::get_preset_offsets, std::nullopt);        
}


int sc_opt_ptr_set_smart_scan(sc_opt_ptr opts_ptr,
                                  const bool enable) {

    int ret;


    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);

    //perform the set
    ret = o->set_smart_scan(enable);
    return (ret != 0) ? -1 : 0;
}


bool sc_opt_ptr_get_smart_scan(const sc_opt_ptr opts_ptr) {

    //cast opaque handle into class
    sc::opt_ptr * o = static_cast<sc::opt_ptr *>(opts_ptr);

    //call getter
    return o->get_smart_scan();
}
