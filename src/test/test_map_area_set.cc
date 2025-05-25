//standard template library
#include <optional>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <iomanip>
#include <ios>

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
#include "target_helper.hh"
#include "memcry_helper.hh"

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
    char str_buf[5];


    std::cout << " --- [" << heading << "] --- " << std::endl << std::hex;

    //for every area
    for (auto iter = sorted_area_nodes.cbegin();
         iter != sorted_area_nodes.cend(); ++iter) {

        area = MC_GET_NODE_AREA((*iter));
        mc_access_to_str(area->access, str_buf);

        _memcry_helper::print_area(area);

    } //end for every area

    std::cout << std::dec;

    return;
}



/*
 *  --- [TESTS] ---
 */

/*
 *  NOTE: Due to differences in environment, we do not assert each area
 *        in the returned map area set; we only assert the size of the
 *        returned set.
 */

TEST_CASE(test_cc_map_area_set_subtests[0]) {

    int ret;

    pid_t pid;
    mc_session session;
    mc_vm_map map;

    sc::map_area_set ma_set;
    std::vector<const cm_lst_node *> sorted_area_nodes;


    /*
     *  Fixture: spawn a target process & open a MemCry session & map on it.
     */

    //clean up old targets & spawn a new target
    ret = _target_helper::clean_targets();
    REQUIRE_EQ(ret, 0);
    pid = _target_helper::start_target();
    REQUIRE_NE(pid, -1);

    //setup a MemCry session & map for the target.
    ret = mc_open(&session, PROCFS, pid);
    REQUIRE_EQ(ret, 0);

    mc_new_vm_map(&map);
    ret = mc_update_map(&session, &map);
    REQUIRE_EQ(ret, 0);

    //create an option class
    sc::opt opts = sc::opt(test_cc_addr_width);
    ret = opts.set_map(&map);
    REQUIRE_EQ(ret, 0);


    //test 1: simple, select the entire map
    SUBCASE(test_cc_map_area_set_subtests[1]) {

        //only test: apply no constaints
        ret = ma_set.update_set(opts);
        CHECK_EQ(ret, 0);

        auto area_nodes = ma_set.get_area_nodes();

        /*
         *  Here it should be sufficient to check the size of the set is 
         *  the same as the number of areas in the MemCry map. I'm well
         *  aware this check can mistakenly pass in creative ways, but it
         *  is still a good heuristic.
         */
        CHECK_EQ(area_nodes.size(), map.vm_areas.len);
        
        //convert hashmap to a sorted vector
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes);
        _cc_print_set("map_area_set: no constraints", sorted_area_nodes);

        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
    } //end test


    //test 2: apply an access permission constraint
    SUBCASE(test_cc_map_area_set_subtests[2]) {

        //first test: apply `rw-` access constraint
        ret = opts.set_access(MC_ACCESS_READ | MC_ACCESS_WRITE);
        
        ret = ma_set.update_set(opts);
        CHECK_EQ(ret, 0);

        auto area_nodes_1 = ma_set.get_area_nodes();
        CHECK_EQ(area_nodes_1.size(), 8);
        
        //convert hashmap to a sorted vector
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_1);
        _cc_print_set("map_area_set: rw- permissions", sorted_area_nodes);

        
        //second test: apply `--x` access constraint
        ret = opts.set_access(MC_ACCESS_EXEC);
        CHECK_EQ(ret, 0);

        ret = ma_set.update_set(opts);
        CHECK_EQ(ret, 0);

        auto area_nodes_2 = ma_set.get_area_nodes();
        CHECK_EQ(area_nodes_2.size(), 4);
        
        //convert hashmap to a sorted vector
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_2);
        _cc_print_set("map_area_set: --x permissions", sorted_area_nodes);


        //third test: apply `---s` access constraint (empty set)
        ret = opts.set_access(MC_ACCESS_SHARED);
        CHECK_EQ(ret, 0);

        ret = ma_set.update_set(opts);
        CHECK_EQ(ret, -1);
        CHECK_EQ(sc_errno, SC_ERR_SCAN_EMPTY);

        auto area_nodes_3 = ma_set.get_area_nodes();
        CHECK_EQ(area_nodes_3.size(), 0);
        
        //convert hashmap to a sorted vector
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_3);
        _cc_print_set(
            "map_area_set: ---s permissions (empty set)", sorted_area_nodes);
        
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
    } //end test


    //test 3: apply address range constraints
    SUBCASE(test_cc_map_area_set_subtests[3]) {
#if 0
        sc::map_area_set ma_set;

        cm_lst_node * main_node, * stack_node;
        mc_vm_obj * main_obj, * stack_obj;
        mc_vm_area * main_area, * stack_area;


        //setup
        main_node = mc_get_obj_by_basename(&map, target_name);
        CHECK_NE(main_node, nullptr);
        stack_node = mc_get_obj_by_basename(&map, "[stack]");
        CHECK_NE(stack_node, nullptr);
        
        main_obj = MC_GET_NODE_OBJ(main_node);
        stack_obj = MC_GET_NODE_OBJ(stack_node);


        //only test: scan only the main executable
        std::vector<std::pair<uintptr_t, uintptr_t>> ranges = {
            {main_obj->start_addr, main_obj->end_addr},
            {stack_obj->start_addr, main_obj->end_addr}
        };

        ret = opts.set_addr_ranges(ranges);
        CHECK_EQ(ret, 0);

        ret = ma_set.update_set(opts);
        CHECK_EQ(ret, 0);

        auto area_nodes = ma_set.get_area_nodes();
        CHECK_EQ(area_nodes.size(), 6);
        
        //convert hashmap to a sorted vector
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes);
        
        main_area = MC_GET_NODE_AREA(sorted_area_nodes[0]);
        stack_area = MC_GET_NODE_AREA(
                         sorted_area_nodes[sorted_area_nodes.size() - 1]);

        CHECK_EQ(std::strncmp(main_area->basename,
                              main_obj->basename, NAME_MAX), 0);
        CHECK_EQ(std::strncmp(stack_area->basename,
                              stack_obj->basename, NAME_MAX), 0);
        
        _cc_print_set(
            "map_area_set: main & heap address range", sorted_area_nodes);
       
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
#endif
    } //end test


    //test 4: apply object & area constraints
    SUBCASE(test_cc_map_area_set_subtests[4]) {
#if 0
        sc::map_area_set ma_set;

        cm_lst_node * main_node, * stack_obj_node, * stack_area_node, * tmp_node;
        mc_vm_obj * main_obj, * stack_obj;
        mc_vm_area * stack_area, * tmp_area;


        //setup
        main_node = mc_get_obj_by_basename(&map, target_name);
        CHECK_NE(main_node, nullptr);
        stack_obj_node = mc_get_obj_by_basename(&map, "[stack]");
        CHECK_NE(stack_obj_node, nullptr);

        main_obj = MC_GET_NODE_OBJ(main_node);
        stack_obj = MC_GET_NODE_OBJ(stack_obj_node);
        stack_area_node = MC_GET_NODE_PTR(stack_obj->vm_area_node_ps.head);
        stack_area = MC_GET_NODE_AREA(stack_area_node);


        //first test: omit objs & omit areas        
        ret = opts.set_omit_objs(
            std::vector<const cm_lst_node *>({main_node}));
        CHECK_EQ(ret, 0);
        ret = opts.set_omit_areas(
            std::vector<const cm_lst_node *>({stack_area_node}));
        CHECK_EQ(ret, 0);

        ret = ma_set.update_set(opts);
        CHECK_EQ(ret, 0);

        auto area_nodes_1 = ma_set.get_area_nodes();
        CHECK_EQ(area_nodes_1.size(), map.vm_areas.len - 6);
        
        //convert hashmap to a sorted vector
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_1);
        _cc_print_set(
            "map_area_set: omit main & [heap]", sorted_area_nodes);

        ret = opts.set_omit_objs(std::nullopt);
        CHECK_EQ(ret, 0);
        ret = opts.set_omit_areas(std::nullopt);
        CHECK_EQ(ret, 0);


        //second test: exclusive objs & exclusive areas
        ret = opts.set_exclusive_objs(
            std::vector<const cm_lst_node *>({main_node}));
        CHECK_EQ(ret, 0);
        ret = opts.set_exclusive_areas(
            std::vector<const cm_lst_node *>({stack_area_node}));
        CHECK_EQ(ret, 0);

        ret = ma_set.update_set(opts);
        CHECK_EQ(ret, 0);

        auto area_nodes_2 = ma_set.get_area_nodes();
        CHECK_EQ(area_nodes_2.size(), 6);

        //convert hashmap to a sorted vector
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_2);
        _cc_print_set(
            "map_area_set: exclusive main & [heap]", sorted_area_nodes);

        ret = opts.set_exclusive_objs(std::nullopt);
        CHECK_EQ(ret, 0);
        ret = opts.set_exclusive_areas(std::nullopt);
        CHECK_EQ(ret, 0);


        //third test: everything

        //exclusively scan main
        ret = opts.set_exclusive_objs(
            std::vector<const cm_lst_node *>({map.vm_objs.head->next}));
        CHECK_EQ(ret, 0);

        //omit the first area of main
        ret = opts.set_omit_areas(
            std::vector<const cm_lst_node *>({map.vm_areas.head}));
        CHECK_EQ(ret, 0);
        
        //only target `rw-` segments
        ret = opts.set_access(MC_ACCESS_READ | MC_ACCESS_WRITE);
        CHECK_EQ(ret, 0);

        //exclude second area of main by address
        tmp_node = map.vm_areas.head->next->next;
        tmp_area = MC_GET_NODE_AREA(tmp_node);
        std::vector<std::pair<uintptr_t, uintptr_t>> ar_vct = {
            {tmp_area->start_addr, tmp_area->end_addr}
        };
        ret = opts.set_addr_ranges(ar_vct);
        CHECK_EQ(ret, 0);

        ret = ma_set.update_set(opts);
        CHECK_EQ(ret, 0);

        auto area_nodes_3 = ma_set.get_area_nodes();
        CHECK_EQ(area_nodes_3.size(), 1);
        
        //convert hashmap to a sorted vector
        sorted_area_nodes = _hashmap_to_sorted_vector(area_nodes_3);
        _cc_print_set(
            "map_area_set: all constraints (expect 1 area)", sorted_area_nodes);

        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
