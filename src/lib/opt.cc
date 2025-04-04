//standard template library
#include <unistd.h>
#include <vector>
#include <string>
#include <optional>

//C standard library
#include <cstring>

//system headers
#include <linux/limits.h>

//external libraries
#include <cmore.h>
#include <memcry.h>

//local headers
#include "scancry.h"
#include "opt.hh"
#include "error.hh"



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

/*
 *  --- [PUBLIC] ---
 */

//getters & setters
void sc::opt::set_file_path_out(const std::optional<std::string> & file_path_out) {
    this->file_path_out = file_path_out;
}


const std::optional<std::string> & sc::opt::get_file_path_out() const {
    return this->file_path_out;
}


void sc::opt::set_file_path_in(const std::optional<std::string> & file_path_in) {
    this->file_path_in = file_path_in;
}


const std::optional<std::string> & sc::opt::get_file_path_in() const {
    return this->file_path_in;
}


void sc::opt::set_sessions(const std::vector<mc_session const *> & sessions) {
    this->sessions = sessions;
}


const std::vector<mc_session const *> & sc::opt::get_sessions() const {
    return this->sessions;
}


void sc::opt::set_map(const mc_vm_map * map) noexcept {
    this->map = map;
}


mc_vm_map const * sc::opt::get_map() const noexcept {
    return this->map;
}


void sc::opt::set_omit_areas(const std::optional<std::vector<cm_lst_node *>> & omit_areas) {
    this->omit_areas = omit_areas;
}


const std::optional<std::vector<cm_lst_node *>> & sc::opt::get_omit_areas() const {
    return this->omit_areas;
}


void sc::opt::set_omit_objs(const std::optional<std::vector<cm_lst_node *>> & omit_objs) {
    this->omit_objs = omit_objs;
}


const std::optional<std::vector<cm_lst_node *>> & sc::opt::get_omit_objs() const {
    return this->omit_objs;
}


void sc::opt::set_exclusive_areas(const std::optional<std::vector<cm_lst_node *>> & exclusive_areas) {
    this->exclusive_areas = exclusive_areas;
}


const std::optional<std::vector<cm_lst_node *>> & sc::opt::get_exclusive_areas() const {
    return this->exclusive_areas;
}


void sc::opt::set_exclusive_objs(const std::optional<std::vector<cm_lst_node *>> & exclusive_objs) {
    sc::opt::exclusive_objs = exclusive_objs;
}


const std::optional<std::vector<cm_lst_node *>> & sc::opt::get_exclusive_objs() const {
    return this->exclusive_objs;
}


void sc::opt::set_addr_range(const std::optional<std::pair<uintptr_t, uintptr_t>> & addr_range) {
    this->addr_range = addr_range;
}


const std::optional<std::pair<uintptr_t, uintptr_t>> sc::opt::get_addr_range() const {
    return this->addr_range;
}


void sc::opt::set_access(const std::optional<cm_byte> & access) noexcept {
    this->access = access;
}


std::optional<cm_byte> sc::opt::get_access() const noexcept {
    return this->access;
}



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

/*
 *  --- [INTERNAL] ---
 */

