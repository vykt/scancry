//standard template library
#include <string>
#include <functional>

//C standard library
#include <cstring>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <doctest/doctest.h>

//local headers
#include "filters.hh"
#include "common.hh"
#include "class_helper.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/opt.hh"



/*
 *  --- [SHARED] ---
 */

namespace _shared {

/*
 *  NOTE: Because setters of the scan and static sets attempt to lock 
 *        the object passed to them by pointer, an empty set is
 *        provided here.
 */

//map area set
static sc::map_area_set ma_set;


//compare memcry sessions (vector)
static void _session_elem_eq(const mc_session & elem_0,
                             const mc_session & elem_1) {

    REQUIRE_EQ(std::memcmp(&elem_0, &elem_1, sizeof(elem_0)), 0);

    return;    
}


//compare offsets (vector)
static void _off_elem_eq(const off_t & elem_0, const off_t & elem_1) {

    REQUIRE_EQ(elem_0, elem_1);

    return;
}


//compare cmore node pointers (red-black tree)
static void _cm_node_key_data_eq(cm_lst_node * const & key_0,
                                 mc_vm_area * const & data_0,
                                 cm_lst_node * const & key_1,
                                 mc_vm_area * const & data_1) {

    REQUIRE_EQ(key_0, key_1);
    REQUIRE_EQ(data_0, data_1);

    return;    
}


//fully populate a `opt`
static void _populate_opt(sc::opt & opts) {

    int ret;

    mc_session ses[4];
    std::memset(ses, 0xFA, sizeof(mc_session) * 4);
    cm_vct new_sessions;
    

    /*
     *  NOTE: Do not fully initialise sessions & scan set, they're
     *        too cumbersome to initialise & are tested independently.
     */

    //build new sessions
    _class_helper::vct::populate(new_sessions, ses, 4);


    //call setters
    ret = opts.set_file_pathname_out("lolyou1337");
    REQUIRE_EQ(ret, 0);

    ret = opts.set_file_pathname_in("phoon");
    REQUIRE_EQ(ret, 0);

    ret = opts.set_sessions(new_sessions);
    REQUIRE_EQ(ret, 0);

    ret = opts.set_map((mc_vm_map *) 0x10203040);
    REQUIRE_EQ(ret, 0);

    ret = opts.set_addr_width(sc::AW64);
    REQUIRE_EQ(ret, 0);

    ret = opts.set_scan_set(&_shared::ma_set);
    REQUIRE_EQ(ret, 0);

    //cleanup
    cm_del_vct(&new_sessions);

    return;
}


//fully populate a `opt_ptrscan`
static void _populate_opt_ptrscan(sc::opt_ptrscan & opts_ptr) {


    int ret;

    off_t offs[4] = { 0x10, 0x20, 0x30, 0x40 };
    cm_vct new_preset_offsets;
    

    /*
     *  NOTE: Do not fully initialise the static set, they're
     *        too cumbersome to initialise & are tested independently.
     */

    //build new preset offsets
    _class_helper::vct::populate(new_preset_offsets, offs, 4);


    //call setters
    ret = opts_ptr.set_target_addr(0x1337);
    REQUIRE_EQ(ret, 0);

    ret = opts_ptr.set_alignment(0x10);
    REQUIRE_EQ(ret, 0);

    ret = opts_ptr.set_max_obj_sz(0x800);
    REQUIRE_EQ(ret, 0);

    ret = opts_ptr.set_max_depth(5);
    REQUIRE_EQ(ret, 0);

    ret = opts_ptr.set_static_set(&_shared::ma_set);
    REQUIRE_EQ(ret, 0);

    ret = opts_ptr.set_preset_offsets(new_preset_offsets);
    REQUIRE_EQ(ret, 0);

    ret = opts_ptr.set_smart_scan(sc::SMART_SCAN_DISABLED);
    REQUIRE_EQ(ret, 0);

    //cleanup
    cm_del_vct(&new_preset_offsets);

    return;

}

} //end namespace `_shared`



/*
 *  --- [OPT] ---
 */

// -- ctor & dtor

namespace _opt {

    namespace _ctor_dtor {

    //ctor assertions
    static void _ctor_asserts(const sc::opt & opts) {
    
        //assert the constructor succeeded
        REQUIRE_EQ(opts.get_ctor_failed(), false);

        #ifdef SC_DEBUG
        //assert file pathnames
        REQUIRE_EQ(opts.file_pathname_out, nullptr);
        REQUIRE_EQ(opts.file_pathname_in, nullptr);
        //assert memcry
        REQUIRE_EQ(opts.sessions.is_init, false);
        REQUIRE_EQ(opts.map, nullptr);
        //assert miscellaneous
        REQUIRE_EQ(opts.addr_width, sc::val_unset::addr_width);
        REQUIRE_EQ(opts.scan_set, nullptr);
        #endif
    }


    //fixture (shared)
    static void _fixture_shared(sc::opt & opts) {
    
        #ifdef SC_DEBUG
        //(fixture) setup file pathnames
        _class_helper::str::setup_stub(opts.file_pathname_out);
        _class_helper::str::setup_stub(opts.file_pathname_in);
        //(fixture) setup memcry
        _class_helper::vct::setup_stub(opts.sessions);
        opts.map = (mc_vm_map *) 0x10203040;
        //(fixture) setup miscellaneous
        opts.addr_width = sc::AW64;
        #endif
    }


    //dtor asserts
    static void _dtor_asserts(const sc::opt & opts) {
    
        #ifdef SC_DEBUG
        /* note: use sanitizer to check file pathname dealloc */
        //assert destructors were run
        REQUIRE_EQ(opts.sessions.is_init, false);
        REQUIRE_EQ(opts.scan_set, nullptr);
        #endif
    }

    } //end namespace `_ctor_dtor`

} //end namespace `_opt`


