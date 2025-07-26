//standard template library
#include <string>
#include <iostream>
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



      /* ===================== * 
 ===== *  C++ INTERFACE TESTS  * =====
       * ===================== */

/*
 *  --- [OPT - HELPERS] ---
 */

//compare memcry sessions (vector)
static void _session_elem_eq(const mc_session & elem_0,
                             const mc_session & elem_1) {

    REQUIRE_EQ(std::memcmp(&elem_0, &elem_1, sizeof(elem_0)), 0);

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

    cm_lst_node * nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };
    mc_vm_area * areas[4] = {
        (mc_vm_area *) 0x50505050,
        (mc_vm_area *) 0x60606060,
        (mc_vm_area *) 0x70707070,
        (mc_vm_area *) 0x80808080
    };
    
    sc::map_area_set new_scan_set;
    

    /*
     *  NOTE: Do not fully initialise sessions & scan set, they're
     *        too cumbersome to initialise & tested independently.
     */

    //build new sessions
    _class_helper::vct::populate(new_sessions, ses, 4);

    #ifdef SC_DEBUG
    //build new scan set
    _class_helper::rbt::populate(new_scan_set.set, nodes, areas, 4);
    #endif

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

    ret = opts.set_scan_set(new_scan_set);
    REQUIRE_EQ(ret, 0);

    //cleanup
    cm_del_vct(&new_sessions);

    return;
}



/*
 *  --- [OPT - TESTS] ---
 */

//ctor & dtor
TEST_CASE(test_cc_opt_subtests[0]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt - ctor & dtor");
    #endif

    //run test helper
    _class_helper::cc::test_ctor_dtor<sc::opt>(

        //ctor asserts
        [](const sc::opt & opts) {

            //assert constructor succeeded
            REQUIRE_EQ(opts._get_ctor_failed(), false);

            #ifdef SC_DEBUG
            //assert file pathnames
            REQUIRE_EQ(opts.file_pathname_out, nullptr);
            REQUIRE_EQ(opts.file_pathname_in, nullptr);
            //assert memcry
            REQUIRE_EQ(opts.sessions.is_init, false);
            REQUIRE_EQ(opts.map, nullptr);
            //assert miscellaneous
            REQUIRE_EQ(opts.addr_width, sc::val_unset::addr_width);
            REQUIRE_EQ(opts.scan_set.set.is_init, false);
            #endif
        },


        //fixture
        [](sc::opt & opts) {

            #ifdef SC_DEBUG
            //(fixture) setup file pathnames
            _class_helper::str::setup_stub(opts.file_pathname_out);
            _class_helper::str::setup_stub(opts.file_pathname_in);
            //(fixture) setup memcry
            _class_helper::vct::setup_stub(opts.sessions);
            opts.map = (mc_vm_map *) 0x10203040;
            //(fixture) setup miscellaneous
            opts.addr_width = sc::AW64;
            _class_helper::rbt::setup_stub(opts.scan_set.set);
            #endif
        },


        //dtor asserts
        [](const sc::opt & opts) {

            #ifdef SC_DEBUG
            /* note: use sanitizer to check file pathname dealloc */
            //assert destructors were run
            REQUIRE_EQ(opts.sessions.is_init, false);
            REQUIRE_EQ(opts.scan_set.set.is_init, false);
            #endif
        }
    );

    return;
}


//`fiie_pathname_out` setter & getter
TEST_CASE(test_cc_opt_subtests[1]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt - file_pathname_out");
    #endif

    const char * new_pathname = "pathname in test";


    //run test helper
    _class_helper::cc::test_str_setter_getter<sc::opt>(

        //provide test helper requirements
        nullptr, new_pathname,
        &sc::opt::set_file_pathname_out,
        &sc::opt::get_file_pathname_out,

        //setter assert
        [&new_pathname](const sc::opt & opts) {

            #ifdef SC_DEBUG
            REQUIRE_NE(opts.file_pathname_out, new_pathname);
            REQUIRE_EQ(strcmp(opts.file_pathname_out, new_pathname),0);
            #endif
        }            
    );

    return;
}


