//standard template library
#include <optional>
#include <vector>
#include <unordered_set>

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
std::vector<cm_lst_node *> hashmap_to_sorted_vector(
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
    ret = mc_update_map(&session, &map);
    REQUIRE_EQ(ret, 0);

    //create a option class
    sc::opt opts = sc::opt(test_arch_byte_width);
    opts.set_map(&map);


    //test 1: simple, select the entire map
    SUBCASE(test_cc_map_area_set_subtests[1]) {

        sc::map_area_set s_set;


        //only test: apply no constaints
        rett = s_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes = s_set.get_area_nodes();
        sorted_area_nodes = hashmap_to_sorted_vector(area_nodes);

        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
    }


    //test 2: apply an access permission constraint
    SUBCASE(test_cc_map_area_set_subtests[2]) {

        sc::map_area_set s_set;


        //first test: apply `rw-` access constraint
        opts.set_access(MC_ACCESS_READ | MC_ACCESS_WRITE);

        rett = s_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_1 = s_set.get_area_nodes();
        sorted_area_nodes = hashmap_to_sorted_vector(area_nodes_1);

        
        //second test: apply `--x` access constraint
        opts.set_access(MC_ACCESS_READ | MC_ACCESS_WRITE);

        rett = s_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_2 = s_set.get_area_nodes();
        sorted_area_nodes = hashmap_to_sorted_vector(area_nodes_2);


        //third test: apply `---s` access constraint (empty set)
        opts.set_access(MC_ACCESS_SHARED);

        rett = s_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_3 = s_set.get_area_nodes();
        sorted_area_nodes = hashmap_to_sorted_vector(area_nodes_3);
        
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
    }


    //test 3: apply address range constraints
    SUBCASE(test_cc_map_area_set_subtests[3]) {

        sc::map_area_set s_set;
        mc_vm_obj * obj;


        //only test: scan only the main executable
        obj = MC_GET_NODE_OBJ(map.vm_objs.head->next);
        opts.set_addr_range(std::pair(obj->start_addr, obj->end_addr));

        rett = s_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_1 = s_set.get_area_nodes();
        sorted_area_nodes = hashmap_to_sorted_vector(area_nodes_1);
       
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
    }


    //test 4: apply address range constraints
    SUBCASE(test_cc_map_area_set_subtests[4]) {

        sc::map_area_set s_set;


        //first test: omit objs & omit areas
        /* Omitting unit_target, ldlinux.so, & vm_area after [heap] */
        opts.set_omit_objs(std::vector({map.vm_objs.head->next,
                                        map.vm_objs.head->next->next->next}));
        opts.set_omit_areas(std::vector({map.vm_areas.head
                                         ->next->next->next->next->next}));

        rett = s_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_1 = s_set.get_area_nodes();
        sorted_area_nodes = hashmap_to_sorted_vector(area_nodes_1);


        //second test: exclusive objs & exclusive areas
        /* Exlusively unit_target, ldlinux.so, & vm_area after [heap] */
        opts.set_omit_objs(std::nullopt); //TODO FIXME KILLME
        opts.set_omit_objs(std::vector({map.vm_objs.head->next,
                                        map.vm_objs.head->next->next->next}));
        opts.set_omit_areas(std::vector({map.vm_areas.head
                                         ->next->next->next->next->next}));

        rett = s_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_2 = s_set.get_area_nodes();
        sorted_area_nodes = hashmap_to_sorted_vector(area_nodes_2);


        //third test: everything
        /* Exlusively unit_target, ldlinux.so, & vm_area after [heap] */
        opts.set_omit_objs(std::vector({map.vm_objs.head->next,
                                        map.vm_objs.head->next->next->next}));
        opts.set_omit_areas(std::vector({map.vm_areas.head
                                         ->next->next->next->next->next}));

        rett = s_set.update_set(opts);
        CHECK_EQ(rett.has_value(), true);

        auto area_nodes_2 = s_set.get_area_nodes();
        sorted_area_nodes = hashmap_to_sorted_vector(area_nodes_2);
       

        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
    }


    return;
    
} //end TEST_CASE

