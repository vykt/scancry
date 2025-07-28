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

//system headers
#include <unistd.h>

//local headers
#include "filters.hh"
#include "common.hh"
#include "class_helper.hh"
#include "memcry_helper.hh"
#include "opt_helper.hh"
#include "target_helper.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/map_area.hh"




      /* ===================== * 
 ===== *  C++ INTERFACE TESTS  * =====
       * ===================== */

/*
 *  --- [OPT_MAP_AREA - HELPERS] ---
 */

//compare cmore node pointers (vector)
static void _cm_node_elem_eq(cm_lst_node * const & elem_0,
                             cm_lst_node * const & elem_1) {

    REQUIRE_EQ(elem_0, elem_1);

    return;    
}


//compare c++ address ranges (vector)
static void _cc_addr_range_elem_eq(const sc::addr_range & addr_range_0,
                                   const sc::addr_range & addr_range_1) {

    REQUIRE_EQ(addr_range_0.get_start_addr(),
               addr_range_1.get_start_addr());

    REQUIRE_EQ(addr_range_0.get_end_addr(),
               addr_range_1.get_end_addr());

    return;
}


//fully populate a `opt_map_area`
static void _populate_opt_map_area(sc::opt_map_area & opts_ma) {

    int ret;

    cm_vct new_nodes;
    cm_vct new_addr_ranges;
    cm_byte new_access = 0b111;

    cm_lst_node * nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };
    sc::addr_range addr_ranges[4] = {
        sc::addr_range(0x1000, 0x2000),
        sc::addr_range(0x2000, 0x3000),
        sc::addr_range(0x3000, 0x4000),
        sc::addr_range(0x4000, 0x5000)
    };



    //setup new vectors
    _class_helper::vct::populate<cm_lst_node *>(
        new_nodes, nodes, 4);
    _class_helper::vct::populate<sc::addr_range>(
        new_addr_ranges, addr_ranges, 4);


    //call setters
    ret = opts_ma.set_omit_areas(new_nodes);
    REQUIRE_EQ(ret, 0);

    ret = opts_ma.set_omit_objs(new_nodes);
    REQUIRE_EQ(ret, 0);
    
    ret = opts_ma.set_exclusive_areas(new_nodes);
    REQUIRE_EQ(ret, 0);

    ret = opts_ma.set_exclusive_objs(new_nodes);
    REQUIRE_EQ(ret, 0);

    ret = opts_ma.set_omit_addr_ranges(new_addr_ranges);
    REQUIRE_EQ(ret, 0);

    ret = opts_ma.set_exclusive_addr_ranges(new_addr_ranges);
    REQUIRE_EQ(ret, 0);

    ret = opts_ma.set_access(new_access);
    REQUIRE_EQ(ret, 0);

    //cleanup
    cm_del_vct(&new_nodes);
    cm_del_vct(&new_addr_ranges);

    return;
}



/*
 *  --- [OPT_MAP_AREA - TESTS] ---
 */

//ctor & dtor
TEST_CASE(test_cc_map_area_subtests[0]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - ctor & dtor");
    #endif

    //run test helper
    _class_helper::cc::test_ctor_dtor<sc::opt_map_area>(

        //ctor asserts
        [](const sc::opt_map_area & opts_ma) {

            //assert constructor succeeded
            REQUIRE_EQ(opts_ma._get_ctor_failed(), false);

            #ifdef SC_DEBUG
            //assert vector attributes
            REQUIRE_EQ(opts_ma.omit_areas.is_init, false);
            REQUIRE_EQ(opts_ma.omit_objs.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_areas.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_objs.is_init, false);
            REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, false);

            //assert access
            REQUIRE_EQ(opts_ma.access, sc::val_unset::access);
            #endif
        },

        //fixture
        [](sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            //(fixture) setup vectors
            _class_helper::vct::setup_stub(opts_ma.omit_areas);
            _class_helper::vct::setup_stub(opts_ma.omit_objs);
            _class_helper::vct::setup_stub(opts_ma.exclusive_areas);
            _class_helper::vct::setup_stub(opts_ma.exclusive_objs);
            _class_helper::vct::setup_stub(opts_ma.omit_addr_ranges);
            _class_helper::vct::setup_stub(opts_ma.exclusive_addr_ranges);
            #endif
        },

        //dtor asserts
        [](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            //assert vector attributes
            REQUIRE_EQ(opts_ma.omit_areas.is_init, false);
            REQUIRE_EQ(opts_ma.omit_objs.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_areas.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_objs.is_init, false);
            REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, false);
            #endif
        }        
    );

    return;
}


//`omit_areas` setter & getter
TEST_CASE(test_cc_map_area_subtests[1]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - omit_areas");
    #endif

    int ret;
    cm_vct new_omit_areas;
    cm_lst_node * nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };


    //setup new omit areas
    _class_helper::vct::populate<cm_lst_node *>(
        new_omit_areas, nodes, 4);


    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        new_omit_areas,
        &sc::opt_map_area::set_omit_areas,
        &sc::opt_map_area::get_omit_areas,

        //setter assert
        [&new_omit_areas](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<cm_lst_node *>(
                new_omit_areas, opts_ma.omit_areas,
                [](cm_lst_node * const & node_0,
                   cm_lst_node * const & node_1) {
                    REQUIRE_EQ(node_0, node_1);                    
            });
            #endif
        },

        //element assert
        [](cm_lst_node * const & node_0, cm_lst_node * const & node_1) {

            REQUIRE_EQ(node_0, node_1);
        }
    );

    //delete new omit areas
    cm_del_vct(&new_omit_areas);

    return;
}


//`omit_objs` setter & getter
TEST_CASE(test_cc_map_area_subtests[2]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - omit_objs");
    #endif

    int ret;
    cm_vct new_omit_objs;
    cm_lst_node * nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };


    //setup new omit areas
    _class_helper::vct::populate<cm_lst_node *>(
        new_omit_objs, nodes, 4);

    
    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        new_omit_objs,
        &sc::opt_map_area::set_omit_objs,
        &sc::opt_map_area::get_omit_objs,

        //setter assert
        [&new_omit_objs](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<cm_lst_node *>(
                new_omit_objs, opts_ma.omit_objs,
                [](cm_lst_node * const & node_0,
                   cm_lst_node * const & node_1) {
                    REQUIRE_EQ(node_0, node_1);                    
            });
            #endif
        },

        //element assert
        [](cm_lst_node * const & node_0, cm_lst_node * const & node_1) {

            REQUIRE_EQ(node_0, node_1);
        }
    );

    //delete new omit obj
    cm_del_vct(&new_omit_objs);

    return;
}


//`exclusive_areas` setter & getter
TEST_CASE(test_cc_map_area_subtests[3]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - exclusive_areas");
    #endif

    int ret;
    cm_vct new_exclusive_areas;
    cm_lst_node * nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };


    //setup new omit areas
    _class_helper::vct::populate<cm_lst_node *>(
        new_exclusive_areas, nodes, 4);

    
    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        new_exclusive_areas,
        &sc::opt_map_area::set_exclusive_areas,
        &sc::opt_map_area::get_exclusive_areas,

        //setter assert
        [&new_exclusive_areas](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<cm_lst_node *>(
                new_exclusive_areas, opts_ma.exclusive_areas,
                [](cm_lst_node * const & node_0,
                   cm_lst_node * const & node_1) {
                    REQUIRE_EQ(node_0, node_1);                    
            });
            #endif
        },

        //element assert
        [](cm_lst_node * const & node_0, cm_lst_node * const & node_1) {

            REQUIRE_EQ(node_0, node_1);
        }
    );

    //delete new omit obj
    cm_del_vct(&new_exclusive_areas);

    return;
}