//`fiie_pathname_in` setter & getter
TEST_CASE(test_cc_opt_subtests[2]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt - file_pathname_in");
    #endif

    const char * new_pathname = "pathname in test";


    //run test helper
    _class_helper::cc::test_str_setter_getter<sc::opt>(

        //provide test helper requirements
        nullptr, new_pathname,
        &sc::opt::set_file_pathname_in,
        &sc::opt::get_file_pathname_in,

        //setter assert
        [&new_pathname](const sc::opt & opts) {

            #ifdef SC_DEBUG
            REQUIRE_NE(opts.file_pathname_in, new_pathname);
            REQUIRE_EQ(strcmp(opts.file_pathname_in, new_pathname),0);
            #endif
        }  
    );

    return;
}


//`sessions` setter & getter
TEST_CASE(test_cc_opt_subtests[3]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt - sessions");
    #endif

    int ret;
    cm_vct new_sessions;
    mc_session ses[4];
    std::memset(ses, 0xFA, sizeof(mc_session) * 4);
    

    //setup new sessions
    _class_helper::vct::populate<mc_session>(
        new_sessions, ses, 4);

    
    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt, mc_session>(

        //provide test helper requirements
        new_sessions,
        &sc::opt::set_sessions,
        &sc::opt::get_sessions,

        //setter assert
        [&new_sessions](const sc::opt & opts) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<mc_session>(
                new_sessions, opts.sessions, _session_elem_eq);
            #endif
        },

        //element assert
        _session_elem_eq
    );

    //delete new omit areas
    cm_del_vct(&new_sessions);

    return;
}


//`map` setter & getter
TEST_CASE(test_cc_opt_subtests[4]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt - map");
    #endif

    mc_vm_map * new_map = (mc_vm_map *) 0x10203040;


    //run test helper
    _class_helper::cc::test_value_setter_getter<sc::opt, mc_vm_map *>(

        //provide test helper requirements
        new_map,
        /* fn pointer type-cast to satisfy compiler */
        (int (sc::opt::*)(mc_vm_map * const)) &sc::opt::set_map,
        &sc::opt::get_map,

        //default value asserts
        [](const sc::opt & opts, const mc_vm_map * map) {
            REQUIRE_EQ(map, nullptr);
        },


        //new value setter asserts
        [&new_map](const sc::opt & opts) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(opts.map, new_map);
            #endif
        },


        //new value getter asserts
        [&new_map](const sc::opt & opts, const mc_vm_map * map) {
            REQUIRE_EQ(map, new_map);
        }
    );

    return;
}


//`addr_width` setter & getter
TEST_CASE(test_cc_opt_subtests[5]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt - addr_width");
    #endif

    //run test helper
    _class_helper::cc::test_enm_setter_getter<sc::opt, sc::addr_width>(

        //provide test helper requirements
        sc::val_unset::addr_width, sc::AW64,
        &sc::opt::set_addr_width,
        &sc::opt::get_addr_width,

        //new value setter asserts
        [](const sc::opt & opts) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(opts.addr_width, sc::AW64);
            #endif
        }
    );

    return;
}


//`scan_set` setter & getter
TEST_CASE(test_cc_opt_subtests[6]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt - scan_set");
    #endif

    sc::map_area_set new_ma_set;
    #ifdef SC_DEBUG
    _class_helper::rbt::setup_stub(new_ma_set.set);
    #endif


    //run test helper
    _class_helper::cc::test_obj_setter_getter<
        sc::opt, sc::map_area_set> (

        //provide test helper requirements
        new_ma_set,
        &sc::opt::set_scan_set,
        &sc::opt::get_scan_set,

        //default object asserts
        [](const sc::opt & opts, const sc::map_area_set & ma_set) {
            REQUIRE_EQ(ma_set.get_set().is_init, false);
        },

        //FIXME: actually properly populate the set
        
        //new object setter asserts
        [](const sc::opt & opts) {
        
            #ifdef SC_DEBUG
            REQUIRE_EQ(opts.scan_set.set.is_init, true);
            #endif
        },

        //new object getter asserts
        [](const sc::opt & opts, const sc::map_area_set & ma_set) {
            REQUIRE_EQ(ma_set.get_set().is_init, true);
        }
    );

    return;
}


