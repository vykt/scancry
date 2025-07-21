//standard template library
#include <string>
#include <iostream>
#include <optional>

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

//test target headers
#include "../lib/scancry.h"
#include "../lib/map_area.hh"




      /* ===================== * 
 ===== *  C++ INTERFACE TESTS  * =====
       * ===================== */

/*
 *  --- [HELPERS] ---
 */

//convert a hashmap into a sorted vector for easier debugging
static std::vector<const cm_lst_node *> _hashmap_to_sorted_vector(
           const std::unordered_set<const cm_lst_node *> & area_nodes) {

    const cm_lst_node * min_area_node, * now_area_node;
    const mc_vm_area * min_area, * now_area;
    
    std::vector<const cm_lst_node *> ret_vct;

    
    //get a mutable copy of the hashmap
    std::unordered_set<const cm_lst_node *> temp_set = area_nodes;

    while (temp_set.size() != 0) {

        //treat first node as minimum to start
        auto min = temp_set.cbegin();
        min_area_node = *min;
        min_area = MC_GET_NODE_AREA(min_area_node);

        //single iteration of selection sort
        for (auto iter = ++temp_set.cbegin(); iter != temp_set.cend(); ++iter) {

            //get area of current iteration
            now_area_node = *iter;
            now_area = MC_GET_NODE_AREA(now_area_node);

            /*
             *  Area addresses can't overlap.
             */
            if (now_area->start_addr < min_area->start_addr) {
                min = iter;
                min_area_node = *min;
                min_area = MC_GET_NODE_AREA(min_area_node);
            }
        }

        //append the minimum element to the CMore vector
        ret_vct.push_back(*min);

        //remove the minimum element from the temporary set
        temp_set.erase(min);
        
    } //end while

    return ret_vct;
}

#if 0
//take a sorted vector of areas and print it
static void _cc_print_set(
    const std::string & heading,
    const std::vector<const cm_lst_node *> & sorted_area_nodes) {

    mc_vm_area * area;

    
    subtitle("set dump", heading);

    //use hex conversion
    std::cout << std::hex;

    //for every area
    for (auto iter = sorted_area_nodes.cbegin();
         iter != sorted_area_nodes.cend(); ++iter) {

        area = MC_GET_NODE_AREA((*iter));
        _memcry_helper::print_area(area);

    } //end for every area

    std::cout << std::dec;

    return;
}
#endif



/*
 *  --- [OPT_MAP_AREA - HELPERS] ---
 */

//compare cmore node pointers
static void _cm_node_elem_eq(cm_lst_node * const & elem_0,
                             cm_lst_node * const & elem_1) {

    REQUIRE_EQ(elem_0, elem_1);

    return;    
}


//compare c++ address ranges
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

    size_t old_vct_len[6];
    cm_byte old_access;


    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - copy ctor");
    #endif

    //run test helper
    _class_helper::cc::test_copy_ctor<sc::opt_map_area>(

        //source object setup
        _populate_opt_map_area,


        //copy ctor asserts 
        [&old_vct_len, &old_access](const sc::opt_map_area & dst_opts_ma,
                                    const sc::opt_map_area & src_opts_ma) {

            //save old vector lengths
            #ifdef SC_DEBUG    
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


//reset
TEST_CASE(test_cc_map_area_subtests[9]) {

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



      /* =================== * 
 ===== *  C INTERFACE TESTS  * =====
       * =================== */

/*
 *  --- [OPT_MAP_AREA - HELPERS] ---
 */



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
            REQUIRE_EQ(opts_ma.access, sc::val_unset::access);
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

    size_t old_vct_len[6];
    cm_byte old_access;


    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - copy ctor");
    #endif

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

            //save old vector lengths
            #ifdef SC_DEBUG    
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


//reset
TEST_CASE(test_c_map_area_subtests[9]) {

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