//`exclusive_objs` setter & getter
TEST_CASE(test_cc_map_area_subtests[4]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - exclusive_objs");
    #endif

    int ret;
    cm_vct new_exclusive_objs;
    cm_lst_node * nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };


    //setup new omit objs
    _class_helper::vct::populate<cm_lst_node *>(
        new_exclusive_objs, nodes, 4);

    
    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        new_exclusive_objs,
        &sc::opt_map_area::set_exclusive_objs,
        &sc::opt_map_area::get_exclusive_objs,

        //setter assert
        [&new_exclusive_objs](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<cm_lst_node *>(
                new_exclusive_objs, opts_ma.exclusive_objs,
                [](cm_lst_node * const & node_0,
                   cm_lst_node * const & node_1) {
                    REQUIRE_EQ(node_0, node_1);                    
            });
            #endif
        },

        //element assert
        [](cm_lst_node * const & node_0, cm_lst_node * const & node_1) {

            REQUIRE_EQ(node_0, node_1);
        }
    );

    //delete new omit obj
    cm_del_vct(&new_exclusive_objs);

    return;
}


//`omit_addr_ranges` setter & getter
TEST_CASE(test_cc_map_area_subtests[5]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - omit_addr_ranges");
    #endif

    int ret;
    cm_vct new_omit_addr_ranges;
    sc::addr_range addr_ranges[4] = {
        sc::addr_range(0x1000, 0x2000),
        sc::addr_range(0x2000, 0x3000),
        sc::addr_range(0x3000, 0x4000),
        sc::addr_range(0x4000, 0x5000)
    };


    //setup new omit addr ranges
    _class_helper::vct::populate<sc::addr_range>(
        new_omit_addr_ranges, addr_ranges, 4);

    
    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_map_area, sc::addr_range>(

        //provide test helper requirements
        new_omit_addr_ranges,
        &sc::opt_map_area::set_omit_addr_ranges,
        &sc::opt_map_area::get_omit_addr_ranges,

        //setter assert
        [&new_omit_addr_ranges](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<sc::addr_range>(
                new_omit_addr_ranges, opts_ma.omit_addr_ranges,
                [](const sc::addr_range & addr_range_0,
                   const sc::addr_range & addr_range_1) {
                    REQUIRE_EQ(addr_range_0.start_addr,
                               addr_range_1.start_addr);
                    REQUIRE_EQ(addr_range_0.end_addr,
                               addr_range_1.end_addr);
            });
            #endif
        },

        //element assert
        [](const sc::addr_range & addr_range_0,
           const sc::addr_range & addr_range_1) {

            REQUIRE_EQ(addr_range_0.get_start_addr(),
                       addr_range_1.get_start_addr());
            REQUIRE_EQ(addr_range_0.get_end_addr(),
                       addr_range_1.get_end_addr());
        }
    );

    //delete new omit addr ranges
    cm_del_vct(&new_omit_addr_ranges);

    return;
}


//`exclusive_addr_ranges` setter & getter
TEST_CASE(test_cc_map_area_subtests[6]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - exclusive_addr_ranges");
    #endif

    int ret;
    cm_vct new_exclusive_addr_ranges;
    sc::addr_range addr_ranges[4] = {
        sc::addr_range(0x1000, 0x2000),
        sc::addr_range(0x2000, 0x3000),
        sc::addr_range(0x3000, 0x4000),
        sc::addr_range(0x4000, 0x5000)
    };


    //setup new exclusive addr ranges
    _class_helper::vct::populate<sc::addr_range>(
        new_exclusive_addr_ranges, addr_ranges, 4);

    
    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_map_area, sc::addr_range>(

        //provide test helper requirements
        new_exclusive_addr_ranges,
        &sc::opt_map_area::set_exclusive_addr_ranges,
        &sc::opt_map_area::get_exclusive_addr_ranges,

        //setter assert
        [&new_exclusive_addr_ranges](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<sc::addr_range>(
                new_exclusive_addr_ranges, opts_ma.exclusive_addr_ranges,
                [](const sc::addr_range & addr_range_0,
                   const sc::addr_range & addr_range_1) {
                    REQUIRE_EQ(addr_range_0.start_addr,
                               addr_range_1.start_addr);
                    REQUIRE_EQ(addr_range_0.end_addr,
                               addr_range_1.end_addr);
            });
            #endif
        },

        //element assert
        [](const sc::addr_range & addr_range_0,
           const sc::addr_range & addr_range_1) {

            REQUIRE_EQ(addr_range_0.get_start_addr(),
                       addr_range_1.get_start_addr());
            REQUIRE_EQ(addr_range_0.get_end_addr(),
                       addr_range_1.get_end_addr());
        }
    );

    //delete new exclusive addr ranges
    cm_del_vct(&new_exclusive_addr_ranges);

    return;
}


//`access` setter & getter
TEST_CASE(test_cc_map_area_subtests[7]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - access");
    #endif

    cm_byte new_access = 0b111;

    
    //run test helper
    _class_helper::cc::test_value_setter_getter<
        sc::opt_map_area, cm_byte>(new_access,
        &sc::opt_map_area::set_access, &sc::opt_map_area::get_access,

        //default value asserts
        [](const sc::opt_map_area & opts_ma, const cm_byte access) {

            REQUIRE_EQ(access, sc::val_unset::access);
        },


        //new value setter asserts
        [new_access](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ma.access, new_access);
            #endif
        },


        //new value getter asserts
        [new_access](const sc::opt_map_area & opts_ma,
                     const cm_byte access) {

            REQUIRE_EQ(access, new_access);
        }
    );
    
    return;
}


//copy ctor
TEST_CASE(test_cc_map_area_subtests[8]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - copy ctor");
    #endif

    size_t old_vct_len[6];
    cm_byte old_access;


    //run test helper
    _class_helper::cc::test_copy_ctor<sc::opt_map_area>(

        //source object setup
        _populate_opt_map_area,


        //copy ctor asserts 
        [&old_vct_len, &old_access](const sc::opt_map_area & dst_opts_ma,
                                    const sc::opt_map_area & src_opts_ma) {

            //assert the constructor succeeded
            REQUIRE_EQ(dst_opts_ma._get_ctor_failed(), false);

            #ifdef SC_DEBUG    
            //save old vector lengths
            old_vct_len[0] = dst_opts_ma.omit_areas.len;
            old_vct_len[1] = dst_opts_ma.omit_objs.len;
            old_vct_len[2] = dst_opts_ma.exclusive_areas.len;
            old_vct_len[3] = dst_opts_ma.exclusive_objs.len;
            old_vct_len[4] = dst_opts_ma.omit_addr_ranges.len;
            old_vct_len[5] = dst_opts_ma.exclusive_addr_ranges.len;
            old_access = dst_opts_ma.access;

            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.omit_areas,
                src_opts_ma.omit_areas,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.omit_objs,
                src_opts_ma.omit_objs,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.exclusive_areas,
                src_opts_ma.exclusive_areas,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.exclusive_objs,
                src_opts_ma.exclusive_objs,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<sc::addr_range>(
                dst_opts_ma.omit_addr_ranges,
                src_opts_ma.omit_addr_ranges,
                _cc_addr_range_elem_eq);
            _class_helper::vct::assert_eq<sc::addr_range>(
                dst_opts_ma.exclusive_addr_ranges,
                src_opts_ma.exclusive_addr_ranges,
                _cc_addr_range_elem_eq);
            REQUIRE_EQ(dst_opts_ma.access, src_opts_ma.access);
            #endif
        },


        //post source dtor asserts
        [&old_vct_len, &old_access](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ma.omit_areas.is_init, true);
            REQUIRE_EQ(opts_ma.omit_areas.len, old_vct_len[0]);

            REQUIRE_EQ(opts_ma.omit_objs.is_init, true);
            REQUIRE_EQ(opts_ma.omit_objs.len, old_vct_len[1]);

            REQUIRE_EQ(opts_ma.exclusive_areas.is_init, true);      
            REQUIRE_EQ(opts_ma.exclusive_areas.len, old_vct_len[2]);

            REQUIRE_EQ(opts_ma.exclusive_objs.is_init, true);            
            REQUIRE_EQ(opts_ma.exclusive_objs.len, old_vct_len[3]);

            REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, true);
            REQUIRE_EQ(opts_ma.omit_addr_ranges.len, old_vct_len[4]);

            REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, true);
            REQUIRE_EQ(opts_ma.exclusive_addr_ranges.len, old_vct_len[5]);

            REQUIRE_EQ(opts_ma.access, old_access);
            #endif
        },

        //dtor asserts
        [](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ma.omit_areas.is_init, false);
            REQUIRE_EQ(opts_ma.omit_objs.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_areas.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_objs.is_init, false);
            REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, false);
            #endif
        }        
    );

    return;
}