//copy ctor
TEST_CASE(test_cc_opt_subtests[7]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt - copy ctor");
    #endif

    //run test helper
    _class_helper::cc::test_copy_ctor<sc::opt>(

        //source object setup
        _populate_opt,

        //copy ctor asserts
        [](const sc::opt & dst_opts, const sc::opt & src_opts) {

            //assert constructor succeeded
            REQUIRE_EQ(dst_opts._get_ctor_failed(), false);

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
                dst_opts.sessions, src_opts.sessions, _session_elem_eq);
            REQUIRE_EQ(dst_opts.map, src_opts.map);

            //miscellaneous
            REQUIRE_EQ(dst_opts.addr_width, src_opts.addr_width);
            _class_helper::rbt::assert_eq<
                cm_lst_node *, mc_vm_area *>(

                dst_opts.scan_set.set,
                src_opts.scan_set.set,
                _cm_node_key_data_eq);
            #endif
        
        },


        //post source dtor asserts
        [](const sc::opt & opts) {

            #ifdef SC_DEBUG
            //assert pathnames
            REQUIRE_NE(opts.file_pathname_out, nullptr);
            REQUIRE_NE(opts.file_pathname_in, nullptr);
            //assert memcry
            REQUIRE_EQ(opts.sessions.is_init, true);
            REQUIRE_NE(opts.map, nullptr);
            //assert miscellaneous
            REQUIRE_EQ(opts.addr_width, sc::AW64);
            REQUIRE_EQ(opts.scan_set.set.is_init, true);
            #endif
        },


        //dtor asserts
        [](const sc::opt & opts) {
            
            #ifdef SC_DEBUG
            REQUIRE_EQ(opts.sessions.is_init, false);
            REQUIRE_EQ(opts.scan_set.set.is_init, false);
            #endif
        }

    );

    return;
}


//copy assign
TEST_CASE(test_cc_opt_subtests[8]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt - copy assign");
    #endif

    //run test helper
    _class_helper::cc::test_copy_assign<sc::opt>(

        //source object setup
        _populate_opt,

        
        //destination object setup
        [](sc::opt & opts) {

            #ifdef SC_DEBUG
            //(fixture) setup file pathnames
            _class_helper::str::setup_stub(opts.file_pathname_out);
            _class_helper::str::setup_stub(opts.file_pathname_in);
            //(fixture) setup memcry
            _class_helper::vct::setup_stub(opts.sessions);
            opts.map = (mc_vm_map *) 0x10203040;
            //(fixture) setup miscellaneous
            opts.addr_width = sc::AW64;
            _class_helper::rbt::setup_stub(opts.scan_set.set);
            #endif
        },


        //copy assign asserts
        [](const sc::opt & dst_opts, const sc::opt & src_opts) {

            //assert constructor succeeded
            REQUIRE_EQ(dst_opts._get_ctor_failed(), false);

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
                dst_opts.sessions, src_opts.sessions, _session_elem_eq);
            REQUIRE_EQ(dst_opts.map, src_opts.map);

            //miscellaneous
            REQUIRE_EQ(dst_opts.addr_width, src_opts.addr_width);
            _class_helper::rbt::assert_eq<
                cm_lst_node *, mc_vm_area *>(
                
                dst_opts.scan_set.set,
                src_opts.scan_set.set,
                _cm_node_key_data_eq);
            #endif
        }
    );

    return;
}


//reset
TEST_CASE(test_cc_opt_subtests[9]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt - reset");
    #endif

    //run test helper
    _class_helper::cc::test_reset<sc::opt>(

        //setup
        _populate_opt,


        //reset asserts
        [](const sc::opt & opts) {

            #ifdef SC_DEBUG
            //filepaths asserts
            REQUIRE_EQ(opts.file_pathname_out, nullptr);
            REQUIRE_EQ(opts.file_pathname_in, nullptr);

            //memcry asserts
            REQUIRE_EQ(opts.sessions.is_init, false);
            REQUIRE_EQ(opts.map, nullptr);

            //miscellaneous asserts
            REQUIRE_EQ(opts.addr_width, sc::val_unset::addr_width);
            REQUIRE_EQ(opts.scan_set.set.is_init, false);
            #endif
        }
    );

    return; 
}



/*
 *  --- [OPT_PTRSCAN - HELPERS] ---
 */

//compare memcry sessions (vector)
static void _off_elem_eq(const off_t & elem_0,
                         const off_t & elem_1) {

    REQUIRE_EQ(elem_0, elem_1);

    return;
}


