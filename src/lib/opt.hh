#pragma once

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


/* Configuration options for a scan. The options in this base class
 * apply to all scans. */
class options {

    private:
        //[attributes]
        //save & load file paths
        std::string file_path_out;
        std::string file_path_in;

        //MemCry session for memory operations
        mc_session const * session;
        mc_vm_map const * map;

        //scan core settings
        std::optional<unsigned int> alignment;
        const unsigned int arch_byte_width;

        /*
         *  The following attributes define constraints to apply
         *  to mc_vm_map. For example, `omit_objs` will not include
         *  any objects in this vector in the scan. Constraints can be
         *  applied at object and area granularity.
         *
         *  omit_*      - Do not include these areas/objects in the scan.
         *  exclusive_* - Only scan these areas/objects (union set).
         */

        //scan constraints
        std::optional<std::vector<cm_lst_node *>> omit_areas_set;
        std::optional<std::vector<cm_lst_node *>> omit_objs_set;
        std::optional<std::vector<cm_lst_node *>> exclusive_areas_set;
        std::optional<std::vector<cm_lst_node *>> exclusive_objs_set;
        std::optional<std::pair<uintptr_t, uintptr_t>> addr_range;


    public:
        //[methods]
        //ctor
        options(unsigned int _arch_byte_width)
        : arch_byte_width(_arch_byte_width), session(nullptr) {}

        /*
         *  This destructor is virtual to coerce this class into including RTTI.
         */
        virtual ~options() {};

        //getters & setters
        void set_file_path_out(std::string _file_path_out) {
            file_path_out = _file_path_out;
        }
        std::string get_file_path_out() const {
            return file_path_out;
        }

        void set_file_path_in(std::string _file_path_in) {
            file_path_in = _file_path_in;
        }
        std::string get_file_path_in() const {
            return file_path_in;
        }

        void set_session(mc_session * _session) {
            session = _session;
        }
        mc_session const * get_session() const {
            return session;
        }

        void set_map(mc_vm_map * _map) {
            map = _map;
        }
        mc_vm_map const * get_map() const {
            return map;
        }

        void set_alignment(int _alignment) {
            alignment = _alignment;
        }
        std::optional<unsigned int> get_alignment() const {
            return alignment;
        }
        std::optional<unsigned int> get_arch_byte_width() const {
            return arch_byte_width;
        }
        
        void set_omit_areas_set(std::vector<cm_lst_node *> & _omit_areas_set) {
            omit_areas_set = _omit_areas_set;
        }
        const std::optional<std::vector<cm_lst_node *>> &
                           get_omit_areas_set() const {
            return omit_areas_set;
        }

        void set_omit_objs_set(std::vector<cm_lst_node *> & _omit_objs_set) {
            omit_objs_set = _omit_objs_set;
        }
        const std::optional<std::vector<cm_lst_node *>> &
                           get_omit_objs_set() const {
            return omit_areas_set;
        }

        void set_exclusive_areas_set(std::vector<cm_lst_node *> &
                                                 _exclusive_areas_set) {
            exclusive_areas_set = _exclusive_areas_set;
        }
        const std::optional<std::vector<cm_lst_node *>> &
                           get_exclusive_areas_set() const {
            return exclusive_areas_set;
        }

        void set_exclusive_objs_set(std::vector<cm_lst_node *> &
                                                _exclusive_objs_set) {
            exclusive_objs_set = _exclusive_objs_set;
        }
        const std::optional<std::vector<cm_lst_node *>> &
                           get_exclusive_objs_set() const {
            return exclusive_objs_set;
        }

        void set_addr_range(std::pair<uintptr_t, uintptr_t> _addr_range) {
            addr_range = _addr_range;
        }
        const std::optional<std::pair<uintptr_t, uintptr_t>>
                                      get_addr_range() const {
            return addr_range;
        }
};


//value scan options (libscanmem)
class val_options : public options {

    //TODO
};


//pointer scan options
class ptr_options : public options {

    private:
        //[attributes]
        //address of root node            
        std::optional<uintptr_t> target_addr;

        //preset offsets (in reverse order)
        std::optional<std::vector<uint32_t>> preset_offsets;


    public:
        //[methods]
        //ctor
        ptr_options(unsigned int _arch_byte_width) : options(_arch_byte_width) {}
        
        //getters & setters
        void set_target_addr(uintptr_t _target_addr) {
            target_addr = _target_addr;
        }
        std::optional<uintptr_t> get_target_addr() const {
            return target_addr;
        }

        void set_preset_offsets(std::vector<uint32_t> _preset_offsets) {
            preset_offsets = _preset_offsets;
        }        
        std::optional<std::vector<uint32_t>> get_preset_offsets() const {
            return preset_offsets;
        }
};


//pattern scan options
class ptn_options : public options {

    //TODO
};