//C++ test
TEST_CASE(test_cc_opt_subtests[0]) {
    _common::title(_common::CC, "opt", "ctor & dtor");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_ctor_dtor<sc::opt>(

        //ctor asserts
        _opt::_ctor_dtor::_ctor_asserts,

        //fixture
        [](sc::opt & opts) {
            _opt::_ctor_dtor::_fixture_shared(opts);
        },

        //dtor asserts
        _opt::_ctor_dtor::_dtor_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[0]) {
    _common::title(_common::C, "opt", "ctor & dtor");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_ctor_dtor<sc_opt, sc::opt>(

        //fn pointers
        sc_new_opt,
        sc_del_opt,

        //ctor asserts
        _opt::_ctor_dtor::_ctor_asserts,

        //fixture
        [](sc::opt & opts) {
            _opt::_ctor_dtor::_fixture_shared(opts);
        }
    );

    return;
}


// -- `file_pathname_out` setter & getter

namespace _opt {

    namespace _file_pathname_out {

    //setter value
    static const char * _new_pathname = "pathname out test";


    //setter asserts
    static void _setter_asserts(const sc::opt & opts) {

        #ifdef SC_DEBUG
        REQUIRE_NE(opts.file_pathname_out, _new_pathname);
        REQUIRE_EQ(strcmp(opts.file_pathname_out, _new_pathname),0);
        #endif
    }

    } //end namespace `_file_pathname_out`

} //end namespace `_opt`


//C++ test
TEST_CASE(test_cc_opt_subtests[1]) {
    _common::title(_common::CC,
                   "opt", "`file_pathname_out` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_str_setter_getter<sc::opt>(

        //provide test helper requirements
        nullptr, _opt::_file_pathname_out::_new_pathname,
        &sc::opt::set_file_pathname_out,
        &sc::opt::get_file_pathname_out,

        //setter assert
        _opt::_file_pathname_out::_setter_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[1]) {
    _common::title(_common::C,
                   "opt", "`file_pathname_out` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_str_setter_getter<sc_opt, sc::opt>(

        //provide test helper requirements
        _opt::_file_pathname_out::_new_pathname,

        //fn pointers
        sc_new_opt,
        sc_del_opt,
        sc_opt_set_file_pathname_out,
        sc_opt_get_file_pathname_out,


        //setter assert
        _opt::_file_pathname_out::_setter_asserts
    );

    return;
}


// -- `file_pathname_in` setter & getter

namespace _opt {

    namespace _file_pathname_in {

    //setter value
    static const char * _new_pathname = "pathname out test";


    //setter asserts
    static void _setter_asserts(const sc::opt & opts) {

        #ifdef SC_DEBUG
        REQUIRE_NE(opts.file_pathname_in, _new_pathname);
        REQUIRE_EQ(strcmp(opts.file_pathname_in, _new_pathname),0);
        #endif
    }

    } //end namespace `_file_pathname_in`

} //end namespace `_opt`


//C++ test
TEST_CASE(test_cc_opt_subtests[1]) {
    _common::title(_common::CC,
                   "opt", "`file_pathname_in` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_str_setter_getter<sc::opt>(

        //provide test helper requirements
        nullptr, _opt::_file_pathname_in::_new_pathname,
        &sc::opt::set_file_pathname_in,
        &sc::opt::get_file_pathname_in,

        //setter assert
        _opt::_file_pathname_in::_setter_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[1]) {
    _common::title(_common::C,
                   "opt", "`file_pathname_in` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_str_setter_getter<sc_opt, sc::opt>(

        //provide test helper requirements
        _opt::_file_pathname_in::_new_pathname,

        //fn pointers
        sc_new_opt,
        sc_del_opt,
        sc_opt_set_file_pathname_in,
        sc_opt_get_file_pathname_in,


        //setter assert
        _opt::_file_pathname_in::_setter_asserts
    );

    return;
}


// -- `sessions` setter & getter

namespace _opt {

    namespace _sessions {

    //setter value
    static cm_vct _new_sessions;
    static mc_session _ses[4];


    //setup data
    static void _setup_data() {
        
        std::memset(_ses, 0xFA, sizeof(mc_session) * 4);
    _class_helper::vct::populate<mc_session>(
        _new_sessions, _ses, 4);
    }

    //teardown data
    static void _teardown_data() {

        cm_del_vct(&_new_sessions);
    }


    //setter asserts
    static void _setter_asserts(const sc::opt & opts) {

        #ifdef SC_DEBUG
        _class_helper::vct::assert_eq<mc_session>(
            _new_sessions, opts.sessions, _shared::_session_elem_eq);
        #endif
    }
        
    } //end namespace `_sessions`

} //end namespace `_opt`


//C++ test
TEST_CASE(test_cc_opt_subtests[3]) {
    _common::title(_common::CC,
                   "opt", "`sessions` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //setup new sessions
    _opt::_sessions::_setup_data();

    //run test helper
    _class_helper::cc::test_vct_setter_getter<sc::opt, mc_session>(

        //provide test helper requirements
        _opt::_sessions::_new_sessions,
        &sc::opt::set_sessions,
        &sc::opt::get_sessions,

        //setter assert
        _opt::_sessions::_setter_asserts,

        //element assert
        _shared::_session_elem_eq
    );

    //delete new omit areas
    _opt::_sessions::_teardown_data();

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[3]) {
    _common::title(_common::C,
                   "opt", "`sessions` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif    

    //setup new sessions
    _opt::_sessions::_setup_data();


    //run test helper
    _class_helper::c::test_vct_setter_getter<
        sc_opt, sc::opt, mc_session>(

        //provide test helper requirements
        _opt::_sessions::_new_sessions,

        //fn pointers
        sc_new_opt,
        sc_del_opt,
        sc_opt_set_sessions,
        sc_opt_get_sessions,

        //setter assert
        _opt::_sessions::_setter_asserts,

        //element assert
        _shared::_session_elem_eq
    );

    //delete new omit areas
    _opt::_sessions::_teardown_data();

    return;
}


// -- `map` setter & getter

namespace _opt {

    namespace _map {

    //setter value
    static mc_vm_map * _new_map = (mc_vm_map *) 0x10203040;


    //default getter asserts
    static void _default_getter_asserts(
        const sc::opt & opts, const mc_vm_map * map) {

        REQUIRE_EQ(map, nullptr);
    }

    //new setter asserts
    static void _new_setter_asserts(const sc::opt & opts) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(opts.map, _new_map);
        #endif
    }

    //new getter asserts
    static void _new_getter_asserts(
        const sc::opt & opts, const mc_vm_map * map) {

        REQUIRE_EQ(map, _new_map);
    }

    } //end namespace `_map`

} //end namespace `_opt`


//C++ test
TEST_CASE(test_cc_opt_subtests[4]) {
    _common::title(_common::CC,
                   "opt", "`map` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_value_setter_getter<sc::opt, mc_vm_map *>(

        //provide test helper requirements
        _opt::_map::_new_map,
        /* fn pointer type-cast to satisfy compiler */
        (int (sc::opt::*)(mc_vm_map * const)) &sc::opt::set_map,
        &sc::opt::get_map,

        //default value getter asserts
        _opt::_map::_default_getter_asserts,

        //new value setter asserts
        _opt::_map::_new_setter_asserts,

        //new value getter asserts
        _opt::_map::_new_getter_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[4]) {
    _common::title(_common::C,
                   "opt", "`map` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_value_setter_getter<
        sc_opt, sc::opt, mc_vm_map *>(

        //provide test helper requirements
        _opt::_map::_new_map,

        //fn pointers
        sc_new_opt,
        sc_del_opt,
        (int (*)(sc_opt *, mc_vm_map * const)) sc_opt_set_map,
        sc_opt_get_map,
    
        //default value getter asserts
        _opt::_map::_default_getter_asserts,

        //new value setter asserts
        _opt::_map::_new_setter_asserts,

        //new value getter asserts
        _opt::_map::_new_getter_asserts
    );

    return;
}


// -- `addr width` setter & getter

namespace _opt {

    namespace _addr_width {

    //setter asserts
    static void _setter_asserts(const sc::opt & opts) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(opts.addr_width, sc::AW64);
        #endif
    }

    } //end namespace `_addr_width`

} //end namespace `_opt`


//C++ test
TEST_CASE(test_cc_opt_subtests[5]) {
    _common::title(_common::CC,
                   "opt", "`addr_width` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_enm_setter_getter<sc::opt, sc::addr_width>(

        //provide test helper requirements
        sc::val_unset::addr_width, sc::AW64,
        &sc::opt::set_addr_width,
        &sc::opt::get_addr_width,

        //setter asserts
        _opt::_addr_width::_setter_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[5]) {
    _common::title(_common::C,
                   "opt", "`addr_width` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_enm_setter_getter<
        sc_opt, sc::opt, sc_addr_width>(

        //provide test helper requirements
        SC_ADDR_WIDTH_UNSET, SC_AW64,

        //fn pointers
        sc_new_opt,
        sc_del_opt,
        sc_opt_set_addr_width,
        sc_opt_get_addr_width,

        //setter asserts
        _opt::_addr_width::_setter_asserts
    );

    return;
}


// -- `scan_set` setter & getter

namespace _opt {

    namespace _scan_set {

    //default getter asserts
    static void _default_getter_asserts(
        const sc::opt & opts, const sc::map_area_set * ma_set) {

        REQUIRE_EQ(ma_set, nullptr);
    }

    //new setter asserts
    static void _new_setter_asserts(const sc::opt & opts) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(opts.scan_set, &_shared::ma_set);
        #endif
    }

    //new getter asserts
    static void _new_getter_asserts(
        const sc::opt & opts, const sc::map_area_set * ma_set) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(ma_set, &_shared::ma_set);
        #endif
    }
    
    } //end namespace `_scan_set`

} //end namespace `_opt`


//C++ test
TEST_CASE(test_cc_opt_subtests[6]) {
    _common::title(_common::CC,
                   "opt", "`scan_set` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_obj_setter_getter<
        sc::opt, sc::map_area_set> (

        //provide test helper requirements
        &_shared::ma_set,
        &sc::opt::set_scan_set,
        &sc::opt::get_scan_set,

        //default getter asserts
        _opt::_scan_set::_default_getter_asserts,

        //new setter asserts
        _opt::_scan_set::_new_setter_asserts,

        //new getter asserts
        _opt::_scan_set::_new_getter_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[6]) {
    _common::title(_common::C,
                   "opt", "`scan_set` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_obj_setter_getter<
        sc_opt, sc::opt, sc_map_area_set> (

        //provide test helper requirements
        (const sc_map_area_set *) &_shared::ma_set,

        //fn pointers
        sc_new_opt,
        sc_del_opt,
        sc_opt_set_scan_set,
        sc_opt_get_scan_set,

        //default getter asserts
        [](const sc::opt & opts, const sc_map_area_set * ma_set) {
            _opt::_scan_set::_default_getter_asserts(
                opts, (sc::map_area_set *) ma_set);
        },

        //new setter asserts
        _opt::_scan_set::_new_setter_asserts,

        //new getter asserts
        [](const sc::opt & opts, const sc_map_area_set * ma_set) {
            _opt::_scan_set::_new_getter_asserts(
                opts, (sc::map_area_set *) ma_set);
        }
    );

    return;
}


// -- copy ctor

namespace _opt {

    namespace _copy_ctor {

    //setup data
    static int _old_vct_len;


    //copy ctor asserts
    static void _copy_ctor_asserts(
        const sc::opt & dst_opts, const sc::opt & src_opts) {

        //assert the constructor succeeded
        REQUIRE_EQ(dst_opts.get_ctor_failed(), false);

        #ifdef SC_DEBUG
        //save old lengths
        _old_vct_len  = dst_opts.sessions.len;

        //assert pathnames
        REQUIRE_NE(dst_opts.file_pathname_out,
                   src_opts.file_pathname_out);
        REQUIRE_EQ(strcmp(dst_opts.file_pathname_out,
                          src_opts.file_pathname_out), 0);

        REQUIRE_NE(dst_opts.file_pathname_in,
                   src_opts.file_pathname_in);
        REQUIRE_EQ(strcmp(dst_opts.file_pathname_out,
                          src_opts.file_pathname_out), 0);

        //assert memcry
        _class_helper::vct::assert_eq<mc_session>(
            dst_opts.sessions, src_opts.sessions,
            _shared::_session_elem_eq);
        REQUIRE_EQ(dst_opts.map, src_opts.map);

        //miscellaneous
        REQUIRE_EQ(dst_opts.addr_width, src_opts.addr_width);
        REQUIRE_EQ(dst_opts.scan_set, src_opts.scan_set);
        #endif      
    }
    
    //post source dtor asserts
    static void _src_dtor_asserts(const sc::opt & opts) {

        #ifdef SC_DEBUG
        //assert pathnames
        REQUIRE_NE(opts.file_pathname_out, nullptr);
        REQUIRE_NE(opts.file_pathname_in, nullptr);
        //assert memcry
        REQUIRE_EQ(opts.sessions.is_init, true);
        REQUIRE_EQ(opts.sessions.len, _old_vct_len);
        REQUIRE_NE(opts.map, nullptr);
        //assert miscellaneous
        REQUIRE_EQ(opts.addr_width, sc::AW64);
        REQUIRE_EQ(opts.scan_set, &_shared::ma_set);
        #endif
    }

    //dtor asserts
    static void _dtor_asserts(const sc::opt & opts) {
        
        #ifdef SC_DEBUG
        REQUIRE_EQ(opts.sessions.is_init, false);
        REQUIRE_EQ(opts.scan_set, nullptr);
        #endif
    }

    } //end namespace `_copy_ctor`

} //end namespace `_opt`


//C++ test
TEST_CASE(test_cc_opt_subtests[7]) {
    _common::title(_common::CC, "opt", "copy ctor");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_copy_ctor<sc::opt>(

        //source object setup
        _shared::_populate_opt,

        //copy ctor asserts
        _opt::_copy_ctor::_copy_ctor_asserts,

        //post source dtor asserts
        _opt::_copy_ctor::_src_dtor_asserts,

        //dtor asserts
        _opt::_copy_ctor::_dtor_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[7]) {
    _common::title(_common::C, "opt", "copy ctor");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_copy_ctor<sc_opt, sc::opt>(

        //fn pointers
        sc_new_opt,
        sc_del_opt,
        sc_copy_opt,


        //source object setup
        _shared::_populate_opt,

        //copy ctor asserts
        _opt::_copy_ctor::_copy_ctor_asserts,

        //post source dtor asserts
        _opt::_copy_ctor::_src_dtor_asserts
    );

    return;
}


// -- copy assign

namespace _opt {

    namespace _copy_assign {

    //destination object setup
    static void _dst_obj_setup(sc::opt & opts) {

        #ifdef SC_DEBUG
        //(fixture) setup file pathnames
        _class_helper::str::setup_stub(opts.file_pathname_out);
        _class_helper::str::setup_stub(opts.file_pathname_in);
        //(fixture) setup memcry
        _class_helper::vct::setup_stub(opts.sessions);
        opts.map = (mc_vm_map *) 0x10203040;
        //(fixture) setup miscellaneous
        opts.addr_width = sc::AW64;
        opts.scan_set = nullptr;
        #endif
    }


    //copy assign asserts
    static void _copy_assign_asserts(
        const sc::opt & dst_opts, const sc::opt & src_opts) {

        //assert the constructor succeeded
        REQUIRE_EQ(dst_opts.get_ctor_failed(), false);

        #ifdef SC_DEBUG
        //assert pathnames
        REQUIRE_NE(dst_opts.file_pathname_out,
                   src_opts.file_pathname_out);
        REQUIRE_EQ(strcmp(dst_opts.file_pathname_out,
                          src_opts.file_pathname_out), 0);

        REQUIRE_NE(dst_opts.file_pathname_in,
                   src_opts.file_pathname_in);
        REQUIRE_EQ(strcmp(dst_opts.file_pathname_out,
                          src_opts.file_pathname_out), 0);

        //assert memcry
        _class_helper::vct::assert_eq<mc_session>(
            dst_opts.sessions, src_opts.sessions,
            _shared::_session_elem_eq);
        REQUIRE_EQ(dst_opts.map, src_opts.map);

        //miscellaneous
        REQUIRE_EQ(dst_opts.addr_width, src_opts.addr_width);
        REQUIRE_EQ(dst_opts.scan_set, src_opts.scan_set);
        #endif
    }

    } //end namespace `_copy_assign`

} //end namespace `_opt`


//C++ test
TEST_CASE(test_cc_opt_subtests[8]) {
    _common::title(_common::CC, "opt", "copy assign");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_copy_assign<sc::opt>(

        //source object setup
        _shared::_populate_opt,
        
        //destination object setup
        _opt::_copy_assign::_dst_obj_setup,

        //copy assign asserts
        _opt::_copy_assign::_copy_assign_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[8]) {
    _common::title(_common::C, "opt", "copy assign");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_copy_assign<sc_opt, sc::opt>(

        //fn pointers
        sc_new_opt,
        sc_del_opt,
        sc_copy_assign_opt,


        //source object setup
        _shared::_populate_opt,

        //destination object setup
        _opt::_copy_assign::_dst_obj_setup,

        //copy assign asserts
        _opt::_copy_assign::_copy_assign_asserts
    );

    return;
}


// -- reset

namespace _opt {

    namespace _reset {

    //reset asserts
    static void _reset_asserts(const sc::opt & opts) {

        #ifdef SC_DEBUG
        //filepaths asserts
        REQUIRE_EQ(opts.file_pathname_out, nullptr);
        REQUIRE_EQ(opts.file_pathname_in, nullptr);

        //memcry asserts
        REQUIRE_EQ(opts.sessions.is_init, false);
        REQUIRE_EQ(opts.map, nullptr);

        //miscellaneous asserts
        REQUIRE_EQ(opts.addr_width, sc::val_unset::addr_width);
        REQUIRE_EQ(opts.scan_set, nullptr);
        #endif
    }

    } //end namespace `_reset`

} //end namespace `_opt`


//C++ test
TEST_CASE(test_cc_opt_subtests[9]) {
    _common::title(_common::CC, "opt", "reset");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_reset<sc::opt>(

        //setup
        _shared::_populate_opt,

        //reset asserts
        _opt::_reset::_reset_asserts
    );

    return; 
}


//C test
TEST_CASE(test_c_opt_subtests[9]) {
    _common::title(_common::C, "opt", "reset");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_reset<sc_opt, sc::opt>(
        
        //fn pointers
        sc_new_opt,
        sc_del_opt,
        sc_opt_reset,


        //setup
        _shared::_populate_opt,

        //reset asserts
        _opt::_reset::_reset_asserts
    );
}



/*
 *  --- [OPT_PTRSCAN] ---
 */

// -- ctor & dtor

namespace _opt_ptrscan {

    namespace _ctor_dtor {

        //ctor asserts
        static void _ctor_asserts(const sc::opt_ptrscan & opts_ptr) {

            //assert the constructor succeeded
            REQUIRE_EQ(opts_ptr.get_ctor_failed(), false);

            #ifdef SC_DEBUG
            //assert primitives
            REQUIRE_EQ(opts_ptr.target_addr, 0x0);
            REQUIRE_EQ(opts_ptr.alignment,
                       sc::val_default::alignment);
            REQUIRE_EQ(opts_ptr.max_obj_sz,
                       sc::val_default::max_obj_sz);
            REQUIRE_EQ(opts_ptr.max_depth,
                       sc::val_default::max_depth);
            REQUIRE_EQ(opts_ptr.static_set, nullptr);
            //assert composites
            REQUIRE_EQ(opts_ptr.preset_offsets.is_init, false);
            //assert smart scan
            REQUIRE_EQ(opts_ptr.smart_scan, sc::val_default::smart_scan);
            #endif
        }

        //fixture (shared)
        static void _fixture_shared(sc::opt_ptrscan & opts_ptr) {

            #ifdef SC_DEBUG
            //(fixture) setup primitives
            opts_ptr.target_addr = 0x1337;
            opts_ptr.alignment   = 0x10;
            opts_ptr.max_obj_sz  = 0x800;
            opts_ptr.max_depth   = 5;
            //(fixture) setup composites
            _class_helper::vct::setup_stub(opts_ptr.preset_offsets);
            #endif
        }

        //dtor asserts
        static void _dtor_asserts(const sc::opt_ptrscan & opts_ptr) {

            #ifdef SC_DEBUG
            //assert destructors were run
            REQUIRE_EQ(opts_ptr.static_set, nullptr);
            REQUIRE_EQ(opts_ptr.preset_offsets.is_init, false);
            #endif
        }
    
    } //end namespace `_ctor_dtor`
    
} //end namespace `opt_ptrscan`


//C++ tests
TEST_CASE(test_cc_opt_subtests[10]) {
    _common::title(_common::CC, "opt_ptrscan", "ctor & dtor");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_ctor_dtor<sc::opt_ptrscan>(

        //ctor asserts
        _opt_ptrscan::_ctor_dtor::_ctor_asserts,

        //fixture
        [](sc::opt_ptrscan & opts_ptr) {

            _opt_ptrscan::_ctor_dtor::_fixture_shared(opts_ptr);
            #ifdef SC_DEBUG
            opts_ptr.smart_scan = sc::SMART_SCAN_ENABLED;
            #endif
        },

        //dtor asserts
        _opt_ptrscan::_ctor_dtor::_dtor_asserts
    );

    return;
}


//ctor & dtor
TEST_CASE(test_c_opt_subtests[10]) {
    _common::title(_common::C, "opt_ptrscan", "ctor & dtor");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_ctor_dtor<sc_opt_ptrscan, sc::opt_ptrscan>(

        //fn pointers
        sc_new_opt_ptr,
        sc_del_opt_ptr,


        //ctor asserts
        _opt_ptrscan::_ctor_dtor::_ctor_asserts,

        //fixture
        [](sc::opt_ptrscan & opts_ptr) {

            _opt_ptrscan::_ctor_dtor::_fixture_shared(opts_ptr);
            #ifdef SC_DEBUG
            opts_ptr.smart_scan = (sc::smart_scan) SC_SMART_SCAN_ENABLED;
            #endif
        }
    );

    return;
}


// -- `target_addr` setter & getter

namespace _opt_ptrscan {

    namespace _target_addr {

    //setter value
    static uintptr_t _new_target_addr = 0x1337;


    //default getter asserts
    static void _default_getter_asserts(
        const sc::opt_ptrscan & opts_ptr, uintptr_t target_addr) {

        REQUIRE_EQ(target_addr, 0x0);
    }

    //new setter asserts
    static void _new_setter_asserts(const sc::opt_ptrscan & opts_ptr) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(opts_ptr.target_addr, _new_target_addr);
        #endif
    }

    //new getter asserts
    static void _new_getter_asserts(
        const sc::opt_ptrscan & opts_ptr, uintptr_t target_addr) {

        REQUIRE_EQ(target_addr, _new_target_addr);
    }
    
    } //end namespace `_target_addr`
    
} //end namespace `opt_ptrscan`


//C++ test
TEST_CASE(test_cc_opt_subtests[11]) {
    _common::title(_common::CC,
                   "opt_ptrscan", "`target_addr` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_value_setter_getter<
        sc::opt_ptrscan, uintptr_t>(

        //provide test helper requirements
        _opt_ptrscan::_target_addr::_new_target_addr,
        &sc::opt_ptrscan::set_target_addr,
        &sc::opt_ptrscan::get_target_addr,


        //default getter asserts
        _opt_ptrscan::_target_addr::_default_getter_asserts,

        //new setter asserts
        _opt_ptrscan::_target_addr::_new_setter_asserts,

        //new getter asserts
        _opt_ptrscan::_target_addr::_new_getter_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[11]) {
    _common::title(_common::C,
                   "opt_ptrscan", "`target_addr` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_value_setter_getter<
        sc_opt_ptrscan, sc::opt_ptrscan, uintptr_t>(

        //provide test helper requirements
        _opt_ptrscan::_target_addr::_new_target_addr,

        //fn pointers
        sc_new_opt_ptr,
        sc_del_opt_ptr,
        sc_opt_ptr_set_target_addr,
        sc_opt_ptr_get_target_addr,


        //default getter asserts
        _opt_ptrscan::_target_addr::_default_getter_asserts,

        //new setter asserts
        _opt_ptrscan::_target_addr::_new_setter_asserts,

        //new getter asserts
        _opt_ptrscan::_target_addr::_new_getter_asserts
    );

    return;
}


// -- `alignment` setter & getter

namespace _opt_ptrscan {

    namespace _alignment {

    //setter value
    static off_t _new_alignment = 0x10;

    
    //default getter asserts
    static void _default_getter_asserts(
        const sc::opt_ptrscan & opts_ptr, off_t alignment) {

        REQUIRE_EQ(alignment, sc::val_default::alignment);
    }

    //new setter asserts
    static void _new_setter_asserts(const sc::opt_ptrscan & opts_ptr) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(opts_ptr.alignment, _new_alignment);
        #endif
    }

    //new getter asserts
    static void _new_getter_asserts(
        const sc::opt_ptrscan & opts_ptr, off_t alignment) {

        REQUIRE_EQ(alignment, _new_alignment);
    }

    } //end namespace `_alignment`
    
} //end namespace `opt_ptrscan`


//C++ test
TEST_CASE(test_cc_opt_subtests[12]) {
    _common::title(_common::CC,
                   "opt_ptrscan", "`alignment` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif
    
    //run test helper
    _class_helper::cc::test_value_setter_getter<
        sc::opt_ptrscan, off_t>(

        //provide test helper requirements
        _opt_ptrscan::_alignment::_new_alignment,
        &sc::opt_ptrscan::set_alignment,
        &sc::opt_ptrscan::get_alignment,


        //default getter asserts
        _opt_ptrscan::_alignment::_default_getter_asserts,

        //new setter asserts
        _opt_ptrscan::_alignment::_new_setter_asserts,

        //new getter asserts
        _opt_ptrscan::_alignment::_new_getter_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[12]) {
    _common::title(_common::C,
                   "opt_ptrscan", "`alignment` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    off_t new_alignment = 0x10;
    
    //run test helper
    _class_helper::c::test_value_setter_getter<
        sc_opt_ptrscan, sc::opt_ptrscan, off_t>(

        //provide test helper requirements
        _opt_ptrscan::_alignment::_new_alignment,

        //fn pointers
        sc_new_opt_ptr,
        sc_del_opt_ptr,
        sc_opt_ptr_set_alignment,
        sc_opt_ptr_get_alignment,


        //default getter asserts
        _opt_ptrscan::_alignment::_default_getter_asserts,

        //new setter asserts
        _opt_ptrscan::_alignment::_new_setter_asserts,

        //new getter asserts
        _opt_ptrscan::_alignment::_new_getter_asserts
    );

    return;
}


// -- `max_obj_sz` setter & getter

namespace _opt_ptrscan {

    namespace _max_obj_sz {

    //setter value
    static off_t _new_max_obj_sz = 0x10;

    
    //default getter asserts
    static void _default_getter_asserts(
        const sc::opt_ptrscan & opts_ptr, off_t max_obj_sz) {

        REQUIRE_EQ(max_obj_sz, sc::val_default::max_obj_sz);
    }

    //new setter asserts
    static void _new_setter_asserts(const sc::opt_ptrscan & opts_ptr) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(opts_ptr.max_obj_sz, _new_max_obj_sz);
        #endif
    }

    //new getter asserts
    static void _new_getter_asserts(
        const sc::opt_ptrscan & opts_ptr, off_t max_obj_sz) {

        REQUIRE_EQ(max_obj_sz, _new_max_obj_sz);
    }

    } //end namespace `_max_obj_sz`
    
} //end namespace `opt_ptrscan`


//C++ test
TEST_CASE(test_cc_opt_subtests[13]) {
    _common::title(_common::CC,
                   "opt_ptrscan", "`max_obj_sz` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_value_setter_getter<
        sc::opt_ptrscan, off_t>(

        //provide test helper requirements
        _opt_ptrscan::_max_obj_sz::_new_max_obj_sz,
        &sc::opt_ptrscan::set_max_obj_sz,
        &sc::opt_ptrscan::get_max_obj_sz,


        //default getter asserts
        _opt_ptrscan::_max_obj_sz::_default_getter_asserts,

        //new setter asserts
        _opt_ptrscan::_max_obj_sz::_new_setter_asserts,

        //new getter asserts
        _opt_ptrscan::_max_obj_sz::_new_getter_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[13]) {
    _common::title(_common::C,
                   "opt_ptrscan", "`max_obj_sz` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_value_setter_getter<
        sc_opt_ptrscan, sc::opt_ptrscan, off_t>(

        //provide test helper requirements
        _opt_ptrscan::_max_obj_sz::_new_max_obj_sz,

        //fn pointers
        sc_new_opt_ptr,
        sc_del_opt_ptr,
        sc_opt_ptr_set_max_obj_sz,
        sc_opt_ptr_get_max_obj_sz,


        //default getter asserts
        _opt_ptrscan::_max_obj_sz::_default_getter_asserts,

        //new setter asserts
        _opt_ptrscan::_max_obj_sz::_new_setter_asserts,

        //new getter asserts
        _opt_ptrscan::_max_obj_sz::_new_getter_asserts
    );

    return;
}


// `max_depth` setter & getter

namespace _opt_ptrscan {

    namespace _max_depth {

    //setter value
    static int _new_max_depth = 0x10;


    //default getter asserts
    static void _default_getter_asserts(
        const sc::opt_ptrscan & opts_ptr, int max_depth) {

        REQUIRE_EQ(max_depth, sc::val_default::max_depth);
    }

    //new setter asserts
    static void _new_setter_asserts(
        const sc::opt_ptrscan & opts_ptr) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(opts_ptr.max_depth, _new_max_depth);
        #endif
    }

    //new getter asserts
    static void _new_getter_asserts(
        const sc::opt_ptrscan & opts_ptr, int max_depth) {

        REQUIRE_EQ(max_depth, _new_max_depth);
    }
    
    } //end namespace `_max_depth`
    
} //end namespace `opt_ptrscan`


//C++ test
TEST_CASE(test_cc_opt_subtests[14]) {
    _common::title(_common::CC,
                   "opt_ptrscan", "`max_depth` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_value_setter_getter<
        sc::opt_ptrscan, int>(

        //provide test helper requirements
        _opt_ptrscan::_max_depth::_new_max_depth,
        &sc::opt_ptrscan::set_max_depth,
        &sc::opt_ptrscan::get_max_depth,


        //default getter asserts
        _opt_ptrscan::_max_depth::_default_getter_asserts,

        //new setter asserts
        _opt_ptrscan::_max_depth::_new_setter_asserts,

        //new getter asserts
        _opt_ptrscan::_max_depth::_new_getter_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[14]) {
    _common::title(_common::C,
                   "opt_ptrscan", "`max_depth` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_value_setter_getter<
        sc_opt_ptrscan, sc::opt_ptrscan, int>(

        //provide test helper requirements
        _opt_ptrscan::_max_depth::_new_max_depth,

        //fn pointers
        sc_new_opt_ptr,
        sc_del_opt_ptr,
        sc_opt_ptr_set_max_depth,
        sc_opt_ptr_get_max_depth,


        //default getter asserts
        _opt_ptrscan::_max_depth::_default_getter_asserts,

        //new setter asserts
        _opt_ptrscan::_max_depth::_new_setter_asserts,

        //new getter asserts
        _opt_ptrscan::_max_depth::_new_getter_asserts
    );

    return;
}


// -- `static_set` setter & getter

namespace _opt_ptrscan {

    namespace _static_set {

    //default object asserts
    static void _default_getter_asserts(
        const sc::opt_ptrscan & opts_ptr,
        const sc::map_area_set * ma_set) {

        REQUIRE_EQ(ma_set, nullptr);
    }
    
    //new object setter asserts
    static void _new_setter_asserts(
        const sc::opt_ptrscan & opts_ptr) {
    
        #ifdef SC_DEBUG
        REQUIRE_EQ(opts_ptr.static_set, &_shared::ma_set);
        #endif
    }

    //new object getter asserts
    static void _new_getter_asserts(
        const sc::opt_ptrscan & opts_ptr,
        const sc::map_area_set * ma_set) {

        REQUIRE_EQ(ma_set, &_shared::ma_set);
    }

    } //end namespace `_static_set`
    
} //end namespace `opt_ptrscan`


//C++ test
TEST_CASE(test_cc_opt_subtests[15]) {
    _common::title(_common::CC,
                   "opt_ptrscan", "`static_set` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_obj_setter_getter<
        sc::opt_ptrscan, sc::map_area_set> (

        //provide test helper requirements
        &_shared::ma_set,
        &sc::opt_ptrscan::set_static_set,
        &sc::opt_ptrscan::get_static_set,


        //default object asserts
        _opt_ptrscan::_static_set::_default_getter_asserts,
        
        //new object setter asserts
        _opt_ptrscan::_static_set::_new_setter_asserts,

        //new object getter asserts
        _opt_ptrscan::_static_set::_new_getter_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[15]) {
    _common::title(_common::C,
                   "opt_ptrscan", "`static_set` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_obj_setter_getter<
        sc_opt_ptrscan, sc::opt_ptrscan, sc_map_area_set> (

        //provide test helper requirements
        (const sc_map_area_set *) &_shared::ma_set,

        //fn pointers
        sc_new_opt_ptr,
        sc_del_opt_ptr,
        sc_opt_ptr_set_static_set,
        sc_opt_ptr_get_static_set,


        //default object asserts
        [](const sc::opt_ptrscan & opts_ptr,
           const sc_map_area_set * ma_set) {
            _opt_ptrscan::_static_set::_default_getter_asserts(
                opts_ptr, (sc::map_area_set *) ma_set);
        },
        
        //new object setter asserts
        _opt_ptrscan::_static_set::_new_setter_asserts,

        //new object getter asserts
        [](const sc::opt_ptrscan & opts_ptr,
           const sc_map_area_set * ma_set) {
            _opt_ptrscan::_static_set::_new_getter_asserts(
                opts_ptr, (sc::map_area_set *) ma_set);
        }
    );

    return;
}


// -- `preset_offsets` setter & getter

namespace _opt_ptrscan {

    namespace _preset_offsets {

    //setup value
    static cm_vct _new_preset_offsets;
    static const off_t _off[4] = { 0x10, 0x20, 0x30, 0x40 };


    //setup data
    static void _setup_data() {
        
        _class_helper::vct::populate<off_t>(
            _new_preset_offsets, _off, 4);
    }

    //teardown data
    static void _teardown_data() {

        cm_del_vct(&_new_preset_offsets);
    }


    //setter assert
    static void _setter_asserts(const sc::opt_ptrscan & opts_ptr) {

        #ifdef SC_DEBUG
        _class_helper::vct::assert_eq<off_t>(
            _new_preset_offsets,
            opts_ptr.preset_offsets,
            _shared::_off_elem_eq);
        #endif
    }

    } //end namespace `_preset_offsets`
    
} //end namespace `opt_ptrscan`


//C++ test
TEST_CASE(test_cc_opt_subtests[16]) {
    _common::title(_common::CC,
                   "opt_ptrscan", "`preset_offsets` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif
    
    //setup new preset offsets
    _opt_ptrscan::_preset_offsets::_setup_data();
    
    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_ptrscan, off_t>(

        //provide test helper requirements
        _opt_ptrscan::_preset_offsets::_new_preset_offsets,
        &sc::opt_ptrscan::set_preset_offsets,
        &sc::opt_ptrscan::get_preset_offsets,


        //setter assert
        _opt_ptrscan::_preset_offsets::_setter_asserts,

        //element assert
        _shared::_off_elem_eq
    );

    //delete new omit areas
    _opt_ptrscan::_preset_offsets::_teardown_data();

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[16]) {
    _common::title(_common::C,
                   "opt_ptrscan", "`preset_offsets` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //setup new preset offsets
    _opt_ptrscan::_preset_offsets::_setup_data();
    
    //run test helper
    _class_helper::c::test_vct_setter_getter<
        sc_opt_ptrscan, sc::opt_ptrscan, off_t>(

        //provide test helper requirements
        _opt_ptrscan::_preset_offsets::_new_preset_offsets,

        //fn pointers
        sc_new_opt_ptr,
        sc_del_opt_ptr,
        sc_opt_ptr_set_preset_offsets,
        sc_opt_ptr_get_preset_offsets,


        //setter assert
        _opt_ptrscan::_preset_offsets::_setter_asserts,

        //element assert
        _shared::_off_elem_eq
    );

    //delete new omit areas
    _opt_ptrscan::_preset_offsets::_teardown_data();

    return;
}


// -- `smart_scan` setter & getter

namespace _opt_ptrscan {

    namespace _smart_scan {

    //setter asserts
    static void _setter_asserts(const sc::opt_ptrscan & opts_ptr) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(opts_ptr.smart_scan, sc::SMART_SCAN_DISABLED);
        #endif
    }

    } //end namespace `_smart_scan`
    
} //end namespace `opt_ptrscan`


//C++ test
TEST_CASE(test_cc_opt_subtests[17]) {
    _common::title(_common::CC,
                   "opt_ptrscan", "`smart_scan` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_enm_setter_getter<
        sc::opt_ptrscan, sc::smart_scan>(

        //provide test helper requirements
        sc::val_default::smart_scan, sc::SMART_SCAN_DISABLED,
        &sc::opt_ptrscan::set_smart_scan,
        &sc::opt_ptrscan::get_smart_scan,


        //setter asserts
        _opt_ptrscan::_smart_scan::_setter_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[17]) {
    _common::title(_common::C,
                   "opt_ptrscan", "`smart_scan` setter & getter");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_enm_setter_getter<
        sc_opt_ptrscan, sc::opt_ptrscan, sc_smart_scan>(

        //provide test helper requirements
        SC_SMART_SCAN_DEFAULT, SC_SMART_SCAN_DISABLED,

        //fn pointers
        sc_new_opt_ptr,
        sc_del_opt_ptr,
        sc_opt_ptr_set_smart_scan,
        sc_opt_ptr_get_smart_scan,


        //setter asserts
        _opt_ptrscan::_smart_scan::_setter_asserts
    );

    return;
}


// -- copy ctor

namespace _opt_ptrscan {

    namespace _copy_ctor {

    //setup data
    static size_t _old_vct_len;


    //copy ctor asserts
    static void _copy_ctor_asserts(
        const sc::opt_ptrscan & dst_opts_ptr,
        const sc::opt_ptrscan & src_opts_ptr) {

        //check ctor succeeded
        REQUIRE_EQ(dst_opts_ptr.get_ctor_failed(), false);

        #ifdef SC_DEBUG
        //save old lengths
        _old_vct_len  = dst_opts_ptr.preset_offsets.len;

        //assert primitives
        REQUIRE_EQ(dst_opts_ptr.target_addr, src_opts_ptr.target_addr);
        REQUIRE_EQ(dst_opts_ptr.alignment, src_opts_ptr.alignment);
        REQUIRE_EQ(dst_opts_ptr.max_obj_sz, src_opts_ptr.max_obj_sz);
        REQUIRE_EQ(dst_opts_ptr.max_depth, src_opts_ptr.max_depth);

        //assert composites
        REQUIRE_EQ(dst_opts_ptr.static_set, src_opts_ptr.static_set);
            
        _class_helper::vct::assert_eq<off_t>(
            dst_opts_ptr.preset_offsets,
            src_opts_ptr.preset_offsets,
            _shared::_off_elem_eq
        );

        //assert smart scan
        REQUIRE_EQ(dst_opts_ptr.smart_scan, src_opts_ptr.smart_scan);
        #endif
    }
    
    //post source dtor asserts
    static void _src_dtor_asserts(const sc::opt_ptrscan & opts_ptr) {

        #ifdef SC_DEBUG
        //assert primitives
        REQUIRE_EQ(opts_ptr.target_addr, 0x1337);
        REQUIRE_EQ(opts_ptr.alignment, 0x10);
        REQUIRE_EQ(opts_ptr.max_obj_sz, 0x800);
        REQUIRE_EQ(opts_ptr.max_depth, 5);
        REQUIRE_EQ(opts_ptr.static_set, &_shared::ma_set);
        //assert composites are initialised
        REQUIRE_EQ(opts_ptr.preset_offsets.is_init, true);
        REQUIRE_EQ(opts_ptr.preset_offsets.len, _old_vct_len);
        //assert smart scan
        REQUIRE_EQ(opts_ptr.smart_scan, sc::SMART_SCAN_DISABLED);
        #endif
    }

    //dtor asserts
    static void _dtor_asserts(const sc::opt_ptrscan & opts_ptr) {

        #ifdef SC_DEBUG
        //assert compositers are uninitialised
        REQUIRE_EQ(opts_ptr.static_set, nullptr);
        REQUIRE_EQ(opts_ptr.preset_offsets.is_init, false);
        #endif
    }

    } //end namespace `_copy_ctor`
    
} //end namespace `opt_ptrscan`


//C++ test
TEST_CASE(test_cc_opt_subtests[18]) {
    _common::title(_common::CC, "opt_ptrscan", "copy ctor");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_copy_ctor<sc::opt_ptrscan>(

        //setup
        _shared::_populate_opt_ptrscan,

        //copy ctor asserts
        _opt_ptrscan::_copy_ctor::_copy_ctor_asserts,

        //post source dtor asserts
        _opt_ptrscan::_copy_ctor::_src_dtor_asserts,

        //dtor asserts
        _opt_ptrscan::_copy_ctor::_dtor_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[18]) {
    _common::title(_common::C, "opt_ptrscan", "copy ctor");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_copy_ctor<sc_opt_ptrscan, sc::opt_ptrscan>(

        //fn pointers
        sc_new_opt_ptr,
        sc_del_opt_ptr,
        sc_copy_opt_ptr,

        //setup
        _shared::_populate_opt_ptrscan,

        //copy ctor asserts
        _opt_ptrscan::_copy_ctor::_copy_ctor_asserts,

        //post source dtor asserts
        _opt_ptrscan::_copy_ctor::_src_dtor_asserts
    );

    return;
}


// -- copy assign

namespace _opt_ptrscan {

    namespace _copy_assign {

    //destination object setup
    static void _dst_obj_setup(sc::opt_ptrscan & opts_ptr) {

        #ifdef SC_DEBUG
        //(fixture) setup primitives
        opts_ptr.target_addr = 0x1337;
        opts_ptr.alignment   = 0x10;
        opts_ptr.max_obj_sz  = 0x800;
        opts_ptr.max_depth   = 5;
        opts_ptr.static_set  = nullptr;
        //(fixture) setup composites
        _class_helper::vct::setup_stub(opts_ptr.preset_offsets);
        //(fixture) setup smart scan
        opts_ptr.smart_scan = sc::SMART_SCAN_ENABLED;
        #endif
    }

    //copy assign asserts
    static void _copy_assign_asserts(
        const sc::opt_ptrscan & dst_opts_ptr,
        const sc::opt_ptrscan & src_opts_ptr) {

        //assert the constructor succeeded
        REQUIRE_EQ(dst_opts_ptr.get_ctor_failed(), false);

        #ifdef SC_DEBUG
        int ret;
        sc::smart_scan dst_smart_scan;
        sc::smart_scan src_smart_scan;

        //assert primitives
        REQUIRE_EQ(dst_opts_ptr.target_addr, src_opts_ptr.target_addr);
        REQUIRE_EQ(dst_opts_ptr.alignment, src_opts_ptr.alignment);
        REQUIRE_EQ(dst_opts_ptr.max_obj_sz, src_opts_ptr.max_obj_sz);
        REQUIRE_EQ(dst_opts_ptr.max_depth, src_opts_ptr.max_depth);
        REQUIRE_EQ(dst_opts_ptr.static_set, src_opts_ptr.static_set);

        //assert composites
        _class_helper::vct::assert_eq<off_t>(
            dst_opts_ptr.preset_offsets,
            src_opts_ptr.preset_offsets,
            _shared::_off_elem_eq
        );

        //assert smart scan
        REQUIRE_EQ(dst_opts_ptr.smart_scan, src_opts_ptr.smart_scan);
        #endif
    }

    } //end namespace `_copy_assign`
    
} //end namespace `opt_ptrscan`


//C++ test
TEST_CASE(test_cc_opt_subtests[19]) {
    _common::title(_common::CC, "opt_ptrscan", "copy assign");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_copy_assign<sc::opt_ptrscan>(

        //source object setup
        _shared::_populate_opt_ptrscan,

        //destination object setup
        _opt_ptrscan::_copy_assign::_dst_obj_setup,

        //copy ctor asserts
        _opt_ptrscan::_copy_assign::_copy_assign_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_opt_subtests[19]) {
    _common::title(_common::C, "opt_ptrscan", "copy assign");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_copy_assign<sc_opt_ptrscan, sc::opt_ptrscan>(

        //fn pointers
        sc_new_opt_ptr,
        sc_del_opt_ptr,
        sc_copy_assign_opt_ptr,


        //source object setup
        _shared::_populate_opt_ptrscan,

        //destination object setup
        _opt_ptrscan::_copy_assign::_dst_obj_setup,

        //copy ctor asserts
        _opt_ptrscan::_copy_assign::_copy_assign_asserts
    );

    return;
}


// -- reset

namespace _opt_ptrscan {

    namespace _reset {

    //reset asserts
    static void _reset_asserts(const sc::opt_ptrscan & opts_ptr) {

        //assert the constructor succeeded
        REQUIRE_EQ(opts_ptr.get_ctor_failed(), false);

        #ifdef SC_DEBUG
        //assert primitives
        REQUIRE_EQ(opts_ptr.target_addr, 0x0);
        REQUIRE_EQ(opts_ptr.alignment, sc::val_default::alignment);
        REQUIRE_EQ(opts_ptr.max_obj_sz, sc::val_default::max_obj_sz);
        REQUIRE_EQ(opts_ptr.max_depth, sc::val_default::max_depth);
        REQUIRE_EQ(opts_ptr.static_set, nullptr);

        //assert composites
        REQUIRE_EQ(opts_ptr.preset_offsets.is_init, false);

        //assert smart scan
        REQUIRE_EQ(opts_ptr.smart_scan, sc::val_default::smart_scan);
        #endif
    }
    
    } //end namespace `_reset`
    
} //end namespace `opt_ptrscan`


//C++ test
TEST_CASE(test_cc_opt_subtests[20]) {
    _common::title(_common::CC, "opt_ptrscan", "reset");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::cc::test_reset<sc::opt_ptrscan>(

        //setup
        _shared::_populate_opt_ptrscan,

        //reset asserts
        _opt_ptrscan::_reset::_reset_asserts
    );

    return; 
}


//C test
TEST_CASE(test_c_opt_subtests[20]) {
    _common::title(_common::C, "opt_ptrscan", "reset");
    #ifndef SC_DEBUG
    _common::release_warning();
    #endif

    //run test helper
    _class_helper::c::test_reset<sc_opt_ptrscan, sc::opt_ptrscan>(

        //fn pointers
        sc_new_opt_ptr,
        sc_del_opt_ptr,
        sc_opt_ptr_reset,


        //setup
        _shared::_populate_opt_ptrscan,

        //reset asserts
        _opt_ptrscan::_reset::_reset_asserts
    );

    return; 
}
