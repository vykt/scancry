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
void sc::opt::set_file_path_out(std::string file_path_out) {
    this->file_path_out = file_path_out;
}


const std::optional<std::string> & sc::opt::get_file_path_out() const {
    return this->file_path_out;
}


void sc::opt::set_file_path_in(std::string file_path_in) {
    this->file_path_in = file_path_in;
}


const std::optional<std::string> & sc::opt::get_file_path_in() const {
    return this->file_path_in;
}


void sc::opt::set_sessions(std::vector<mc_session const *> sessions) {
    this->sessions = sessions;
}


const std::vector<mc_session const *> & sc::opt::get_sessions() const {
    return this->sessions;
}


void sc::opt::set_map(mc_vm_map * map) {
    this->map = map;
}


mc_vm_map const * sc::opt::get_map() const {
    return this->map;
}


void sc::opt::set_alignment(int alignment) {
    this->alignment = alignment;
}


std::optional<unsigned int> sc::opt::get_alignment() const {
    return this->alignment;
}


unsigned int sc::opt::get_arch_byte_width() const {
    return this->arch_byte_width;
}

void sc::opt::set_omit_areas(std::vector<cm_lst_node *> & omit_areas) {
    this->omit_areas = omit_areas;
}


const std::optional<std::vector<cm_lst_node *>> &
                               sc::opt::get_omit_areas() const {
    return this->omit_areas;
}


void sc::opt::set_omit_objs(std::vector<cm_lst_node *> & omit_objs) {
    this->omit_objs = omit_objs;
}


const std::optional<std::vector<cm_lst_node *>> &
                   sc::opt::get_omit_objs() const {
    return this->omit_objs;
}


void sc::opt::set_exclusive_areas(std::vector<cm_lst_node *> &
                                                  exclusive_areas) {
    this->exclusive_areas = exclusive_areas;
}


const std::optional<std::vector<cm_lst_node *>> &
                    sc::opt::get_exclusive_areas() const {
    return this->exclusive_areas;
}


void sc::opt::set_exclusive_objs(std::vector<cm_lst_node *> & exclusive_objs) {
    sc::opt::exclusive_objs = exclusive_objs;
}


const std::optional<std::vector<cm_lst_node *>> &
                   sc::opt::get_exclusive_objs() const {
    return this->exclusive_objs;
}


void sc::opt::set_addr_range(std::pair<uintptr_t, uintptr_t> addr_range) {
    this->addr_range = addr_range;
}


const std::optional<std::pair<uintptr_t, uintptr_t>>
                             sc::opt::get_addr_range() const {
    return this->addr_range;
}



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

/*
 *  --- [INTERNAL] ---
 */