//fully populate a `opt_ptrscan`
static void _populate_opt_ptrscan(sc::opt_ptrscan & opts_ptr) {


    int ret;

    off_t offs[4] = { 0x10, 0x20, 0x30, 0x40 };
    cm_vct new_preset_offsets;
    
    cm_lst_node * nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };
    mc_vm_area * areas[4] = {
        (mc_vm_area *) 0x50505050,
        (mc_vm_area *) 0x60606060,
        (mc_vm_area *) 0x70707070,
        (mc_vm_area *) 0x80808080
    };
    sc::map_area_set new_static_set;
    

    /*
     *  NOTE: Do not fully initialise the static set, they're
     *        too cumbersome to initialise & tested independently.
     */

    //build new sessions
    _class_helper::vct::populate(new_preset_offsets, offs, 4);

    #ifdef SC_DEBUG
    //build new scan set
    _class_helper::rbt::populate(new_static_set.set, nodes, areas, 4);
    #endif

    ret = opts_ptr.set_target_addr(0x1337);
    REQUIRE_EQ(ret, 0);

    ret = opts_ptr.set_alignment(0x10);
    REQUIRE_EQ(ret, 0);

    ret = opts_ptr.set_max_obj_sz(0x800);
    REQUIRE_EQ(ret, 0);

    ret = opts_ptr.set_max_depth(5);
    REQUIRE_EQ(ret, 0);

    ret = opts_ptr.set_static_set(new_static_set);
    REQUIRE_EQ(ret, 0);

    ret = opts_ptr.set_preset_offsets(new_preset_offsets);
    REQUIRE_EQ(ret, 0);

    ret = opts_ptr.set_smart_scan(sc::SMART_SCAN_DISABLED);
    REQUIRE_EQ(ret, 0);

    //cleanup
    cm_del_vct(&new_preset_offsets);

    return;

}



/*
 *  --- [OPT_PTRSCAN - TESTS] ---
 */

//ctor & dtor
TEST_CASE(test_cc_opt_subtests[10]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_ptrscan - ctor & dtor");
    #endif

    //run test helper
    _class_helper::cc::test_ctor_dtor<sc::opt_ptrscan>(

        //ctor asserts
        [](const sc::opt_ptrscan & opts_ptr) {

            #ifdef SC_DEBUG
            //assert primitives
            REQUIRE_EQ(opts_ptr.target_addr, 0x0);
            REQUIRE_EQ(opts_ptr.alignment,
                       sc::val_default::alignment);
            REQUIRE_EQ(opts_ptr.max_obj_sz,
                       sc::val_default::max_obj_sz);
            REQUIRE_EQ(opts_ptr.max_depth,
                       sc::val_default::max_depth);
            //assert composites
            REQUIRE_EQ(opts_ptr.static_set.set.is_init,false);
            REQUIRE_EQ(opts_ptr.preset_offsets.is_init, false);
            //assert smart scan
            REQUIRE_EQ(opts_ptr.smart_scan, sc::val_default::smart_scan);
            #endif
        },


        //fixture
        [](sc::opt_ptrscan & opts_ptr) {

            #ifdef SC_DEBUG
            //(fixture) setup primitives
            opts_ptr.target_addr = 0x1337;
            opts_ptr.alignment   = 0x10;
            opts_ptr.max_obj_sz  = 0x800;
            opts_ptr.max_depth   = 5;
            //(fixture) setup composites
            _class_helper::rbt::setup_stub(opts_ptr.static_set.set);
            _class_helper::vct::setup_stub(opts_ptr.preset_offsets);
            //(fixture) setup smart scan
            opts_ptr.smart_scan = sc::SMART_SCAN_ENABLED;
            #endif
        },


        //dtor asserts
        [](const sc::opt_ptrscan & opts_ptr) {

            #ifdef SC_DEBUG
            //assert destructors were run
            REQUIRE_EQ(opts_ptr.static_set.set.is_init, false);
            REQUIRE_EQ(opts_ptr.preset_offsets.is_init, false);
            #endif
        }
    );

    return;
}


