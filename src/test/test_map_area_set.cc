//standard template library
#include <optional>
#include <vector>
#include <unordered_set>
#include <iostream> //TODO DEBUG, remove
#include <ios>
#include <cstdio>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <doctest/doctest.h>

//system headers
#include <unistd.h>

//local headers
#include "filters.hh"
#include "common.hh"
#include "target_helper.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/map_area_set.hh"



      /* ===================== * 
 ===== *  C++ INTERFACE TESTS  * =====
       * ===================== */

/*
 *  --- [HELPERS] ---
 */

//convert a hashmap into a sorted vector for easier debugging
static std::vector<cm_lst_node *> _hashmap_to_sorted_vector(
           const std::unordered_set<cm_lst_node *> & area_nodes) {

    cm_lst_node * min_area_node, * now_area_node;
    mc_vm_area * min_area, * now_area;
    
    std::vector<cm_lst_node *> ret_v;
    std::unordered_set<cm_lst_node *>::iterator min;

    
    //get a mutable copy of the hashmap
    std::unordered_set<cm_lst_node *> temp_set = area_nodes;

    while (temp_set.size() != 0) {

        //treat first node as minimum to start
        min = temp_set.begin();
        min_area_node = *min;
        min_area = MC_GET_NODE_AREA(min_area_node);

        //single iteration of selection sort
        for (auto iter = ++temp_set.begin(); iter != temp_set.end(); ++iter) {

            //get area of current iteration
            now_area_node = *iter;
            now_area = MC_GET_NODE_AREA(now_area_node);

            /* area addresses can't overlap */
            if (now_area->start_addr < min_area->start_addr) {
                min = iter;
                min_area_node = *min;
                min_area = MC_GET_NODE_AREA(min_area_node);
            }
        }

        //append the minimum element to the CMore vector
        ret_v.push_back(*min);

        //remove the minimum element from the temporary set
        temp_set.erase(min);
        
    } //end while

    return ret_v;
}


/*
 *  --- [TESTS] ---
 */

/*
 *  TODO: It is possible to automate the assertion of the returned
 *        map area set, however it is incredibly tedious and prone
 *        to breaking from changes in the environment. For now, just
 *        use a debugger to check the returned set is correct. Good
 *        tooling to do this exists in the provided gdb scripts.
 */

TEST_CASE(test_cc_map_area_set_subtests[0]) {

    int ret;
    std::optional<int> rett;
    pid_t pid;

    mc_session session;
    mc_vm_map map;

    std::vector<cm_lst_node *> sorted_area_nodes;


    /*
     *  Fixture: spawn a target process & open a MemCry session & map on it.
     */

    //clean up old targets & spawn a new target
    rett = clean_targets();
    REQUIRE_EQ(rett.has_value(), true);
    pid = start_target();
    REQUIRE_NE(pid, -1);

    //setup a MemCry session & map for the target.
    ret = mc_open(&session, PROCFS, pid);
    REQUIRE_EQ(ret, 0);

    mc_new_vm_map(&map);
    ret = mc_update_map(&session, &map);
    REQUIRE_EQ(ret, 0);

    //create a option class
    sc::opt opts = sc::opt(test_arch_byte_width);
    opts.set_map(&map);


    //test 1: simple, select the entire map
    SUBCASE(test_cc_map_area_set_subtests[1]) {

        sc::map_area_set ma_set;


        //only test: apply no constaints
        rett = ma_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes = ma_set.get_area_nodes();
        
        //a debugger will appreciate seeing a sorted area set
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes);

        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
    }


    //test 2: apply an access permission constraint
    SUBCASE(test_cc_map_area_set_subtests[2]) {

        sc::map_area_set ma_set;


        //first test: apply `rw-` access constraint
        opts.set_access(MC_ACCESS_READ | MC_ACCESS_WRITE);

        rett = ma_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_1 = ma_set.get_area_nodes();
        CHECK_EQ(area_nodes_1.size(), 8);
        
        //a debugger will appreciate seeing a sorted area set
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_1);

        
        //second test: apply `--x` access constraint
        opts.set_access(MC_ACCESS_EXEC);

        rett = ma_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_2 = ma_set.get_area_nodes();
        CHECK_EQ(area_nodes_2.size(), 4);
        
        //a debugger will appreciate seeing a sorted area set
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_2);


        //third test: apply `---s` access constraint (empty set)
        opts.set_access(MC_ACCESS_SHARED);

        rett = ma_set.update_set(opts);
        CHECK_EQ(rett.has_value(), false);
        CHECK_EQ(sc_errno, SC_ERR_SCAN_EMPTY);

        auto area_nodes_3 = ma_set.get_area_nodes();
        CHECK_EQ(area_nodes_3.size(), 0);
        
        //a debugger will appreciate seeing a sorted area set
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_3);
        
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
    }


    //test 3: apply address range constraints
    SUBCASE(test_cc_map_area_set_subtests[3]) {

        sc::map_area_set ma_set;
        mc_vm_obj * obj;


        //only test: scan only the main executable
        obj = MC_GET_NODE_OBJ(map.vm_objs.head->next);
        opts.set_addr_range(std::pair(obj->start_addr, obj->end_addr));

        rett = ma_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_1 = ma_set.get_area_nodes();
        CHECK_EQ(area_nodes_1.size(), 5);
        
        //a debugger will appreciate seeing a sorted area set
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_1);
       
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
    }


    //test 4: apply object & area constraints
    SUBCASE(test_cc_map_area_set_subtests[4]) {

        sc::map_area_set ma_set;
        mc_vm_obj * obj;
        mc_vm_area * area;


        //first test: omit objs & omit areas
        /* Omitting unit_target, ldlinux.so, & vm_area after [heap] */
        opts.set_omit_objs(std::vector({map.vm_objs.head->next,
                                        map.vm_objs.head->next->next->next->next/*->next->next*/}));
        opts.set_omit_areas(std::vector({map.vm_areas.head
                                         ->next->next->next->next->next->next}));

        rett = ma_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_1 = ma_set.get_area_nodes();
        std::cout << "area_nodes_1.size(): " << area_nodes_1.size() << std::endl;
        CHECK_EQ(area_nodes_1.size(), 11);
        
        //a debugger will appreciate seeing a sorted area set
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_1);

        opts.set_omit_objs(std::nullopt);
        opts.set_omit_areas(std::nullopt);


        //second test: exclusive objs & exclusive areas
        /* Exlusively unit_target, ldlinux.so, & vm_area after [heap] */
        opts.set_exclusive_objs(std::vector({map.vm_objs.head->next,
                                             map.vm_objs.head->next->next->next->next/*->next->next*/}));
        opts.set_exclusive_areas(std::vector({map.vm_areas.head
                                              ->next->next->next->next->next->next}));

        rett = ma_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_2 = ma_set.get_area_nodes();
        std::cout << "area_nodes_2.size(): " << area_nodes_2.size() << std::endl;
        CHECK_EQ(area_nodes_2.size(), 11);

        //a debugger will appreciate seeing a sorted area set
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_2);

        opts.set_exclusive_objs(std::nullopt);
        opts.set_exclusive_areas(std::nullopt);


        //third test: everything
        /* Exlusively unit_target */
        opts.set_exclusive_objs(std::vector({map.vm_objs.head->next}));
        /* Omit the first vm_area of unit_target */
        opts.set_omit_areas(std::vector({map.vm_areas.head}));
        /* Only target `rw-` areas */
        opts.set_access(MC_ACCESS_READ | MC_ACCESS_WRITE);
        /* Exclude the second vm_area of unit_target by address */
        area = MC_GET_NODE_AREA(map.vm_areas.head->next->next);
        obj = MC_GET_NODE_OBJ(map.vm_objs.head->next);
        opts.set_addr_range(std::pair(area->start_addr, obj->end_addr));
        
        rett = ma_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_3 = ma_set.get_area_nodes();
        std::cout << "area_nodes_3.size(): " << area_nodes_3.size() << std::endl;
        CHECK_EQ(area_nodes_3.size(), 1);
        
        //a debugger will appreciate seeing a sorted area set
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_2);

        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
    }


    //cleanup
    ret = mc_del_vm_map(&map);
    CHECK_EQ(ret, 0);

    ret = mc_close(&session);
    CHECK_EQ(ret, 0);

    return;
    
} //end TEST_CASE



      /* =================== * 
 ===== *  C INTERFACE TESTS  * =====
       * =================== */