//copy assignment
TEST_CASE(test_cc_map_area_subtests[9]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - copy assign");
    #endif

    //run test helper
    _class_helper::cc::test_copy_assign<sc::opt_map_area>(

        //source object setup
        _populate_opt_map_area,


        //destination object setup
        [](sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            //(fixture) setup vectors
            _class_helper::vct::setup_stub(opts_ma.omit_areas);
            _class_helper::vct::setup_stub(opts_ma.omit_objs);
            _class_helper::vct::setup_stub(opts_ma.exclusive_areas);
            _class_helper::vct::setup_stub(opts_ma.exclusive_objs);
            _class_helper::vct::setup_stub(opts_ma.omit_addr_ranges);
            _class_helper::vct::setup_stub(opts_ma.exclusive_addr_ranges);
            #endif
        },


        //post copy assignment asserts
        [](const sc::opt_map_area & dst_opts_ma,
           const sc::opt_map_area & src_opts_ma) {

            //assert the constructor succeeded
            REQUIRE_EQ(dst_opts_ma._get_ctor_failed(), false);

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.omit_areas,
                src_opts_ma.omit_areas,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.omit_objs,
                src_opts_ma.omit_objs,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.exclusive_areas,
                src_opts_ma.exclusive_areas,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.exclusive_objs,
                src_opts_ma.exclusive_objs,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<sc::addr_range>(
                dst_opts_ma.omit_addr_ranges,
                src_opts_ma.omit_addr_ranges,
                _cc_addr_range_elem_eq);
            _class_helper::vct::assert_eq<sc::addr_range>(
                dst_opts_ma.exclusive_addr_ranges,
                src_opts_ma.exclusive_addr_ranges,
                _cc_addr_range_elem_eq);
            REQUIRE_EQ(dst_opts_ma.access, src_opts_ma.access);
            #endif
        }
    );

    return;
}


//reset
TEST_CASE(test_cc_map_area_subtests[10]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - reset");
    #endif

    //run test helper
    _class_helper::cc::test_reset<sc::opt_map_area>(

        //setup
        _populate_opt_map_area,


        //reset asserts
        [](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ma.omit_areas.is_init, false);
            REQUIRE_EQ(opts_ma.omit_objs.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_areas.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_objs.is_init, false);
            REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, false);
            REQUIRE_EQ(opts_ma.access, sc::val_unset::access);
            #endif
        }
    );

    return;
}



/*
 *  --- [MAP_AREA_SET - HELPERS] ---
 */

//compare cmore node pointers (red-black tree)
static void _cm_node_key_data_eq(cm_lst_node * const & elem_0,
                                 cm_lst_node * const & elem_1) {

    REQUIRE_EQ(elem_0, elem_1);

    return;    
}


/*
 *  --- [MAP_AREA_SET - TESTS] ---
 */

//ctor & dtor
TEST_CASE(test_cc_map_area_subtests[11]) {

    #ifndef SC_DEBUG
    _common::release_warning("map_area_set - ctor & dtor");
    #endif

    //run test helper
    _class_helper::cc::test_ctor_dtor<sc::map_area_set>(

        //post-constructor assertions
        [](const sc::map_area_set & ma_set) {

            //assert the constructor succeeded
            REQUIRE_EQ(ma_set._get_ctor_failed(), false);

            #ifdef SC_DEBUG
            REQUIRE_EQ(ma_set.set.is_init, false);
            #endif
        },


        //fixture
        [](sc::map_area_set & ma_set) {

            #ifdef SC_DEBUG
            _class_helper::rbt::setup_stub(ma_set.set);
            #endif
        },


        //post-destructor assertions
        [](const sc::map_area_set & ma_set) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(ma_set.set.is_init, false);
            #endif
        }
    );

    return;
}