//`target_addr` setter & getter
TEST_CASE(test_cc_opt_subtests[11]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_ptrscan - target_addr");
    #endif

    uintptr_t new_target_addr = 0x1337;
    
    //run test helper
    _class_helper::cc::test_value_setter_getter<
        sc::opt_ptrscan, uintptr_t>(

        //provide test helper requirements
        new_target_addr,
        &sc::opt_ptrscan::set_target_addr,
        &sc::opt_ptrscan::get_target_addr,

        //default getter asserts
        [](const sc::opt_ptrscan & opts_ptr, uintptr_t target_addr) {
            REQUIRE_EQ(target_addr, 0x0);
        },

        //new setter asserts
        [new_target_addr](const sc::opt_ptrscan & opts_ptr) {
            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ptr.target_addr, new_target_addr);
            #endif
        },

        //new getter asserts
        [new_target_addr](
            const sc::opt_ptrscan & opts_ptr, uintptr_t target_addr) {
            REQUIRE_EQ(target_addr, new_target_addr);
        }
    );

    return;
}


//`alignment` setter & getter
TEST_CASE(test_cc_opt_subtests[12]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_ptrscan - alignment");
    #endif

    off_t new_alignment = 0x10;
    
    //run test helper
    _class_helper::cc::test_value_setter_getter<
        sc::opt_ptrscan, off_t>(

        //provide test helper requirements
        new_alignment,
        &sc::opt_ptrscan::set_alignment,
        &sc::opt_ptrscan::get_alignment,

        //default getter asserts
        [](const sc::opt_ptrscan & opts_ptr, off_t alignment) {
            REQUIRE_EQ(alignment, sc::val_default::alignment);
        },

        //new setter asserts
        [new_alignment](const sc::opt_ptrscan & opts_ptr) {
            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ptr.alignment, new_alignment);
            #endif
        },

        //new getter asserts
        [new_alignment](
            const sc::opt_ptrscan & opts_ptr, off_t alignment) {
            REQUIRE_EQ(alignment, new_alignment);
        }
    );

    return;
}



//`max_obj_sz` setter & getter
TEST_CASE(test_cc_opt_subtests[13]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_ptrscan - max_obj_sz");
    #endif

    off_t new_max_obj_sz = 0x10;
    
    //run test helper
    _class_helper::cc::test_value_setter_getter<
        sc::opt_ptrscan, off_t>(

        //provide test helper requirements
        new_max_obj_sz,
        &sc::opt_ptrscan::set_max_obj_sz,
        &sc::opt_ptrscan::get_max_obj_sz,

        //default getter asserts
        [](const sc::opt_ptrscan & opts_ptr, off_t max_obj_sz) {
            REQUIRE_EQ(max_obj_sz, sc::val_default::max_obj_sz);
        },

        //new setter asserts
        [new_max_obj_sz](const sc::opt_ptrscan & opts_ptr) {
            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ptr.max_obj_sz, new_max_obj_sz);
            #endif
        },

        //new getter asserts
        [new_max_obj_sz](
            const sc::opt_ptrscan & opts_ptr, off_t max_obj_sz) {
            REQUIRE_EQ(max_obj_sz, new_max_obj_sz);
        }
    );

    return;
}


//`max_depth` setter & getter
TEST_CASE(test_cc_opt_subtests[14]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_ptrscan - max_depth");
    #endif

    int new_max_depth = 0x10;
    
    //run test helper
    _class_helper::cc::test_value_setter_getter<
        sc::opt_ptrscan, int>(

        //provide test helper requirements
        new_max_depth,
        &sc::opt_ptrscan::set_max_depth,
        &sc::opt_ptrscan::get_max_depth,

        //default getter asserts
        [](const sc::opt_ptrscan & opts_ptr, int max_depth) {
            REQUIRE_EQ(max_depth, sc::val_default::max_depth);
        },

        //new setter asserts
        [new_max_depth](const sc::opt_ptrscan & opts_ptr) {
            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ptr.max_depth, new_max_depth);
            #endif
        },

        //new getter asserts
        [new_max_depth](
            const sc::opt_ptrscan & opts_ptr, int max_depth) {
            REQUIRE_EQ(max_depth, new_max_depth);
        }
    );

    return;
}


