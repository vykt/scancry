//standard template library
#include <optional>
#include <sys/types.h>
#include <vector>
#include <string>
#include <functional>

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
#include "error.hh"



/*
 *  --- [_OPT_SCAN | INTERNAL] ---
 */

/*
 *  NOTE: We must provide an explicit body for the abstract destructor
 *        of `_opt_scan` so the linker can produce a valid chain of
 *        destructors to call.
 */
sc::_opt_scan::~_opt_scan() {}



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

/*
 *  --- [OPT | PUBLIC] ---
 */

sc::opt::opt(enum addr_width _addr_width)
 : _lockable(),
   map(nullptr),
   addr_width(_addr_width) {}


sc::opt::opt(const opt & opts)
 : _lockable(),
   file_path_out(opts.file_path_out),
   file_path_in(opts.file_path_in),
   sessions(opts.sessions),
   map(opts.map),
   omit_areas(opts.omit_areas),
   omit_objs(opts.omit_objs),
   exclusive_areas(opts.exclusive_areas),
   exclusive_objs(opts.exclusive_objs),
   omit_addr_ranges(opts.omit_addr_ranges),
   exclusive_addr_ranges(opts.exclusive_addr_ranges),
   access(opts.access),
   addr_width(opts.addr_width) {}


sc::opt::opt(const opt && opts)
 : _lockable(),
   file_path_out(opts.file_path_out),
   file_path_in(opts.file_path_in),
   sessions(opts.sessions),
   map(opts.map),
   omit_areas(opts.omit_areas),
   omit_objs(opts.omit_objs),
   exclusive_areas(opts.exclusive_areas),
   exclusive_objs(opts.exclusive_objs),
   omit_addr_ranges(opts.omit_addr_ranges),
   exclusive_addr_ranges(opts.exclusive_addr_ranges),
   access(opts.access),
   addr_width(opts.addr_width) {}


[[nodiscard]] int sc::opt::reset() {
    _LOCK(-1)
    this->file_path_in = std::nullopt;
    this->file_path_out = std::nullopt;
    this->sessions.clear();
    this->map = nullptr;
    this->omit_areas = std::nullopt;
    this->omit_objs = std::nullopt;
    this->exclusive_areas = std::nullopt;
    this->exclusive_objs = std::nullopt;
    this->omit_addr_ranges = std::nullopt;
    this->exclusive_addr_ranges = std::nullopt;
    this->access = std::nullopt;
    _UNLOCK(-1)

    return 0;
}


