//standard template library
#include <optional>
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
#include "scancry_impl.h"



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
   addr_width(opts.addr_width),
   file_path_out(opts.file_path_out),
   file_path_in(opts.file_path_in),
   sessions(opts.sessions),
   map(opts.map),
   omit_areas(opts.omit_areas),
   omit_objs(opts.omit_objs),
   exclusive_areas(opts.exclusive_areas),
   exclusive_objs(opts.exclusive_objs),
   addr_ranges(opts.addr_ranges),
   access(opts.access) {}


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
    this->addr_ranges = std::nullopt;
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


[[nodiscard]] int sc::opt::set_map(
    const mc_vm_map * map) noexcept {

    _LOCK(-1)
    this->map = map;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] mc_vm_map const *
    sc::opt::get_map() const noexcept {

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


[[nodiscard]] int sc::opt::set_addr_ranges(const std::optional<
    std::vector<std::pair<uintptr_t, uintptr_t>>> & addr_range) {

    _LOCK(-1)
    this->addr_ranges = addr_range;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<
    std::vector<std::pair<uintptr_t, uintptr_t>>>
        sc::opt::get_addr_ranges() const {

    return this->addr_ranges;
}


[[nodiscard]] int sc::opt::set_access(
    const std::optional<cm_byte> & access) noexcept {

    _LOCK(-1)
    this->access = access;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] std::optional<cm_byte> sc::opt::get_access() const noexcept {

    return this->access;
}



/*
 *  --- [OPT_PTRSCAN | PUBLIC] ---
 */

sc::opt_ptrscan::opt_ptrscan()
 : _opt_scan(),
   smart_scan(true) {}


sc::opt_ptrscan::opt_ptrscan(const opt_ptrscan & opts_ptrscan)
 : _opt_scan(),
   target_addr(opts_ptrscan.target_addr),
   alignment(opts_ptrscan.alignment),
   max_obj_sz(opts_ptrscan.max_obj_sz),
   max_depth(opts_ptrscan.max_depth),
   static_areas(opts_ptrscan.static_areas),
   preset_offsets(opts_ptrscan.preset_offsets),
   smart_scan(opts_ptrscan.smart_scan) {}


[[nodiscard]] int sc::opt_ptrscan::reset() {

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
[[nodiscard]] int sc::opt_ptrscan::set_target_addr(
    const std::optional<uintptr_t> & target_addr) {

    _LOCK(-1)
    this->target_addr = target_addr;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] std::optional<uintptr_t>
    sc::opt_ptrscan::get_target_addr() const noexcept {

    return this->target_addr;
}


[[nodiscard]] int sc::opt_ptrscan::set_alignment(
    const std::optional<off_t> & alignment) {

    _LOCK(-1)
    this->alignment = alignment;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] std::optional<off_t>
    sc::opt_ptrscan::get_alignment() const noexcept {

    return this->alignment;
}


[[nodiscard]] int sc::opt_ptrscan::set_max_obj_sz(
    const std::optional<off_t> & max_obj_sz) {

    _LOCK(-1)
    this->max_obj_sz = max_obj_sz;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] std::optional<off_t>
    sc::opt_ptrscan::get_max_obj_sz() const noexcept {

    return this->max_obj_sz;
}


[[nodiscard]] int sc::opt_ptrscan::set_max_depth(
    const std::optional<off_t> & max_depth) {

    _LOCK(-1)
    this->max_depth = max_depth;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] std::optional<off_t>
    sc::opt_ptrscan::get_max_depth() const noexcept {

    return this->max_depth;
}


[[nodiscard]] int sc::opt_ptrscan::set_static_areas(
    const std::optional<std::vector<const cm_lst_node *>> & static_areas) {

    _LOCK(-1)
    if (this->static_areas.has_value()) {
        this->static_areas = std::unordered_set<const cm_lst_node *>(
            static_areas->begin(), static_areas->end());
    } else {
        this->static_areas = std::nullopt;
    }
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<std::unordered_set<const cm_lst_node *>>
    & sc::opt_ptrscan::get_static_areas() const {

    return this->static_areas;
}


[[nodiscard]] int sc::opt_ptrscan::set_preset_offsets(
    const std::optional<std::vector<off_t>> & preset_offsets) {

    _LOCK(-1)
    this->preset_offsets = preset_offsets;
    _UNLOCK(-1)

    return 0;
}


[[nodiscard]] const std::optional<std::vector<off_t>>
    & sc::opt_ptrscan::get_preset_offsets() const {

    return this->preset_offsets;
}



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

/*
 *  --- [INTERNAL] ---
 */

//generic setter of vectors
template <typename O, typename T>
_SC_DBG_STATIC int _vector_setter(void * opts_X, const cm_vct * cmore_vct,
                                  int (O::*set)(
                                      const std::optional<std::vector<T>> &)) {

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
            ret = c_iface::from_cmore_vct<T>(cmore_vct, stl_vct);
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


//generic getter of constraints
template <typename O, typename T>
_SC_DBG_STATIC int _vector_getter(void * opts_X, cm_vct * cmore_vct,
                                  const std::optional<std::vector<T>> &
                                      (O::*get)() const) {

    int ret;

    
    //cast opaque handle into class
    O * o = static_cast<O *>(opts_X);

    //copy contents of the STL vector into the CMore vector.
    try {
        //get the STL vector
        const std::optional<std::vector<T>> & stl_vct = (o->*get)();

        //if this constraint doesn't have a value, fail
        if (stl_vct.has_value() == false) return -1;

        //perform the copy
        ret = c_iface::to_cmore_vct<T>(cmore_vct, stl_vct.value());
        if (ret != 0) return -1;

        return 0;
        
    } catch (const std::exception & excp) {
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


const mc_vm_map * sc_opt_get_map(const sc_opt opts) {

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
 * In these setter functions, the C++ setter is passed a STL vector
 * constructed from a CMore vector
 */

int sc_opt_set_omit_areas(sc_opt opts, const cm_vct * omit_areas) {

    //call generic setter
    return _vector_setter<sc::opt, const cm_lst_node *>(
        opts, omit_areas, &sc::opt::set_omit_areas);
}


int sc_opt_get_omit_areas(const sc_opt opts, cm_vct * omit_areas) {

    //call generic getter
    return _vector_getter<sc::opt, const cm_lst_node *>(
        opts, omit_areas, &sc::opt::get_omit_areas);
}


int sc_opt_set_omit_objs(sc_opt opts, const cm_vct * omit_objs) {

    //call generic setter
    return _vector_setter<sc::opt, const cm_lst_node *>(
        opts, omit_objs, &sc::opt::set_omit_objs);
}


int sc_opt_get_omit_objs(const sc_opt opts, cm_vct * omit_objs) {

    //call generic getter
    return _vector_getter<sc::opt, const cm_lst_node *>(
        opts, omit_objs, &sc::opt::get_omit_objs);
}


int sc_opt_set_exclusive_areas(sc_opt opts, const cm_vct * exclusive_areas) {

    //call generic setter
    return _vector_setter<sc::opt, const cm_lst_node *>(
        opts, exclusive_areas, &sc::opt::set_exclusive_areas);
}


int sc_opt_get_exclusive_areas(const sc_opt opts, cm_vct * exclusive_areas) {
    
    //call generic getter
    return _vector_getter<sc::opt, const cm_lst_node *>(
        opts, exclusive_areas, &sc::opt::get_exclusive_areas);
}


int sc_opt_set_exclusive_objs(sc_opt opts, const cm_vct * exclusive_objs) {

    //call generic setter
    return _vector_setter<sc::opt, const cm_lst_node *>(
        opts, exclusive_objs, &sc::opt::set_exclusive_objs);
}


int sc_opt_get_exclusive_objs(const sc_opt opts, cm_vct * exclusive_objs) {

    //call generic getter
    return _vector_getter<sc::opt, const cm_lst_node *>(
        opts, exclusive_objs, &sc::opt::get_exclusive_objs);
}


int sc_opt_set_addr_ranges(sc_opt opts, const cm_vct * ranges) {

    int ret;
    sc_addr_range range;


    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    try {
        //convert to a STL vector of pairs
        std::vector<std::pair<uintptr_t, uintptr_t>> stl_vct;
        for (int i = 0; i < ranges->len; ++i) {

            ret = cm_vct_get(ranges, i, &range);
            if (ret != 0) {
                sc_errno = SC_ERR_CMORE;
                return -1;
            }

            stl_vct.push_back(std::pair(range.min, range.max));
        }

        //set pairs
        ret = o->set_addr_ranges(stl_vct);
        if (ret != 0) return -1;

        return 0;

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_opt_get_addr_ranges(const sc_opt opts, cm_vct * addr_ranges) {

    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //initialise the CMore vector
    int ret = cm_new_vct(addr_ranges, sizeof(sc_addr_range));
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        return -1;
    }

    //copy the address range into a sc_addr_range
    try {
        //get the address range pair
        const std::optional<std::vector<std::pair<uintptr_t, uintptr_t>>> & ars
            = o->get_addr_ranges();

        //if a STL vector is present, do the copy
        if (ars.has_value() && !ars.value().empty()) {
            ret = cm_vct_rsz(addr_ranges, ars.value().size());
            std::memcpy(addr_ranges->data, ars.value().data(),
                        rett.value().size() * sizeof(cm_lst_node *));
            return 0;

        } else {
            cm_del_vct(v);
            sc_errno = SC_ERR_OPT_EMPTY;
            return -1;
        }




        //if it is set, convert it to a sc_addr_range
        if (ar.has_value()) {
            range->min = ar.value().first;
            range->max = ar.value().second;
            return 0;
            
        } else {
            return -1;
        }

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_opt_set_access(sc_opt opts, const cm_byte access) {

    int ret;

    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    try {
        if (access == (cm_byte) -1) ret =  o->set_access(std::nullopt);
        else ret = o->set_access(access);
        return (ret != 0) ? -1 : 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}

    
cm_byte sc_opt_get_access(const sc_opt opts) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //return NULL if optional is not set or there is an error
    try {
        std::optional<cm_byte> access = o->get_access();
    
        if (access.has_value()) {
            return access.value();
        } else {
            sc_errno = SC_ERR_OPT_EMPTY;
            return -1;
        }
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}



/*
 *  --- [OPT_PTRSCAN | EXTERNAL] ---
 */

//new class opt_ptrscan
sc_opt_ptrscan sc_new_opt_ptrscan() {

    try {
        return new sc::opt_ptrscan();

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return nullptr;
    }
};


//delete class opt
int sc_del_opt_ptrscan(sc_opt_ptrscan opts_ptrscan) {

    //cast opaque handle into class
    sc::opt_ptrscan * o = static_cast<sc::opt_ptrscan *>(opts_ptrscan);

    try {
        delete o;
        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


//reset class opt_ptrscan
int sc_opt_ptrscan_reset(sc_opt_ptrscan opts) {

    int ret;

    
    //cast opaque handle into class
    sc::opt_ptrscan * o = static_cast<sc::opt_ptrscan *>(opts);

    try {
        ret = o->reset();
        return (ret != 0) ? -1 : 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_opt_ptrscan_set_target_addr(sc_opt_ptrscan opts_ptrscan,
                                   const uintptr_t target_addr) {

    int ret;


    //cast opaque handle into class
    sc::opt_ptrscan * o = static_cast<sc::opt_ptrscan *>(opts_ptrscan);
    
    try {
        if (target_addr == 0x0) ret = o->set_target_addr(std::nullopt);
        else ret = o->set_target_addr(target_addr);
        return (ret != 0) ? -1 : 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


uintptr_t sc_opt_ptrscan_get_target_addr(const sc_opt_ptrscan opts_ptrscan) {
    
    //cast opaque handle into class
    sc::opt_ptrscan * o = static_cast<sc::opt_ptrscan *>(opts_ptrscan);

    //return NULL if optional is not set or there is an error
    try {
        const std::optional<uintptr_t> & target_addr
            = o->get_target_addr();

        if (target_addr.has_value()) {
            return target_addr.value();
        } else {
            sc_errno = SC_ERR_OPT_EMPTY;
            return 0;
        }
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return 0;
    }
}


int sc_opt_ptrscan_set_alignment(sc_opt_ptrscan opts_ptrscan,
                                 const off_t alignment) {

    int ret;


    //cast opaque handle into class
    sc::opt_ptrscan * o = static_cast<sc::opt_ptrscan *>(opts_ptrscan);
    
    try {
        if (alignment == 0x0) ret = o->set_alignment(std::nullopt);
        else ret = o->set_alignment(alignment);
        return (ret != 0) ? -1 : 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


uintptr_t sc_opt_ptrscan_get_alignment(const sc_opt_ptrscan opts_ptrscan) {
    
    //cast opaque handle into class
    sc::opt_ptrscan * o = static_cast<sc::opt_ptrscan *>(opts_ptrscan);

    //return NULL if optional is not set or there is an error
    try {
        const std::optional<off_t> & alignment
            = o->get_alignment();

        if (alignment.has_value()) {
            return alignment.value();
        } else {
            sc_errno = SC_ERR_OPT_EMPTY;
            return -1;
        }
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return 0;
    }
}


int sc_opt_ptrscan_set_max_obj_sz(sc_opt_ptrscan opts_ptrscan,
                                  const off_t max_obj_sz) {

    int ret;


    //cast opaque handle into class
    sc::opt_ptrscan * o = static_cast<sc::opt_ptrscan *>(opts_ptrscan);
    
    try {
        if (max_obj_sz == 0x0) ret = o->set_max_obj_sz(std::nullopt);
        else ret = o->set_max_obj_sz(max_obj_sz);
        return (ret != 0) ? -1 : 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


uintptr_t sc_opt_ptrscan_get_max_obj_sz(const sc_opt_ptrscan opts_ptrscan) {
    
    //cast opaque handle into class
    sc::opt_ptrscan * o = static_cast<sc::opt_ptrscan *>(opts_ptrscan);

    //return NULL if optional is not set or there is an error
    try {
        const std::optional<off_t> & max_obj_sz
            = o->get_max_obj_sz();

        if (max_obj_sz.has_value()) {
            return max_obj_sz.value();
        } else {
            sc_errno = SC_ERR_OPT_EMPTY;
            return -1;
        }
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return 0;
    }
}


int sc_opt_ptrscan_set_static_areas(sc_opt_ptrscan opts_ptrscan,
                                    const cm_vct * static_areas) {

    //call generic setter
    return _opt_ptrscan_c_vector_setter<const cm_lst_node *>(
        opts_ptrscan, static_areas, &sc::opt_ptrscan::set_static_areas);
}


int sc_opt_ptrscan_get_static_areas(const sc_opt_ptrscan opts_ptrscan,
                                    cm_vct * static_areas) {

    /*
     *  NOTE: C++ returns a std::unordered_set. It only returns an
     *        unordered set because that is the container used by the
     *        internal implementation. Since we can't simply memcpy()
     *        the container and must move it an element at a time, it
     *        makes sense to move it to a more appropriate container.
     */

    int ret;


    //cast opaque handle into class
    sc::opt_ptrscan * o = static_cast<sc::opt_ptrscan *>(opts_ptrscan);

    //initialise the CMore vector
    ret = cm_new_vct(static_areas, sizeof(cm_lst_node *));
    if (ret == -1) {
        sc_errno = SC_ERR_CMORE;
        return -1;
    }

    try {
        //fetch reference to C++ hashmap
        std::optional<std::unordered_set<const cm_lst_node *>> static_areas_set
            = o->get_static_areas();

        //return error if no static areas are present
        if (static_areas_set.has_value() == false) {
            sc_errno = SC_ERR_OPT_EMPTY;
            return -1;
        }

        //for every static area
        for (auto iter = static_areas_set.value().begin();
             iter != static_areas_set.value().end(); ++iter) {

            //append the area to the vector
            ret = cm_vct_apd(static_areas, *iter);
            if (ret != 0) {
                sc_errno = SC_ERR_CMORE;
                cm_del_vct(static_areas);
                return -1;
            }
        } //end for every static area

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        cm_del_vct(static_areas);
        return -1;
    }

    return 0;    
}


int sc_opt_ptrscan_set_preset_offsets(sc_opt_ptrscan opts_ptrscan,
                                      const cm_vct * preset_offsets) {

    //call generic setter
    return _opt_ptrscan_c_vector_setter<off_t>(
        opts_ptrscan, preset_offsets, &sc::opt_ptrscan::set_preset_offsets);
}


int sc_opt_ptrscan_get_preset_offsets(const sc_opt_ptrscan opts_ptrscan,
                                      cm_vct * preset_offsets) {

    //call generic getter
    return _opt_ptrscan_c_vector_getter<off_t>(
        opts_ptrscan, preset_offsets, &sc::opt_ptrscan::get_preset_offsets);        
}

//set class opt->file_path_out
int sc_opt_ptrscan_set_smart_scan(sc_opt_ptrscan opts_ptrscan, bool enable) {

    int ret;


    //cast opaque handle into class
    sc::opt_ptrscan * o = static_cast<sc::opt_ptrscan *>(opts_ptrscan);

    try {
        ret = o->set_smart_scan(enable);
        return (ret != 0) ? -1 : 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


bool sc_opt_ptrscan_get_smart_scan(const sc_opt_ptrscan opts_ptrscan) {

    //cast opaque handle into class
    sc::opt_ptrscan * o = static_cast<sc::opt_ptrscan *>(opts_ptrscan);

    //call getter
    return o->get_smart_scan();
}