//`static_set` setter & getter
TEST_CASE(test_cc_opt_subtests[15]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_ptrscan - static_set");
    #endif

    sc::map_area_set new_ma_set;
    #ifdef SC_DEBUG
    _class_helper::rbt::setup_stub(new_ma_set.set);
    #endif


    //run test helper
    _class_helper::cc::test_obj_setter_getter<
        sc::opt_ptrscan, sc::map_area_set> (

        //provide test helper requirements
        new_ma_set,
        &sc::opt_ptrscan::set_static_set,
        &sc::opt_ptrscan::get_static_set,

        //default object asserts
        [](const sc::opt_ptrscan & opts_ptr,
           const sc::map_area_set & ma_set) {
            REQUIRE_EQ(ma_set.get_set().is_init, false);
        },
        
        //new object setter asserts
        [](const sc::opt_ptrscan & opts_ptr) {
        
            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ptr.static_set.set.is_init, true);
            #endif
        },

        //new object getter asserts
        [](const sc::opt_ptrscan & opts_ptr,
           const sc::map_area_set & ma_set) {
            REQUIRE_EQ(ma_set.get_set().is_init, true);
        }
    );

    return;
}


//`preset_offsets` setter & getter
TEST_CASE(test_cc_opt_subtests[16]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_ptrscan - preset_offsets");
    #endif

    int ret;
    cm_vct new_preset_offsets;
    off_t off[4] = { 0x10, 0x20, 0x30, 0x40 };
    

    //setup new preset offsets
    _class_helper::vct::populate<off_t>(
        new_preset_offsets, off, 4);

    
    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_ptrscan, off_t>(

        //provide test helper requirements
        new_preset_offsets,
        &sc::opt_ptrscan::set_preset_offsets,
        &sc::opt_ptrscan::get_preset_offsets,

        //setter assert
        [&new_preset_offsets](const sc::opt_ptrscan & opts_ptr) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<off_t>(
                new_preset_offsets,
                opts_ptr.preset_offsets,
                _off_elem_eq);
            #endif
        },

        //element assert
        _off_elem_eq
    );

    //delete new omit areas
    cm_del_vct(&new_preset_offsets);

    return;
}


//`smart_scan` setter & getter
TEST_CASE(test_cc_opt_subtests[17]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_ptrscan - smart_scan");
    #endif

    //run test helper
    _class_helper::cc::test_enm_setter_getter<
        sc::opt_ptrscan, sc::smart_scan>(

        //provide test helper requirements
        sc::val_default::smart_scan, sc::SMART_SCAN_DISABLED,
        &sc::opt_ptrscan::set_smart_scan,
        &sc::opt_ptrscan::get_smart_scan,

        //new value setter asserts
        [](const sc::opt_ptrscan & opts_ptr) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ptr.smart_scan, sc::SMART_SCAN_DISABLED);
            #endif
        }
    );

    return;
}


//copy ctor
TEST_CASE(test_cc_opt_subtests[18]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_ptrscan - copy ctor");
    #endif

    //run test helper
    _class_helper::cc::test_copy_ctor<sc::opt_ptrscan>(

        //setup
        _populate_opt_ptrscan,


        //copy ctor asserts
        [](const sc::opt_ptrscan & dst_opts_ptr,
           const sc::opt_ptrscan & src_opts_ptr) {

            #ifdef SC_DEBUG
            int ret;
            sc::smart_scan dst_smart_scan;
            sc::smart_scan src_smart_scan;

            //assert primitives
            REQUIRE_EQ(dst_opts_ptr.target_addr, src_opts_ptr.target_addr);
            REQUIRE_EQ(dst_opts_ptr.alignment, src_opts_ptr.alignment);
            REQUIRE_EQ(dst_opts_ptr.max_obj_sz, src_opts_ptr.max_obj_sz);
            REQUIRE_EQ(dst_opts_ptr.max_depth, src_opts_ptr.max_depth);

            //assert composites
            _class_helper::rbt::assert_eq<
                cm_lst_node *, mc_vm_area *>(
                
                dst_opts_ptr.static_set.set,
                src_opts_ptr.static_set.set,
                _cm_node_key_data_eq
            );
                
            _class_helper::vct::assert_eq<off_t>(
                dst_opts_ptr.preset_offsets,
                src_opts_ptr.preset_offsets,
                _off_elem_eq
            );

            //assert smart scan
            REQUIRE_EQ(dst_opts_ptr.smart_scan, src_opts_ptr.smart_scan);
            #endif
        },

        
        //post source dtor asserts
        [](const sc::opt_ptrscan & opts_ptr) {

            #ifdef SC_DEBUG
            //assert primitives
            REQUIRE_EQ(opts_ptr.target_addr, 0x1337);
            REQUIRE_EQ(opts_ptr.alignment, 0x10);
            REQUIRE_EQ(opts_ptr.max_obj_sz, 0x800);
            REQUIRE_EQ(opts_ptr.max_depth, 5);
            //assert composites are initialised
            REQUIRE_EQ(opts_ptr.static_set.set.is_init, true);
            REQUIRE_EQ(opts_ptr.preset_offsets.is_init, true);
            //assert smart scan
            REQUIRE_EQ(opts_ptr.smart_scan, sc::SMART_SCAN_DISABLED);
            #endif
        },


        //dtor asserts
        [](const sc::opt_ptrscan & opts_ptr) {

            #ifdef SC_DEBUG
            //assert compositers are uninitialised
            REQUIRE_EQ(opts_ptr.static_set.set.is_init, false);
            REQUIRE_EQ(opts_ptr.preset_offsets.is_init, false);
            #endif
        }
    );

    return;
}