//getters & setters
[[nodiscard]] int sc::opt::set_file_path_out(
    const std::optional<std::string> & file_path_out) {

    _LOCK(-1)
    this->file_path_out = file_path_out;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<std::string> &
    sc::opt::get_file_path_out() const {

    return this->file_path_out;
}


[[nodiscard]] int sc::opt::set_file_path_in(
    const std::optional<std::string> & file_path_in) {

    _LOCK(-1)
    this->file_path_in = file_path_in;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<std::string> &
    sc::opt::get_file_path_in() const {

    return this->file_path_in;
}


[[nodiscard]] int sc::opt::set_sessions(
    const std::vector<mc_session const *> & sessions) {

    _LOCK(-1)
    this->sessions = sessions;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::vector<mc_session const *> &
    sc::opt::get_sessions() const {

    return this->sessions;
}


[[nodiscard]] int sc::opt::set_map(const mc_vm_map * map) noexcept {

    _LOCK(-1)
    this->map = (mc_vm_map *) map;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] mc_vm_map * sc::opt::get_map() const noexcept {

    return this->map;
}


[[nodiscard]] int sc::opt::set_omit_areas(
    const std::optional<std::vector<const cm_lst_node *>> & omit_areas) {

    _LOCK(-1)
    this->omit_areas = omit_areas;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<std::vector<const cm_lst_node *>>
    & sc::opt::get_omit_areas() const {

    return this->omit_areas;
}


[[nodiscard]] int sc::opt::set_omit_objs(
    const std::optional<std::vector<const cm_lst_node *>> & omit_objs) {

    _LOCK(-1)
    this->omit_objs = omit_objs;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<std::vector<const cm_lst_node *>>
        & sc::opt::get_omit_objs() const {

    return this->omit_objs;
}


[[nodiscard]] int sc::opt::set_exclusive_areas(
    const std::optional<std::vector<const cm_lst_node *>> & exclusive_areas) {

    _LOCK(-1)
    this->exclusive_areas = exclusive_areas;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<std::vector<const cm_lst_node *>>
    & sc::opt::get_exclusive_areas() const {

    return this->exclusive_areas;
}


[[nodiscard]] int sc::opt::set_exclusive_objs(
    const std::optional<std::vector<const cm_lst_node *>> & exclusive_objs) {

    _LOCK(-1)
    sc::opt::exclusive_objs = exclusive_objs;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<std::vector<const cm_lst_node *>>
    & sc::opt::get_exclusive_objs() const {

    return this->exclusive_objs;
}


[[nodiscard]] int sc::opt::set_omit_addr_ranges(const std::optional<
    std::vector<std::pair<uintptr_t, uintptr_t>>> & addr_range) {

    _LOCK(-1)
    this->omit_addr_ranges = addr_range;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<
    std::vector<std::pair<uintptr_t, uintptr_t>>> &
        sc::opt::get_omit_addr_ranges() const {

    return this->omit_addr_ranges;
}


[[nodiscard]] int sc::opt::set_exclusive_addr_ranges(const std::optional<
    std::vector<std::pair<uintptr_t, uintptr_t>>> & addr_range) {

    _LOCK(-1)
    this->exclusive_addr_ranges = addr_range;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<
    std::vector<std::pair<uintptr_t, uintptr_t>>> &
        sc::opt::get_exclusive_addr_ranges() const {

    return this->exclusive_addr_ranges;
}


[[nodiscard]] int sc::opt::set_access(
    const std::optional<cm_byte> access) noexcept {

    _LOCK(-1)
    this->access = access;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] std::optional<cm_byte>
    sc::opt::get_access() const noexcept {

    return this->access;
}



/*
 *  --- [OPT_PTR | PUBLIC] ---
 */

sc::opt_ptr::opt_ptr()
 : _opt_scan(),
   smart_scan(true) {}


sc::opt_ptr::opt_ptr(const opt_ptr & opts_ptr)
 : _opt_scan(),
   target_addr(opts_ptr.target_addr),
   alignment(opts_ptr.alignment),
   max_obj_sz(opts_ptr.max_obj_sz),
   max_depth(opts_ptr.max_depth),
   static_areas(opts_ptr.static_areas),
   preset_offsets(opts_ptr.preset_offsets),
   smart_scan(opts_ptr.smart_scan) {}


sc::opt_ptr::opt_ptr(const opt_ptr && opts_ptr)
 : _opt_scan(),
   target_addr(opts_ptr.target_addr),
   alignment(opts_ptr.alignment),
   max_obj_sz(opts_ptr.max_obj_sz),
   max_depth(opts_ptr.max_depth),
   static_areas(opts_ptr.static_areas),
   preset_offsets(opts_ptr.preset_offsets),
   smart_scan(opts_ptr.smart_scan) {}


[[nodiscard]] int sc::opt_ptr::reset() {

    _LOCK(-1)
    this->target_addr = std::nullopt;
    this->alignment = std::nullopt;
    this->max_obj_sz = std::nullopt;
    this->max_depth = std::nullopt;
    this->static_areas = std::nullopt;
    this->preset_offsets = std::nullopt;
    this->smart_scan = true;
    _UNLOCK(-1)

    return 0;
}


//getters & setters
[[nodiscard]] int sc::opt_ptr::set_target_addr(
    const std::optional<uintptr_t> target_addr) noexcept {

    _LOCK(-1)
    this->target_addr = target_addr;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] std::optional<uintptr_t>
    sc::opt_ptr::get_target_addr() const noexcept {

    return this->target_addr;
}


[[nodiscard]] int sc::opt_ptr::set_alignment(
    const std::optional<off_t> alignment) noexcept {

    _LOCK(-1)
    this->alignment = alignment;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] std::optional<off_t>
    sc::opt_ptr::get_alignment() const noexcept {

    return this->alignment;
}


[[nodiscard]] int sc::opt_ptr::set_max_obj_sz(
    const std::optional<off_t> max_obj_sz) noexcept {

    _LOCK(-1)
    this->max_obj_sz = max_obj_sz;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] std::optional<off_t>
    sc::opt_ptr::get_max_obj_sz() const noexcept {

    return this->max_obj_sz;
}


[[nodiscard]] int sc::opt_ptr::set_max_depth(
    const std::optional<int> max_depth) noexcept {

    _LOCK(-1)
    this->max_depth = max_depth;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] std::optional<int>
    sc::opt_ptr::get_max_depth() const noexcept {

    return this->max_depth;
}


[[nodiscard]] int sc::opt_ptr::set_static_areas(
    const std::optional<std::vector<const cm_lst_node *>> & static_areas) {

    _LOCK(-1)
    if (static_areas.has_value()) {
        this->static_areas = std::unordered_set<const cm_lst_node *>(
            static_areas->begin(), static_areas->end());
    } else {
        this->static_areas = std::nullopt;
    }
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<std::unordered_set<const cm_lst_node *>>
    & sc::opt_ptr::get_static_areas() const {

    return this->static_areas;
}


[[nodiscard]] int sc::opt_ptr::set_preset_offsets(
    const std::optional<std::vector<off_t>> & preset_offsets) {

    _LOCK(-1)
    this->preset_offsets = preset_offsets;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<std::vector<off_t>>
    & sc::opt_ptr::get_preset_offsets() const {

    return this->preset_offsets;
}


[[nodiscard]] int
    sc::opt_ptr::set_smart_scan(const bool enable) noexcept {

    _LOCK(-1)
    this->smart_scan = enable;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] bool
    sc::opt_ptr::get_smart_scan() const noexcept {

    return this->smart_scan;        
}



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
};


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