//produce a scan set
TEST_CASE(test_cc_map_area_subtests[12]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::cc::args opt_args;

    sc::map_area_set ma_set;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);


    //no constraints
    SUBCASE("no constraints") {

        //setup map area options
        _opt_helper::cc::setup(opt_args, mcry_args, [](auto & args){});

        //update the set
        ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
        REQUIRE_EQ(ret, 0);

        //dump the set
        _common::title(_common::CC, "update_set", "no constraints");
        const char * explanation
         = "\nFor this test, expect the complete map of the target\n"
           "process minus any blacklisted areas (like `[vvar]`)";
        std::cout << explanation << std::endl;
        _common::subtitle("update_set - no constraints", "map dump:");
        _class_helper::ma_set::print_set(ma_set);
    }


    //all constraints
    SUBCASE("all constraints") {

        //setup map area options
        _opt_helper::cc::setup(opt_args, mcry_args,

            /*
             *  NOTE: Setting up constraints for this test is very
             *        tedious. As a result, this test attempts to
             *        to achieve complete code coverage from a
             *        single set of constraints. This is a futile
             *        effort, but it will do for now.
             */
            
            [&mcry_args](_opt_helper::cc::args & args) {

                int ret;

                mc_vm_map * m = &mcry_args.map;
                cm_lst_node * node;
                mc_vm_area * area;
                mc_vm_obj * obj;
                uintptr_t start_addr, end_addr;
                sc::addr_range range(0x0, 0x0);

                cm_vct omit_areas;
                cm_vct omit_objs;
                cm_vct exclusive_areas;
                cm_vct exclusive_objs;
                cm_vct omit_addr_ranges;
                cm_vct exclusive_addr_ranges;
                cm_byte access = MC_ACCESS_READ | MC_ACCESS_WRITE;


                //initialise constraint vectors
                ret = cm_new_vct(&omit_areas, sizeof(cm_lst_node *));
                REQUIRE_EQ(ret, 0);
                ret = cm_new_vct(&omit_objs, sizeof(cm_lst_node *));
                REQUIRE_EQ(ret, 0);
                ret = cm_new_vct(&exclusive_areas, sizeof(cm_lst_node *));
                REQUIRE_EQ(ret, 0);
                ret = cm_new_vct(&exclusive_objs, sizeof(cm_lst_node *));
                REQUIRE_EQ(ret, 0);
                ret = cm_new_vct(&omit_addr_ranges,
                                 sizeof(sc::addr_range));
                REQUIRE_EQ(ret, 0);
                ret = cm_new_vct(&exclusive_addr_ranges,
                                 sizeof(sc::addr_range));
                REQUIRE_EQ(ret, 0);


                //exclusive address range - range of unit_target object
                node = mc_get_obj_by_basename(
                           m, _target_helper::target_name);
                REQUIRE_NE(node, nullptr);
                obj = MC_GET_NODE_OBJ(node);
                REQUIRE_NE(obj, nullptr);

                range = sc::addr_range(obj->start_addr, obj->end_addr);
                ret = cm_vct_apd(&exclusive_addr_ranges, &range);
                REQUIRE_EQ(ret, 0);
                
                ret = args.opts_ma.set_exclusive_addr_ranges(
                          exclusive_addr_ranges);
                REQUIRE_EQ(ret, 0);


                //exclusive objects - `[heap]` & `libc.so.6`
                node = mc_get_obj_by_basename(m, "[heap]");
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&exclusive_objs, &node);
                REQUIRE_EQ(ret, 0);

                node = mc_get_obj_by_basename(m, "libc.so.6");
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&exclusive_objs, &node);
                REQUIRE_EQ(ret, 0);

                ret = args.opts_ma.set_exclusive_objs(exclusive_objs);
                REQUIRE_EQ(ret, 0);


                //exclusive areas - `pattern1.bin` & `pattern2.bin`
                node = mc_get_obj_by_basename(
                           m, _target_helper::pattern_1_basename);
                REQUIRE_NE(node, nullptr);
                obj = MC_GET_NODE_OBJ(node);
                REQUIRE_NE(obj, nullptr);
                node = MC_GET_NODE_PTR(obj->vm_area_node_ps.head);
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&exclusive_areas, &node);
                REQUIRE_EQ(ret, 0);

                node = mc_get_obj_by_basename(
                           m, _target_helper::pattern_2_basename);
                REQUIRE_NE(node, nullptr);
                obj = MC_GET_NODE_OBJ(node);
                REQUIRE_NE(obj, nullptr);
                node = MC_GET_NODE_PTR(obj->vm_area_node_ps.head);
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&exclusive_areas, &node);
                REQUIRE_EQ(ret, 0);

                ret = args.opts_ma.set_exclusive_areas(exclusive_areas);
                REQUIRE_EQ(ret, 0);


                //omit address ranges - subset of libc
                node = mc_get_obj_by_basename(m, "libc.so.6");
                REQUIRE_NE(node, nullptr);
                obj = MC_GET_NODE_OBJ(node);
                REQUIRE_NE(node, nullptr);
                
                range = sc::addr_range(obj->start_addr + 0x800,
                                       obj->end_addr - 0x800);
                ret = cm_vct_apd(&omit_addr_ranges, &range);
                REQUIRE_EQ(ret, 0);

                ret = args.opts_ma.set_omit_addr_ranges(omit_addr_ranges);
                REQUIRE_EQ(ret, 0);


                //omit objects - `pattern2.bin`
                node = mc_get_obj_by_basename(
                           m, _target_helper::pattern_2_basename);
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&omit_objs, &node);
                REQUIRE_EQ(ret, 0);

                ret = args.opts_ma.set_omit_objs(omit_objs);
                REQUIRE_EQ(ret, 0);


                //omit areas - 2nd & 3rd area of main executable
                node = mc_get_obj_by_basename(
                           m, _target_helper::target_name);
                REQUIRE_NE(node, nullptr);
                obj = MC_GET_NODE_OBJ(node);
                REQUIRE_NE(node, nullptr);
                
                node = MC_GET_NODE_PTR(
                           obj->vm_area_node_ps.head->next);
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&omit_areas, &node);
                REQUIRE_EQ(ret, 0);

                node = MC_GET_NODE_PTR(
                           obj->vm_area_node_ps.head->next->next);
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&omit_areas, &node);
                REQUIRE_EQ(ret, 0);

                ret = args.opts_ma.set_omit_areas(omit_areas);
                REQUIRE_EQ(ret, 0);


                //access - read & write
                ret = args.opts_ma.set_access(access);
                REQUIRE_EQ(ret, 0);


                //cleanup constraint vectors
                cm_del_vct(&omit_areas);
                cm_del_vct(&omit_objs);
                cm_del_vct(&exclusive_areas);
                cm_del_vct(&exclusive_objs);
                cm_del_vct(&omit_addr_ranges);
                cm_del_vct(&exclusive_addr_ranges);
            }
        );

        //update the set
        ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
        REQUIRE_EQ(ret, 0);

        //dump the set
        _common::title(_common::CC, "update_set", "all constraints");
        const char * explanation
         = "For this test, expect:\n"
           " > a read & write area from the main executable\n"
           " > a `[heap]` area\n"
           " > a read & write area from `libc.so.6`\n"
           "For the exact constraints used, consult the sources.";
        std::cout << explanation << std::endl;
        _common::subtitle("update_set - all constraints", "map dump:");
        _class_helper::ma_set::print_set(ma_set);
    }


    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


