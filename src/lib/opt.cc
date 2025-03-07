//standard template library
#include <unistd.h>
#include <vector>
#include <string>
#include <optional>

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
void sc::opt::set_file_path_out(std::string _file_path_out) {
    this->file_path_out = _file_path_out;
}

std::string sc::opt::get_file_path_out() const {
    return this->file_path_out;
}

void sc::opt::set_file_path_in(std::string _file_path_in) {
    this->file_path_in = _file_path_in;
}

std::string sc::opt::get_file_path_in() const {
    return this->file_path_in;
}

void sc::opt::set_session(mc_session * _session) {
    this->session = _session;
}

mc_session const * sc::opt::get_session() const {
    return this->session;
}

void sc::opt::set_map(mc_vm_map * _map) {
    this->map = _map;
}

mc_vm_map const * sc::opt::get_map() const {
    return this->map;
}

void sc::opt::set_alignment(int _alignment) {
    this->alignment = _alignment;
}

std::optional<unsigned int> sc::opt::get_alignment() const {
    return this->alignment;
}

std::optional<unsigned int> sc::opt::get_arch_byte_width() const {
    return this->arch_byte_width;
}

void sc::opt::set_omit_areas(std::vector<cm_lst_node *> & _omit_areas) {
    this->omit_areas = _omit_areas;
}

const std::optional<std::vector<cm_lst_node *>> &
                               sc::opt::get_omit_areas() const {
    return this->omit_areas;
}

void sc::opt::set_omit_objs(std::vector<cm_lst_node *> & _omit_objs) {
    this->omit_objs = _omit_objs;
}

const std::optional<std::vector<cm_lst_node *>> &
                   sc::opt::get_omit_objs() const {
    return this->omit_areas;
}

void sc::opt::set_exclusive_areas(std::vector<cm_lst_node *> &
                                                  _exclusive_areas) {
    this->exclusive_areas = _exclusive_areas;
}

const std::optional<std::vector<cm_lst_node *>> &
                    sc::opt::get_exclusive_areas() const {
    return this->exclusive_areas;
}

void sc::opt::set_exclusive_objs(std::vector<cm_lst_node *> & _exclusive_objs) {
    sc::opt::exclusive_objs = _exclusive_objs;
}
const std::optional<std::vector<cm_lst_node *>> &
                   sc::opt::get_exclusive_objs() const {
    return this->exclusive_objs;
}

void sc::opt::set_addr_range(std::pair<uintptr_t, uintptr_t> _addr_range) {
    this->addr_range = _addr_range;
}
const std::optional<std::pair<uintptr_t, uintptr_t>>
                             sc::opt::get_addr_range() const {
    return this->addr_range;
}



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */


/*
 *  --- [EXTERNAL] ---
 */

void * new_sc_opt(const int arch_byte_width) {

    try {
        return new sc::opt(arch_byte_width);    
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return nullptr;
    }
};


void del_sc_opt(sc_opt opts) {
    
    try {
        delete static_cast<sc::opt *>(opts);    
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
    }

    return;
}

int sc_opt_file_path_out(sc_opt opts, const char * path);
const char * sc_opt_get_file_path_out(sc_opt opts);

int sc_opt_file_path_in(sc_opt opts, const char * path);
const char * sc_opt_get_file_path_in(sc_opt opts);

void sc_opt_session(sc_opt opts, mc_session * session);
mc_session const * sc_opt_get_session(sc_opt opts);

void sc_opt_map(sc_opt opts, mc_vm_map * map);
mc_vm_map const * sc_opt_get_map(sc_opt opts);

void sc_opt_alignment(sc_opt opts, int alignment);
int sc_opt_get_alignment(sc_opt opts);

int sc_opt_get_arch_byte_width(sc_opt opts);

int sc_opt_omit_areas(sc_opt opts, cm_vct * omit_areas);
int sc_opt_get_omit_areas(sc_opt opts, cm_vct * omit_areas);

int sc_opt_omit_objs(sc_opt opts, cm_vct * omit_objs);
int sc_opt_get_omit_objs(sc_opt opts, cm_vct * omit_objs);

int sc_opt_exclusive_areas(sc_opt opts, cm_vct * exclusive_areas);
int sc_opt_get_exclusive_areas(sc_opt opts, cm_vct * exclusive_areas);

int sc_opt_exclusive_objs(sc_opt opts, cm_vct * exclusive_objs);
int sc_opt_get_exclusive_objs(sc_opt opts, cm_vct * exclusive_objs);

void sc_opt_addr_range(sc_opt opts, sc_addr_range * range);
sc_addr_range sc_opt_get_addr_range(sc_opt opts);
