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
#include "target_helper.hh"
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



/*
 *  --- [TESTS] ---
 */

//`opt_map_area` ctor & dtor
TEST_CASE(test_cc_map_area_subtests[0]) {

    _class_helper::cc::test_ctor_dtor<sc::opt_map_area>(

        //ctor asserts cb
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
            REQUIRE_EQ(opts_ma.access, 0b0);
            #else
            _common::warning("opt_map_area ctor",
                             "Release build; can't assert private members.");
            #endif 
        },

        [](sc::opt_map_area & opts_ma) {

            #ifdef DEBUG

            #else

            #endif
        },

        //dtor asserts cb
        [](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            //assert vector attributes
            REQUIRE_EQ(opts_ma.omit_areas.is_init, false);
            REQUIRE_EQ(opts_ma.omit_objs.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_areas.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_objs.is_init, false);
            REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, false);
            #else
            _common::warning("opt_map_area dtor",
                             "Release build; can't assert private members.");
            #endif
        }
        
    );
    
    
}