//copy ctor
TEST_CASE(test_cc_map_area_subtests[13]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::cc::args opt_args;

    sc::map_area_set ma_set;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup default map area options
    _opt_helper::cc::setup(opt_args, mcry_args, [](auto & args){});


    //run test helper
    _class_helper::cc::test_copy_ctor<sc::map_area_set>(

        //setup source object
        [&opt_args, &mcry_args](sc::map_area_set & ma_set) {

            int ret;

            //update the set
            ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
            REQUIRE_EQ(ret, 0);

            return;
        },


        //copy ctor asserts 
        [](const sc::map_area_set & dst_map_set,
           const sc::map_area_set & src_map_set) {

            //assert the constructor succeeded
            REQUIRE_EQ(dst_map_set._get_ctor_failed(), false);

            //assert both sets are equal
            _class_helper::rbt::assert_eq<cm_lst_node *, mc_vm_area *>(
                dst_map_set.get_set(), src_map_set.get_set(),
                [](cm_lst_node * const & node_0,
                   mc_vm_area * const & area_0,
                   cm_lst_node * const & node_1,
                   mc_vm_area * const & area_1) {

                    REQUIRE_EQ(node_0, node_1);
                    REQUIRE_EQ(area_0, area_1);
                    return;
                }
            );
        },


        //post source dtor asserts
        [](const sc::map_area_set & ma_set) {
        
            REQUIRE_EQ(ma_set.get_set().is_init, true);
            return;
        },


        //post dtor asserts
        [](const sc::map_area_set & ma_set) {
        
            REQUIRE_EQ(ma_set.get_set().is_init, false);
            return;
        }
    );


    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


//copy assign
TEST_CASE(test_cc_map_area_subtests[14]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::cc::args opt_args;

    sc::map_area_set ma_set;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup default map area options
    _opt_helper::cc::setup(opt_args, mcry_args, [](auto & args){});


    //run test helper
    _class_helper::cc::test_copy_assign<sc::map_area_set>(

        //source object setup
        [&opt_args, &mcry_args](sc::map_area_set & ma_set) {

            int ret;

            //update the set
            ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
            REQUIRE_EQ(ret, 0);

            return;
        },


        //destination object setup
        [](sc::map_area_set & ma_set) {

            #ifdef SC_DEBUG
            _class_helper::rbt::setup_stub(ma_set.set);
            #endif
        },


        //copy assign asserts 
        [](const sc::map_area_set & dst_map_set,
           const sc::map_area_set & src_map_set) {

            //assert the constructor succeeded
            REQUIRE_EQ(dst_map_set._get_ctor_failed(), false);

            //assert both sets are equal
            _class_helper::rbt::assert_eq<cm_lst_node *, mc_vm_area *>(
                dst_map_set.get_set(), src_map_set.get_set(),
                [](cm_lst_node * const & node_0,
                   mc_vm_area * const & area_0,
                   cm_lst_node * const & node_1,
                   mc_vm_area * const & area_1) {

                    REQUIRE_EQ(node_0, node_1);
                    REQUIRE_EQ(area_0, area_1);
                    return;
                }
            );
        }
    );


    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


//reset
TEST_CASE(test_cc_map_area_subtests[15]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::cc::args opt_args;

    sc::map_area_set ma_set;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup default map area options
    _opt_helper::cc::setup(opt_args, mcry_args, [](auto & args){});


    //run test helper
    _class_helper::cc::test_reset<sc::map_area_set>(

        //setup
        [&opt_args, &mcry_args](sc::map_area_set & ma_set) {

            int ret;

            //update the set
            ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
            REQUIRE_EQ(ret, 0);

            return;
        },


        //reset asserts
        [](const sc::map_area_set & ma_set) {
            
            REQUIRE_EQ(ma_set.get_set().is_init, false);
            return;
        }
    );


    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}




      /* =================== * 
 ===== *  C INTERFACE TESTS  * =====
       * =================== */

/*
 *  --- [OPT_MAP_AREA - TESTS] ---
 */

//ctor & dtor
TEST_CASE(test_c_map_area_subtests[0]) {
    
    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - ctor & dtor");
    #endif

    //run test helper
    _class_helper::c::test_ctor_dtor<
        sc_opt_map_area, sc::opt_map_area>(

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,


        //ctor asserts
        [](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            //assert vector attributes
            REQUIRE_EQ(opts_ma.omit_areas.is_init, false);
            REQUIRE_EQ(opts_ma.omit_objs.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_areas.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_objs.is_init, false);
            REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, false);

            //assert access
            REQUIRE_EQ(opts_ma.access, SC_ACCESS_UNSET);
            #endif
        }       
    );

    return;
}


//`omit_areas` setter & getter
TEST_CASE(test_c_map_area_subtests[1]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - omit_areas");
    #endif

    int ret;
    cm_vct new_omit_areas;
    cm_lst_node * nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };


    //setup new omit areas
    _class_helper::vct::populate<cm_lst_node *>(
        new_omit_areas, nodes, 4);

    
    //run test helper
    _class_helper::c::test_vct_setter_getter<
        sc_opt_map_area, sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        new_omit_areas,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_omit_areas,
        sc_opt_ma_get_omit_areas,


        //setter assert
        [&new_omit_areas](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<cm_lst_node *>(
                new_omit_areas, opts_ma.omit_areas,
                [](cm_lst_node * const & node_0,
                   cm_lst_node * const & node_1) {
                    REQUIRE_EQ(node_0, node_1);                    
            });
            #endif
        },


        //element assert
        [](cm_lst_node * const & node_0, cm_lst_node * const & node_1) {

            REQUIRE_EQ(node_0, node_1);
        }
    );

    //delete new omit areas
    cm_del_vct(&new_omit_areas);

    return;
}



//`omit_objs` setter & getter
TEST_CASE(test_c_map_area_subtests[2]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - omit_objs");
    #endif

    int ret;
    cm_vct new_omit_objs;
    cm_lst_node * nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };


    //setup new omit objs
    _class_helper::vct::populate<cm_lst_node *>(
        new_omit_objs, nodes, 4);

    
    //run test helper
    _class_helper::c::test_vct_setter_getter<
        sc_opt_map_area, sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        new_omit_objs,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_omit_objs,
        sc_opt_ma_get_omit_objs,


        //setter assert
        [&new_omit_objs](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<cm_lst_node *>(
                new_omit_objs, opts_ma.omit_objs,
                [](cm_lst_node * const & node_0,
                   cm_lst_node * const & node_1) {
                    REQUIRE_EQ(node_0, node_1);                    
            });
            #endif
        },


        //element assert
        [](cm_lst_node * const & node_0, cm_lst_node * const & node_1) {

            REQUIRE_EQ(node_0, node_1);
        }
    );

    //delete new omit objs
    cm_del_vct(&new_omit_objs);

    return;
}


//`exclusive_areas` setter & getter
TEST_CASE(test_c_map_area_subtests[3]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - exclusive_areas");
    #endif

    int ret;
    cm_vct new_exclusive_areas;
    cm_lst_node * nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };


    //setup new exclusive areas
    _class_helper::vct::populate<cm_lst_node *>(
        new_exclusive_areas, nodes, 4);

    
    //run test helper
    _class_helper::c::test_vct_setter_getter<
        sc_opt_map_area, sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        new_exclusive_areas,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_exclusive_areas,
        sc_opt_ma_get_exclusive_areas,


        //setter assert
        [&new_exclusive_areas](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<cm_lst_node *>(
                new_exclusive_areas, opts_ma.exclusive_areas,
                [](cm_lst_node * const & node_0,
                   cm_lst_node * const & node_1) {
                    REQUIRE_EQ(node_0, node_1);                    
            });
            #endif
        },

        
        //element assert
        [](cm_lst_node * const & node_0, cm_lst_node * const & node_1) {

            REQUIRE_EQ(node_0, node_1);
        }
    );

    //delete new exclusive areas
    cm_del_vct(&new_exclusive_areas);

    return;
}



//`exclusive_objs` setter & getter
TEST_CASE(test_c_map_area_subtests[4]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - exclusive_objs");
    #endif

    int ret;
    cm_vct new_exclusive_objs;
    cm_lst_node * nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };


    //setup new omit areas
    _class_helper::vct::populate<cm_lst_node *>(
        new_exclusive_objs, nodes, 4);

    
    //run test helper
    _class_helper::c::test_vct_setter_getter<
        sc_opt_map_area, sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        new_exclusive_objs,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_exclusive_objs,
        sc_opt_ma_get_exclusive_objs,


        //setter assert
        [&new_exclusive_objs](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<cm_lst_node *>(
                new_exclusive_objs, opts_ma.exclusive_objs,
                [](cm_lst_node * const & node_0,
                   cm_lst_node * const & node_1) {
                    REQUIRE_EQ(node_0, node_1);                    
            });
            #endif
        },


        //element assert
        [](cm_lst_node * const & node_0, cm_lst_node * const & node_1) {

            REQUIRE_EQ(node_0, node_1);
        }
    );

    //delete new omit areas
    cm_del_vct(&new_exclusive_objs);

    return;
}


//`omit_addr_ranges` setter & getter
TEST_CASE(test_c_map_area_subtests[5]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - omit_addr_ranges");
    #endif

    int ret;
    cm_vct new_omit_addr_ranges;
    sc_addr_range addr_ranges[4] = {
        {0x1000, 0x2000},
        {0x2000, 0x3000},
        {0x3000, 0x4000},
        {0x4000, 0x5000}
    };

    //setup new omit addr ranges
    _class_helper::vct::populate<sc_addr_range>(
        new_omit_addr_ranges, addr_ranges, 4);
    
    //run test helper
    _class_helper::c::test_vct_conv_setter_getter<
        sc_opt_map_area, sc::opt_map_area, sc_addr_range>(

        //provide test helper requirements
        new_omit_addr_ranges,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_omit_addr_ranges,
        sc_opt_ma_get_omit_addr_ranges,


        //setter assert
        [&new_omit_addr_ranges](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<sc_addr_range>(
                new_omit_addr_ranges, opts_ma.omit_addr_ranges,
                [](const sc_addr_range & addr_range_0,
                   const sc_addr_range & addr_range_1) {
                    REQUIRE_EQ(addr_range_0.start_addr,
                               addr_range_1.start_addr);
                    REQUIRE_EQ(addr_range_0.end_addr,
                               addr_range_1.end_addr);
            });
            #endif
        },


        //element assert
        [](const sc_addr_range & addr_range_0,
           const sc_addr_range & addr_range_1) {

            REQUIRE_EQ(addr_range_0.start_addr,
                       addr_range_1.start_addr);
            REQUIRE_EQ(addr_range_0.end_addr,
                       addr_range_1.end_addr);
        }
    );

    //delete new omit addr ranges
    cm_del_vct(&new_omit_addr_ranges);

    return;
}