#endif
    } //end test


    //test 4: apply object & area constraints
    SUBCASE(test_cc_map_area_set_subtests[4]) {

        /*
         *  TODO: Implement.
         */       
        
    } //end test


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
 *  --- [HELPERS] ---
 */

//take a sorted vector of areas and print it
static void _c_print_set(
    const std::string & heading,
    const cm_vct * sorted_area_nodes) {

    int ret;

    cm_lst_node * area_node;
    mc_vm_area * area;
    char str_buf[5];


    std::cout << " --- [" << heading << "] --- " << std::endl << std::hex;

    //for every area
    for (int i = 0; i < sorted_area_nodes->len; ++i) {

        ret = cm_vct_get(sorted_area_nodes, i, &area_node);
        CHECK_EQ(ret, 0);
        area = MC_GET_NODE_AREA(area_node);
        mc_access_to_str(area->access, str_buf);

        /*
         *  Format: <start_addr> - <end_addr> - <perms> - <basename:12>
         */
        std::cout << std::hex;
        std::cout << "0x" << area->start_addr << " - 0x" << area->end_addr;
        std::cout << " | " << std::string(str_buf);
        std::cout << " | " << std::left << std::setw(10);
        std::cout << std::string(area->basename).substr(0, 12) << std::endl; 

    } //end for every area

    std::cout << std::dec;
    
    return;
}