//generic setter of constraints
_SC_DBG_STATIC
int _opt_c_constraint_setter(sc_opt opts, const cm_vct * v,
                             void (sc::opt::*set)(const std::optional<std::vector<cm_lst_node *>> &)) {
    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //call the setter with the STL vector
    try {
        if (v == nullptr) {
            (o->*set)(std::nullopt);
            
        } else {
            //create a STL vector
            std::vector<cm_lst_node *> rett;
            rett.resize(v->len);
            std::memcpy(rett.data(), v->data, v->data_sz * v->len);

            //perform the set
            (o->*set)(rett);
        }
        return 0;

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


//generic getter of constraints
_SC_DBG_STATIC
int _opt_c_constraint_getter(const sc_opt opts, cm_vct * v,
                             const std::optional<std::vector<cm_lst_node *>> & (sc::opt::*get)() const) {
    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //initialise the CMore vector
    int ret = cm_new_vct(v, sizeof(cm_lst_node *));
    if (ret == -1) {
        sc_errno = SC_ERR_CMORE;
        return -1;
    }

    //copy contents of the STL vector into the CMore vector.
    try {
        //get the STL vector
        const std::optional<std::vector<cm_lst_node *>> & rett = (o->*get)();

        //if a STL vector is present, do the copy
        if (rett.has_value() && !rett.value().empty()) {
            ret = cm_vct_rsz(v, rett.value().size());
            std::memcpy(v->data, rett.value().data(),
                        rett.value().size() * sizeof(cm_lst_node *));
            return 0;

        } else {
            cm_del_vct(v);
            sc_errno = SC_ERR_OPT_EMPTY;
            return -1;
        }
        
    } catch (const std::exception & excp) {
        cm_del_vct(v);
        exception_sc_errno(excp);
        return -1;
    }
}



/*
 *  --- [EXTERNAL] ---
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


//set class opt->file_path_out
int sc_opt_set_file_path_out(sc_opt opts, const char * path) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    try {
        if (path == nullptr) o->set_file_path_out(std::nullopt);
        else o->set_file_path_out(path);
        return 0;
        
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

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);
    
    try {
        if (path == nullptr) o->set_file_path_in(std::nullopt);
        else o->set_file_path_in(path);
        return 0;
        
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
    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //create a STL vector
    std::vector<const mc_session *> v;
    v.resize(sessions->len);
    std::memcpy(v.data(), sessions->data,
                sessions->data_sz * sessions->len);

    //call the setter with the STL vector
    try {
        o->set_sessions(v);
        return 0;

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
    if (ret == -1) {
        sc_errno = SC_ERR_CMORE;
        return -1;
    }

    //copy contents of the STL vector into the CMore vector.
    try {
        //get the STL vector
        const std::vector<const mc_session *> & v = o->get_sessions();

        //copy the STL vector into the CMore vector
        ret = cm_vct_rsz(sessions, v.size());
        std::memcpy(sessions->data, v.data(),
                    v.size() * sizeof(cm_lst_node *));
        return 0;
        
    } catch (const std::exception & excp) {
        cm_del_vct(sessions);
        exception_sc_errno(excp);
        return -1;
    }
}


void sc_opt_set_map(sc_opt opts, const mc_vm_map * map) {
    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    o->set_map(map);
    return;
}


mc_vm_map const * sc_opt_get_map(const sc_opt opts) {

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
    return _opt_c_constraint_setter(opts, omit_areas,
                                    &sc::opt::set_omit_areas);
}


int sc_opt_get_omit_areas(const sc_opt opts, cm_vct * omit_areas) {

    //call generic getter
    return _opt_c_constraint_getter(opts, omit_areas,
                                    &sc::opt::get_omit_areas);
}


int sc_opt_set_omit_objs(sc_opt opts, const cm_vct * omit_objs) {

    //call generic setter
    return _opt_c_constraint_setter(opts, omit_objs,
                                    &sc::opt::set_omit_objs);
}


int sc_opt_get_omit_objs(const sc_opt opts, cm_vct * omit_objs) {

    //call generic getter
    return _opt_c_constraint_getter(opts, omit_objs,
                                    &sc::opt::get_omit_objs);
}


int sc_opt_set_exclusive_areas(sc_opt opts, const cm_vct * exclusive_areas) {

    //call generic setter
    return _opt_c_constraint_setter(opts, exclusive_areas,
                                    &sc::opt::set_exclusive_areas);
}


int sc_opt_get_exclusive_areas(const sc_opt opts, cm_vct * exclusive_areas) {
    
    //call generic getter
    return _opt_c_constraint_getter(opts, exclusive_areas,
                                    &sc::opt::get_exclusive_areas);
}


int sc_opt_set_exclusive_objs(sc_opt opts, const cm_vct * exclusive_objs) {

    //call generic setter
    return _opt_c_constraint_setter(opts, exclusive_objs,
                                    &sc::opt::set_exclusive_objs);
}


int sc_opt_get_exclusive_objs(const sc_opt opts, cm_vct * exclusive_objs) {

    //call generic getter
    return _opt_c_constraint_getter(opts, exclusive_objs,
                                    &sc::opt::get_exclusive_objs);
}


int sc_opt_set_addr_range(sc_opt opts, const sc_addr_range * range) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    try {
        if (range == nullptr) o->set_addr_range(std::nullopt);
        else o->set_addr_range(std::pair(range->min, range->max));
        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_opt_get_addr_range(const sc_opt opts, sc_addr_range * range) {

    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //copy the address range into a sc_addr_range
    try {
        //get the address range pair
        const std::optional<std::pair<uintptr_t, uintptr_t>> & ar
            = o->get_addr_range();

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
    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    try {
        if (access == (cm_byte) -1) o->set_access(std::nullopt);
        else o->set_access(access);
        return 0;
        
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



// TODO: Use the following for setting static areas:
    /*
     *  NOTE: Provided static areas that are not included in the map_area_set
     *        are ignored.
     */

    //fetch areas selected for scanning
    const std::unordered_set<cm_lst_node *> & scan_area_set = ma_set.get_area_nodes();
    std::unordered_set<cm_lst_node *> ret_static_set;


    //for every proposed static area
    for (auto iter = static_areas.begin(); iter != static_areas.end(); ++iter) {

        //if static area not included in current scan area set, ignore it
        if (scan_area_set.find(*iter) == scan_area_set.end()) continue;

        //add static area to the static areas set
        ret_static_set.insert(*iter);
    }

    return ret_static_set;