//`exclusive_addr_ranges` setter & getter
TEST_CASE(test_c_map_area_subtests[5]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - exclusive_addr_ranges");
    #endif

    int ret;
    cm_vct new_exclusive_addr_ranges;
    sc_addr_range addr_ranges[4] = {
        {0x1000, 0x2000},
        {0x2000, 0x3000},
        {0x3000, 0x4000},
        {0x4000, 0x5000}
    };


    //setup new exclusive addr ranges
    _class_helper::vct::populate<sc_addr_range>(
        new_exclusive_addr_ranges, addr_ranges, 4);

    
    //run test helper
    _class_helper::c::test_vct_conv_setter_getter<
        sc_opt_map_area, sc::opt_map_area, sc_addr_range>(

        //provide test helper requirements
        new_exclusive_addr_ranges,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_exclusive_addr_ranges,
        sc_opt_ma_get_exclusive_addr_ranges,


        //setter assert
        [&new_exclusive_addr_ranges](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<sc_addr_range>(
                new_exclusive_addr_ranges, opts_ma.exclusive_addr_ranges,
                [](const sc_addr_range & addr_range_0,
                   const sc_addr_range & addr_range_1) {
                    REQUIRE_EQ(addr_range_0.start_addr,
                               addr_range_1.start_addr);
                    REQUIRE_EQ(addr_range_0.end_addr,
                               addr_range_1.end_addr);
            });
            #endif
        },


        //element assert
        [](const sc_addr_range & addr_range_0,
           const sc_addr_range & addr_range_1) {
            REQUIRE_EQ(addr_range_0.start_addr,
                       addr_range_1.start_addr);
            REQUIRE_EQ(addr_range_0.end_addr,
                       addr_range_1.end_addr);
        }
    );

    //delete new exclusive addr ranges
    cm_del_vct(&new_exclusive_addr_ranges);

    return;
}


//`access` setter & getter
TEST_CASE(test_c_map_area_subtests[7]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - access");
    #endif

    cm_byte new_access = 0b111;
    
    //run test helper
    _class_helper::c::test_value_setter_getter<
        sc_opt_map_area, sc::opt_map_area, cm_byte>(

        //provide test helper requirements
        new_access,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_access,
        sc_opt_ma_get_access,


        //default value asserts
        [](const sc::opt_map_area & opts_ma, const cm_byte access) {
            REQUIRE_EQ(access, sc::val_unset::access);
        },


        //new value setter asserts
        [new_access](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ma.access, new_access);
            #endif
        },


        //new value getter asserts
        [new_access](const sc::opt_map_area & opts_ma,
                     const cm_byte access) {
            REQUIRE_EQ(access, new_access);
        }
    );
    
    return;
}


//copy ctor
TEST_CASE(test_c_map_area_subtests[8]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - copy ctor");
    #endif

    size_t old_vct_len[6];
    cm_byte old_access;


    //run test helper
    _class_helper::c::test_copy_ctor<
        sc_opt_map_area, sc::opt_map_area>(

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_copy_opt_ma,


        //source object setup
        _populate_opt_map_area,


        //copy ctor asserts 
        [&old_vct_len, &old_access](const sc::opt_map_area & dst_opts_ma,
                                    const sc::opt_map_area & src_opts_ma) {

            //assert the constructor succeeded
            REQUIRE_EQ(dst_opts_ma._get_ctor_failed(), false);

            #ifdef SC_DEBUG    
            //save old vector lengths
            old_vct_len[0] = dst_opts_ma.omit_areas.len;
            old_vct_len[1] = dst_opts_ma.omit_objs.len;
            old_vct_len[2] = dst_opts_ma.exclusive_areas.len;
            old_vct_len[3] = dst_opts_ma.exclusive_objs.len;
            old_vct_len[4] = dst_opts_ma.omit_addr_ranges.len;
            old_vct_len[5] = dst_opts_ma.exclusive_addr_ranges.len;
            old_access = dst_opts_ma.access;

            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.omit_areas,
                src_opts_ma.omit_areas,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.omit_objs,
                src_opts_ma.omit_objs,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.exclusive_areas,
                src_opts_ma.exclusive_areas,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.exclusive_objs,
                src_opts_ma.exclusive_objs,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<sc::addr_range>(
                dst_opts_ma.omit_addr_ranges,
                src_opts_ma.omit_addr_ranges,
                _cc_addr_range_elem_eq);
            _class_helper::vct::assert_eq<sc::addr_range>(
                dst_opts_ma.exclusive_addr_ranges,
                src_opts_ma.exclusive_addr_ranges,
                _cc_addr_range_elem_eq);
            REQUIRE_EQ(dst_opts_ma.access, src_opts_ma.access);
            #endif
        },


        //post source dtor asserts
        [&old_vct_len, &old_access](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ma.omit_areas.is_init, true);
            REQUIRE_EQ(opts_ma.omit_areas.len, old_vct_len[0]);

            REQUIRE_EQ(opts_ma.omit_objs.is_init, true);
            REQUIRE_EQ(opts_ma.omit_objs.len, old_vct_len[1]);

            REQUIRE_EQ(opts_ma.exclusive_areas.is_init, true);      
            REQUIRE_EQ(opts_ma.exclusive_areas.len, old_vct_len[2]);

            REQUIRE_EQ(opts_ma.exclusive_objs.is_init, true);            
            REQUIRE_EQ(opts_ma.exclusive_objs.len, old_vct_len[3]);

            REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, true);
            REQUIRE_EQ(opts_ma.omit_addr_ranges.len, old_vct_len[4]);

            REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, true);
            REQUIRE_EQ(opts_ma.exclusive_addr_ranges.len, old_vct_len[5]);

            REQUIRE_EQ(opts_ma.access, old_access);
            #endif
        }
    );

    return;
}