//copy assign
TEST_CASE(test_cc_opt_subtests[19]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_ptrscan - copy assign");
    #endif

    //run test helper
    _class_helper::cc::test_copy_assign<sc::opt_ptrscan>(

        //source object setup
        _populate_opt_ptrscan,


        //destination object setup
        [](sc::opt_ptrscan & opts_ptr) {

            #ifdef SC_DEBUG
            //(fixture) setup primitives
            opts_ptr.target_addr = 0x1337;
            opts_ptr.alignment   = 0x10;
            opts_ptr.max_obj_sz  = 0x800;
            opts_ptr.max_depth   = 5;
            //(fixture) setup composites
            _class_helper::rbt::setup_stub(opts_ptr.static_set.set);
            _class_helper::vct::setup_stub(opts_ptr.preset_offsets);
            //(fixture) setup smart scan
            opts_ptr.smart_scan = sc::SMART_SCAN_ENABLED;
            #endif
        },

        //copy ctor asserts
        [](const sc::opt_ptrscan & dst_opts_ptr,
           const sc::opt_ptrscan & src_opts_ptr) {

            #ifdef SC_DEBUG
            int ret;
            sc::smart_scan dst_smart_scan;
            sc::smart_scan src_smart_scan;

            //assert primitives
            REQUIRE_EQ(dst_opts_ptr.target_addr, src_opts_ptr.target_addr);
            REQUIRE_EQ(dst_opts_ptr.alignment, src_opts_ptr.alignment);
            REQUIRE_EQ(dst_opts_ptr.max_obj_sz, src_opts_ptr.max_obj_sz);
            REQUIRE_EQ(dst_opts_ptr.max_depth, src_opts_ptr.max_depth);

            //assert composites
            _class_helper::rbt::assert_eq<
                cm_lst_node *, mc_vm_area *>(
                
                dst_opts_ptr.static_set.set,
                src_opts_ptr.static_set.set,
                _cm_node_key_data_eq
            );
                
            _class_helper::vct::assert_eq<off_t>(
                dst_opts_ptr.preset_offsets,
                src_opts_ptr.preset_offsets,
                _off_elem_eq
            );

            //assert smart scan
            REQUIRE_EQ(dst_opts_ptr.smart_scan, src_opts_ptr.smart_scan);
            #endif
        }
    );

    return;
}


//reset
TEST_CASE(test_cc_opt_subtests[20]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_ptrscan - reset");
    #endif

    //run test helper
    _class_helper::cc::test_reset<sc::opt_ptrscan>(

        //setup
        _populate_opt_ptrscan,


        //reset asserts
        [](const sc::opt_ptrscan & opts_ptr) {

            #ifdef SC_DEBUG
            //assert primitives
            REQUIRE_EQ(opts_ptr.target_addr, 0x0);
            REQUIRE_EQ(opts_ptr.alignment, sc::val_default::alignment);
            REQUIRE_EQ(opts_ptr.max_obj_sz, sc::val_default::max_obj_sz);
            REQUIRE_EQ(opts_ptr.max_depth, sc::val_default::max_depth);

            //assert composites
            REQUIRE_EQ(opts_ptr.static_set.set.is_init, false);
            REQUIRE_EQ(opts_ptr.preset_offsets.is_init, false);

            //assert smart scan
            REQUIRE_EQ(opts_ptr.smart_scan, sc::val_default::smart_scan);
            #endif
        }
    );

    return; 
}