//generic setter of constraints
_SC_DBG_STATIC
int _opt_c_constraint_setter(sc_opt opts, cm_vct * v,
                             void (sc::opt::*set)(std::vector<cm_lst_node *> &)) {
    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //create a STL vector
    std::vector<cm_lst_node *> rett;
    rett.resize(v->len);
    std::memcpy(rett.data(), v->data, v->data_sz * v->len);

    //call the setter with the STL vector
    try {
        (o->*set)(rett);
        return 0;

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


//generic getter of constraints
_SC_DBG_STATIC
int _opt_c_constraint_getter(sc_opt opts, cm_vct * v,
                             const std::optional<std::vector<cm_lst_node *>> &
                             (sc::opt::*get)() const) {
    
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
sc_opt sc_new_opt(const int arch_byte_width) {

    try {
        return new sc::opt(arch_byte_width);    

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
        o->set_file_path_out(path);
        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


const char * sc_opt_get_file_path_out(sc_opt opts) {

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
        o->set_file_path_in(path);
        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


const char * sc_opt_get_file_path_in(sc_opt opts) {
    
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


int sc_opt_set_sessions(sc_opt opts, cm_vct * sessions) {
    
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


int sc_opt_get_sessions(sc_opt opts, cm_vct * sessions) {
    
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
        const std::optional<std::vector<const mc_session *>> & v
            = o->get_sessions();

        //if a STL vector is present, do the copy
        if (v.has_value() && !v.value().empty()) {
            ret = cm_vct_rsz(sessions, v.value().size());
            std::memcpy(sessions->data, v.value().data(),
                        v.value().size() * sizeof(cm_lst_node *));
            return 0;

        } else {
            cm_del_vct(sessions);
            sc_errno = SC_ERR_OPT_EMPTY;
            return -1;
        }
        
    } catch (const std::exception & excp) {
        cm_del_vct(sessions);
        exception_sc_errno(excp);
        return -1;
    }
}


void sc_opt_set_map(sc_opt opts, mc_vm_map * map) {
    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    o->set_map(map);
    return;
}


mc_vm_map const * sc_opt_get_map(sc_opt opts) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    return o->get_map();
}


int sc_opt_set_alignment(sc_opt opts, int alignment) {
    
    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    try {
        o->set_alignment(alignment);
        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}

    
unsigned int sc_opt_get_alignment(sc_opt opts) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    //return NULL if optional is not set or there is an error
    try {
        std::optional<int> alignment = o->get_alignment();
    
        if (alignment.has_value()) {
            return alignment.value();
        } else {
            sc_errno = SC_ERR_OPT_EMPTY;
            return -1;
        }
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


unsigned int sc_opt_get_arch_byte_width(sc_opt opts) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    return o->get_arch_byte_width();
}


/*
 * In these setter functions, the C++ setter is passed a STL vector
 * constructed from a CMore vector
 */

int sc_opt_set_omit_areas(sc_opt opts, cm_vct * omit_areas) {

    //call generic setter
    return _opt_c_constraint_setter(opts, omit_areas,
                                    &sc::opt::set_omit_areas);
}


int sc_opt_get_omit_areas(sc_opt opts, cm_vct * omit_areas) {

    //call generic getter
    return _opt_c_constraint_getter(opts, omit_areas,
                                    &sc::opt::get_omit_areas);
}


int sc_opt_set_omit_objs(sc_opt opts, cm_vct * omit_objs) {

    //call generic setter
    return _opt_c_constraint_setter(opts, omit_objs,
                                    &sc::opt::set_omit_objs);
}


int sc_opt_get_omit_objs(sc_opt opts, cm_vct * omit_objs) {

    //call generic getter
    return _opt_c_constraint_getter(opts, omit_objs,
                                    &sc::opt::get_omit_objs);
}


int sc_opt_set_exclusive_areas(sc_opt opts, cm_vct * exclusive_areas) {

    //call generic setter
    return _opt_c_constraint_setter(opts, exclusive_areas,
                                    &sc::opt::set_exclusive_areas);
}


int sc_opt_get_exclusive_areas(sc_opt opts, cm_vct * exclusive_areas) {
    
    //call generic getter
    return _opt_c_constraint_getter(opts, exclusive_areas,
                                    &sc::opt::get_exclusive_areas);
}


int sc_opt_set_exclusive_objs(sc_opt opts, cm_vct * exclusive_objs) {

    //call generic setter
    return _opt_c_constraint_setter(opts, exclusive_objs,
                                    &sc::opt::set_exclusive_objs);
}


int sc_opt_get_exclusive_objs(sc_opt opts, cm_vct * exclusive_objs) {

    //call generic getter
    return _opt_c_constraint_getter(opts, exclusive_objs,
                                    &sc::opt::get_exclusive_objs);
}


int sc_opt_set_addr_range(sc_opt opts, sc_addr_range * range) {

    //cast opaque handle into class
    sc::opt * o = static_cast<sc::opt *>(opts);

    try {
        o->set_addr_range(std::pair(range->min, range->max));
        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_opt_get_addr_range(sc_opt opts, sc_addr_range * range) {

    
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