//copy assign
TEST_CASE(test_c_map_area_subtests[9]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - copy assign");
    #endif

    //run test helper
    _class_helper::c::test_copy_assign<
        sc_opt_map_area, sc::opt_map_area>(

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_copy_assign_opt_ma,


        //source object setup
        _populate_opt_map_area,


        //destination object setup
        [](sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            //(fixture) setup vectors
            _class_helper::vct::setup_stub(opts_ma.omit_areas);
            _class_helper::vct::setup_stub(opts_ma.omit_objs);
            _class_helper::vct::setup_stub(opts_ma.exclusive_areas);
            _class_helper::vct::setup_stub(opts_ma.exclusive_objs);
            _class_helper::vct::setup_stub(opts_ma.omit_addr_ranges);
            _class_helper::vct::setup_stub(opts_ma.exclusive_addr_ranges);
            #endif
        },


        //copy ctor asserts 
        [](const sc::opt_map_area & dst_opts_ma,
           const sc::opt_map_area & src_opts_ma) {

            //assert the constructor succeeded
            REQUIRE_EQ(dst_opts_ma._get_ctor_failed(), false);

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.omit_areas,
                src_opts_ma.omit_areas,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.omit_objs,
                src_opts_ma.omit_objs,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.exclusive_areas,
                src_opts_ma.exclusive_areas,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<cm_lst_node *>(
                dst_opts_ma.exclusive_objs,
                src_opts_ma.exclusive_objs,
                _cm_node_elem_eq);
            _class_helper::vct::assert_eq<sc::addr_range>(
                dst_opts_ma.omit_addr_ranges,
                src_opts_ma.omit_addr_ranges,
                _cc_addr_range_elem_eq);
            _class_helper::vct::assert_eq<sc::addr_range>(
                dst_opts_ma.exclusive_addr_ranges,
                src_opts_ma.exclusive_addr_ranges,
                _cc_addr_range_elem_eq);
            REQUIRE_EQ(dst_opts_ma.access, src_opts_ma.access);
            #endif
        }
    );

    return;
}


//reset
TEST_CASE(test_c_map_area_subtests[10]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - reset");
    #endif

    //run test helper
    _class_helper::c::test_reset<
        sc_opt_map_area, sc::opt_map_area>(

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_reset,

        
        //setup
        _populate_opt_map_area,

        
        //reset asserts
        [](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ma.omit_areas.is_init, false);
            REQUIRE_EQ(opts_ma.omit_objs.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_areas.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_objs.is_init, false);
            REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, false);
            REQUIRE_EQ(opts_ma.access, sc::val_unset::access);
            #endif
        }
    );

    return;
}



/*
 *  --- [MAP_AREA_SET - TESTS] ---
 */

//ctor & dtor
TEST_CASE(test_c_map_area_subtests[11]) {
    
    #ifndef SC_DEBUG
    _common::release_warning(" (C) map_area_set - ctor & dtor");
    #endif

    //run test helper
    _class_helper::c::test_ctor_dtor<
        sc_map_area_set, sc::map_area_set>(

        //fn pointers
        sc_new_ma_set,
        sc_del_ma_set,


        //ctor asserts
        [](const sc::map_area_set & ma_set) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(ma_set.set.is_init, false);
            #endif
        }
    );

    return;
}


//produce a scan set
TEST_CASE(test_c_map_area_subtests[12]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::c::args opt_args;

    sc_map_area_set * ma_set;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup a map area set
    ma_set = sc_new_ma_set();
    REQUIRE_NE(ma_set, nullptr);


    //no constraints
    SUBCASE("no constraints") {

        //setup map area options
        _opt_helper::c::setup(opt_args, mcry_args, [](auto & args){});

        //update the set
        ret = sc_ma_set_update_set(
                  ma_set, opt_args.opts_ma, &mcry_args.map);
        REQUIRE_EQ(ret, 0);

        //dump the set
        _common::title(_common::C, "update_set", "no constraints");
        const char * explanation
         = "\nFor this test, expect the complete map of the target\n"
           "process minus any blacklisted areas (like `[vvar]`)";
        std::cout << explanation << std::endl;
        _common::subtitle("update_set - no constraints", "map dump:");
        _class_helper::ma_set::print_set(*(sc::map_area_set *) ma_set);

    }


    //all constraints
    SUBCASE("all constraints") {

        //setup map area options
        _opt_helper::c::setup(opt_args, mcry_args,

            /*
             *  NOTE: Setting up constraints for this test is very
             *        tedious. As a result, this test attempts to
             *        to achieve complete code coverage from a
             *        single set of constraints. This is a futile
             *        effort, but it will do for now.
             */
            
            [&mcry_args](_opt_helper::c::args & args) {

                int ret;

                mc_vm_map * m = &mcry_args.map;
                cm_lst_node * node;
                mc_vm_area * area;
                mc_vm_obj * obj;
                uintptr_t start_addr, end_addr;
                sc_addr_range range;

                cm_vct omit_areas;
                cm_vct omit_objs;
                cm_vct exclusive_areas;
                cm_vct exclusive_objs;
                cm_vct omit_addr_ranges;
                cm_vct exclusive_addr_ranges;
                cm_byte access = MC_ACCESS_READ | MC_ACCESS_WRITE;


                //initialise constraint vectors
                ret = cm_new_vct(&omit_areas, sizeof(cm_lst_node *));
                REQUIRE_EQ(ret, 0);
                ret = cm_new_vct(&omit_objs, sizeof(cm_lst_node *));
                REQUIRE_EQ(ret, 0);
                ret = cm_new_vct(&exclusive_areas, sizeof(cm_lst_node *));
                REQUIRE_EQ(ret, 0);
                ret = cm_new_vct(&exclusive_objs, sizeof(cm_lst_node *));
                REQUIRE_EQ(ret, 0);
                ret = cm_new_vct(&omit_addr_ranges,
                                 sizeof(sc_addr_range));
                REQUIRE_EQ(ret, 0);
                ret = cm_new_vct(&exclusive_addr_ranges,
                                 sizeof(sc_addr_range));
                REQUIRE_EQ(ret, 0);


                //exclusive address range - range of unit_target object
                node = mc_get_obj_by_basename(
                           m, _target_helper::target_name);
                REQUIRE_NE(node, nullptr);
                obj = MC_GET_NODE_OBJ(node);
                REQUIRE_NE(obj, nullptr);

                range = {obj->start_addr, obj->end_addr};
                ret = cm_vct_apd(&exclusive_addr_ranges, &range);
                REQUIRE_EQ(ret, 0);
                
                ret = sc_opt_ma_set_exclusive_addr_ranges(
                          args.opts_ma, &exclusive_addr_ranges);
                REQUIRE_EQ(ret, 0);


                //exclusive objects - `[heap]` & `libc.so.6`
                node = mc_get_obj_by_basename(m, "[heap]");
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&exclusive_objs, &node);
                REQUIRE_EQ(ret, 0);

                node = mc_get_obj_by_basename(m, "libc.so.6");
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&exclusive_objs, &node);
                REQUIRE_EQ(ret, 0);

                ret = sc_opt_ma_set_exclusive_objs(
                          args.opts_ma, &exclusive_objs);
                REQUIRE_EQ(ret, 0);


                //exclusive areas - `pattern1.bin` & `pattern2.bin`
                node = mc_get_obj_by_basename(
                           m, _target_helper::pattern_1_basename);
                REQUIRE_NE(node, nullptr);
                obj = MC_GET_NODE_OBJ(node);
                REQUIRE_NE(obj, nullptr);
                node = MC_GET_NODE_PTR(obj->vm_area_node_ps.head);
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&exclusive_areas, &node);
                REQUIRE_EQ(ret, 0);

                node = mc_get_obj_by_basename(
                           m, _target_helper::pattern_2_basename);
                REQUIRE_NE(node, nullptr);
                obj = MC_GET_NODE_OBJ(node);
                REQUIRE_NE(obj, nullptr);
                node = MC_GET_NODE_PTR(obj->vm_area_node_ps.head);
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&exclusive_areas, &node);
                REQUIRE_EQ(ret, 0);

                ret = sc_opt_ma_set_exclusive_areas(
                          args.opts_ma, &exclusive_areas);
                REQUIRE_EQ(ret, 0);


                //omit address ranges - subset of libc
                node = mc_get_obj_by_basename(m, "libc.so.6");
                REQUIRE_NE(node, nullptr);
                obj = MC_GET_NODE_OBJ(node);
                REQUIRE_NE(node, nullptr);
                
                range = {obj->start_addr + 0x800, obj->end_addr - 0x800};
                ret = cm_vct_apd(&omit_addr_ranges, &range);
                REQUIRE_EQ(ret, 0);

                ret = sc_opt_ma_set_omit_addr_ranges(
                          args.opts_ma, &omit_addr_ranges);
                REQUIRE_EQ(ret, 0);


                //omit objects - `pattern2.bin`
                node = mc_get_obj_by_basename(
                           m, _target_helper::pattern_2_basename);
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&omit_objs, &node);
                REQUIRE_EQ(ret, 0);

                ret = sc_opt_ma_set_omit_objs(
                          args.opts_ma, &omit_objs);
                REQUIRE_EQ(ret, 0);


                //omit areas - 2nd & 3rd area of main executable
                node = mc_get_obj_by_basename(
                           m, _target_helper::target_name);
                REQUIRE_NE(node, nullptr);
                obj = MC_GET_NODE_OBJ(node);
                REQUIRE_NE(node, nullptr);
                
                node = MC_GET_NODE_PTR(
                           obj->vm_area_node_ps.head->next);
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&omit_areas, &node);
                REQUIRE_EQ(ret, 0);

                node = MC_GET_NODE_PTR(
                           obj->vm_area_node_ps.head->next->next);
                REQUIRE_NE(node, nullptr);
                ret = cm_vct_apd(&omit_areas, &node);
                REQUIRE_EQ(ret, 0);

                ret = sc_opt_ma_set_omit_areas(
                          args.opts_ma, &omit_areas);
                REQUIRE_EQ(ret, 0);


                //access - read & write
                ret = sc_opt_ma_set_access(args.opts_ma, access);
                REQUIRE_EQ(ret, 0);


                //cleanup constraint vectors
                cm_del_vct(&omit_areas);
                cm_del_vct(&omit_objs);
                cm_del_vct(&exclusive_areas);
                cm_del_vct(&exclusive_objs);
                cm_del_vct(&omit_addr_ranges);
                cm_del_vct(&exclusive_addr_ranges);

                return;
            }
        );

        //update the set
        ret = sc_ma_set_update_set(ma_set,
                                   opt_args.opts_ma, &mcry_args.map);
        REQUIRE_EQ(ret, 0);

        //dump the set
        _common::title(_common::C, "update_set", "all constraints");
        const char * explanation
         = "For this test, expect:\n"
           " > a read & write area from the main executable\n"
           " > a `[heap]` area\n"
           " > a read & write area from `libc.so.6`\n"
           "For the exact constraints used, consult the sources.";
        std::cout << explanation << std::endl;
        _common::subtitle("update_set - all constraints", "map dump:");
        _class_helper::ma_set::print_set(*(sc::map_area_set *) ma_set);
    }

    //teardown the map area set
    sc_del_ma_set(ma_set);

    //teardown options
    _opt_helper::c::teardown(opt_args);

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