/*
 *  --- [TESTS] ---
 */

TEST_CASE(test_c_map_area_set_subtests[0]) {

    int ret;
    std::optional<int> rett;
    pid_t pid;

    mc_session session;
    mc_vm_map map;

    std::vector<cm_lst_node *> sorted_area_nodes;
    

    /*
     *  Fixture: spawn a target process & open a MemCry session & map on it.
     */

    //clean up old targets & spawn a new target
    rett = clean_targets();
    REQUIRE_EQ(rett.has_value(), true);
    pid = start_target();
    REQUIRE_NE(pid, -1);

    //setup a MemCry session & map for the target.
    ret = mc_open(&session, PROCFS, pid);
    REQUIRE_EQ(ret, 0);

    mc_new_vm_map(&map);
    
    ret = mc_update_map(&session, &map);
    REQUIRE_EQ(ret, 0);

    //create a option class
    sc_opt opts = sc_new_opt(test_arch_byte_width);
    sc_opt_set_map(opts, &map);


    //test C wrappers work correctly
    SUBCASE(test_c_map_area_set_subtests[1]) {

        mc_vm_obj * obj;

        sc_map_area_set ma_set;
        sc_addr_range addr_range;

        cm_vct v;
    

        //create new sc_map_area_set
        ma_set = sc_new_map_area_set();
        CHECK_NE(ma_set, nullptr);
        

        //only test: executable-only scan, check update & getter 

        //exlusively scan unit_target by applying an address range constraint
        obj = MC_GET_NODE_OBJ(map.vm_objs.head->next);
        addr_range.min = obj->start_addr;
        addr_range.max = obj->end_addr;
        ret = sc_opt_set_addr_range(opts, &addr_range);
        CHECK_EQ(ret, 0);

        //update the map area set that fufills constraints
        ret = sc_update_set(ma_set, opts);
        CHECK_EQ(ret, 0);

        //get the updated set
        ret = sc_get_set(ma_set, &v);
        CHECK_EQ(v.len, 5);


        //cleanup
        cm_del_vct(&v);

        ret = sc_del_map_area_set(ma_set);
        CHECK_EQ(ret, 0);
        
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
    }


    //cleanup    
    ret = sc_del_opt(opts);
    CHECK_EQ(ret, 0);

    ret = mc_del_vm_map(&map);
    CHECK_EQ(ret, 0);

    ret = mc_close(&session);
    CHECK_EQ(ret, 0);

    return;

} //end TEST_CASE