/*
 *  --- [TESTS] ---
 */

TEST_CASE(test_c_map_area_set_subtests[0]) {

    int ret;
    pid_t pid;

    mc_session session;
    mc_vm_map map;

    std::vector<cm_lst_node *> sorted_area_nodes;
    

    /*
     *  Fixture: spawn a target process & open a MemCry session & map on it.
     */

    //clean up old targets & spawn a new target
    ret = _target_helper::clean_targets();
    REQUIRE_EQ(ret, 0);
    pid = _target_helper::start_target();
    REQUIRE_NE(pid, -1);

    //setup a MemCry session & map for the target.
    ret = mc_open(&session, PROCFS, pid);
    REQUIRE_EQ(ret, 0);

    mc_new_vm_map(&map);
    
    ret = mc_update_map(&session, &map);
    REQUIRE_EQ(ret, 0);

    //create a option class
    sc_opt opts = sc_new_opt(test_c_addr_width);
    REQUIRE_NE(opts, SC_BAD_OBJ);

    ret = sc_opt_set_map(opts, &map);
    REQUIRE_EQ(ret, 0);


    //test C wrappers work correctly
    SUBCASE(test_c_map_area_set_subtests[1]) {

        mc_vm_obj * obj;
        sc_map_area_set ma_set;
        cm_vct set_vct;
    

        //create new sc_map_area_set
        ma_set = sc_new_map_area_set();
        CHECK_NE(ma_set, nullptr);
        

        //only test: executable-only scan, check update & getter 
        ret = sc_opt_set_access(opts, MC_ACCESS_EXEC);
        CHECK_EQ(ret, 0);

        //update the map area set that fufills constraints
        ret = sc_update_set(ma_set, opts);
        CHECK_EQ(ret, 0);

        //get the updated set
        ret = sc_get_set(ma_set, &set_vct);
        CHECK_EQ(ret, 0);

        _c_print_set(
            "map_area_set: C wrapper --x", &set_vct);

        //cleanup
        cm_del_vct(&set_vct);

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