//copy ctor
TEST_CASE(test_c_map_area_subtests[13]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::c::args opt_args;

    sc_map_area_set * ma_set;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup default map area options
    _opt_helper::c::setup(opt_args, mcry_args, [](auto & args){});

    //setup a map area set
    ma_set = sc_new_ma_set();
    REQUIRE_NE(ma_set, nullptr);


    //run test helper
    _class_helper::c::test_copy_ctor<sc_map_area_set, sc::map_area_set>(

        //fn pointers
        sc_new_ma_set,
        sc_del_ma_set,
        sc_copy_ma_set,


        //setup source object
        [&opt_args, &mcry_args](sc::map_area_set & ma_set) {

            int ret;

            //update the set
            ret = ma_set.update_set(
                *(sc::opt_map_area *) opt_args.opts_ma, mcry_args.map);
            REQUIRE_EQ(ret, 0);

            return;
        },


        //copy ctor asserts 
        [](const sc::map_area_set & dst_map_set,
           const sc::map_area_set & src_map_set) {

            //assert both sets are equal
            _class_helper::rbt::assert_eq<cm_lst_node *, mc_vm_area *>(
                dst_map_set.get_set(), src_map_set.get_set(),
                [](cm_lst_node * const & node_0,
                   mc_vm_area * const & area_0,
                   cm_lst_node * const & node_1,
                   mc_vm_area * const & area_1) {

                    REQUIRE_EQ(node_0, node_1);
                    REQUIRE_EQ(area_0, area_1);
                    return;
                }
            );
        },


        //post source dtor asserts
        [](const sc::map_area_set & ma_set) {
        
            REQUIRE_EQ(ma_set.get_set().is_init, true);
            return;
        }
    );


    //teardown the map area set
    sc_del_ma_set(ma_set);

    //teardown options
    _opt_helper::c::teardown(opt_args);

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


//copy assign
TEST_CASE(test_c_map_area_subtests[14]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::c::args opt_args;

    sc_map_area_set * ma_set;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup default map area options
    _opt_helper::c::setup(opt_args, mcry_args, [](auto & args){});

    //setup a map area set
    ma_set = sc_new_ma_set();
    REQUIRE_NE(ma_set, nullptr);


    //run test helper
    _class_helper::c::test_copy_assign<sc_map_area_set, sc::map_area_set>(

        //fn pointers
        sc_new_ma_set,
        sc_del_ma_set,
        sc_copy_assign_ma_set,


        //setup source object
        [&opt_args, &mcry_args](sc::map_area_set & ma_set) {

            int ret;

            //update the set
            ret = ma_set.update_set(
                *(sc::opt_map_area *) opt_args.opts_ma, mcry_args.map);
            REQUIRE_EQ(ret, 0);

            return;
        },


        //setup destination object
        [](sc::map_area_set & ma_set) {

            #ifdef SC_DEBUG
            _class_helper::rbt::setup_stub(ma_set.set);
            #endif
        },


        //copy ctor asserts 
        [](const sc::map_area_set & dst_map_set,
           const sc::map_area_set & src_map_set) {

            //assert both sets are equal
            _class_helper::rbt::assert_eq<cm_lst_node *, mc_vm_area *>(
                dst_map_set.get_set(), src_map_set.get_set(),
                [](cm_lst_node * const & node_0,
                   mc_vm_area * const & area_0,
                   cm_lst_node * const & node_1,
                   mc_vm_area * const & area_1) {

                    REQUIRE_EQ(node_0, node_1);
                    REQUIRE_EQ(area_0, area_1);
                    return;
                }
            );
        }
    );


    //teardown the map area set
    sc_del_ma_set(ma_set);

    //teardown options
    _opt_helper::c::teardown(opt_args);

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


//reset
TEST_CASE(test_c_map_area_subtests[15]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::c::args opt_args;

    sc_map_area_set * ma_set;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup default map area options
    _opt_helper::c::setup(opt_args, mcry_args, [](auto & args){});


    //run test helper
    _class_helper::c::test_reset<sc_map_area_set, sc::map_area_set>(

        //fn pointers
        sc_new_ma_set,
        sc_del_ma_set,
        sc_ma_set_reset,


        //setup
        [&opt_args, &mcry_args](sc::map_area_set & ma_set) {

            int ret;


            //update the set
            ret = ma_set.update_set(
                *(sc::opt_map_area *) opt_args.opts_ma, mcry_args.map);
            REQUIRE_EQ(ret, 0);

            return;
        },


        //reset asserts
        [](const sc::map_area_set & ma_set) {
            
            REQUIRE_EQ(ma_set.get_set().is_init, false);
            return;
        }
    );


    //teardown options
    _opt_helper::c::teardown(opt_args);

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}

