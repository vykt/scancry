//standard template library
#include <optional>
#include <vector>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <doctest/doctest.h>

//system headers
#include <unistd.h>

//local headers
#include "filters.hh"
#include "common.hh"
#include "memcry_helper.hh"
#include "target_helper.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/ptrscan.hh"


      /* ===================== * 
 ===== *  C++ INTERFACE TESTS  * =====
       * ===================== */

/*
 *  --- [HELPERS] ---
 */

//find the first `rw-` area of an object; the object _must_ contain one
static cm_lst_node * _get_rw_area(cm_lst_node * obj_node) {

    int ret;

    cm_lst_node * area_node;
    mc_vm_area * area;


    mc_vm_obj * obj = MC_GET_NODE_OBJ(obj_node);

    for (int i = 0; i < obj->vm_area_node_ps.len; ++i) {

        ret = cm_lst_get(&obj->vm_area_node_ps, i, &area_node);
        CHECK_EQ(ret, 0);
        area = MC_GET_NODE_AREA(area_node);
        CHECK_NE(area, nullptr);

        if (area->access == (MC_ACCESS_READ | MC_ACCESS_WRITE)) break;
    }

    return area_node;
}


//walk a pointer chain and return the final address
static uintptr_t _walk_ptrchain(mc_session & session, uintptr_t start_addr,
                                std::vector<off_t> offsets) {

    int ret;
    uintptr_t addr = start_addr;

    for (int i = 0; i < offsets.size(); ++i) {

        addr += offsets[i];
        if (i != (offsets.size() - 1)) {
            ret = mc_read(&session, addr, (cm_byte *) &addr, sizeof(addr));
            CHECK_EQ(ret, 0);
        }
    }

    return addr;
}

/*
 *  NOTE: The offset of `game_off` depends on the build of `unit_target`.
 *        If tests fail, verify this offset is correct.
 */

const constexpr off_t game_off   = 0xb0; //NOTE: Depends on build
const constexpr off_t entity_off = sizeof(uintptr_t);
const constexpr off_t stats_off  = 0x10;
const constexpr off_t pos_off    = 0x14;
const constexpr off_t armour_off = 0x4;


static uintptr_t _set_target(sc::opt_ptr & opts_ptr, mc_session & session,
                             mc_vm_map & map, std::vector<off_t> offs) {
    
    int ret;
    uintptr_t target_addr;

    cm_lst_node * obj_node, * area_node;
    mc_vm_area * area;
    

    //fetch the target object
    obj_node = map.vm_objs.head->next;

    //find the `rw-` area
    area_node = _get_rw_area(obj_node);
    area = MC_GET_NODE_AREA(area_node);

    //get the given player's armour address
    target_addr = _walk_ptrchain(session, area->start_addr, offs);
    std::cout << "target address: 0x" << std::hex << target_addr
              << std::dec << std::endl;

    ret = opts_ptr.set_target_addr(target_addr);
    CHECK_EQ(ret, 0);

    return target_addr;
}


static void _print_chains(const std::vector<sc::ptrscan_chain> & chains) {

    std::optional<const cm_lst_node *> obj_node;
    mc_vm_obj * obj;

    //for every pointer chain
    for (auto iter = chains.cbegin(); iter != chains.cend(); ++iter) {

        //get relevant fields
        obj_node = iter->get_obj_node();
        obj = MC_GET_NODE_OBJ(obj_node.value());
        const std::vector<off_t> & offs = iter->get_offsets();

        //print this entry
        std::cout << obj->basename << ":" << std::hex;
        for (auto off_iter = offs.cbegin();
             off_iter != offs.cend(); ++off_iter) {

            std::cout << " +0x" << *off_iter;
        }
        std::cout << std::dec << std::endl;

    } //end for every pointer chain

    return;
}



/*
 *  --- [TESTS] ---
 */

TEST_CASE(test_cc_ptrscan_subtests[0]) {

    int ret;

    pid_t pid;
    uintptr_t target_addr;
    _memcry_helper::args mcry_args;

    sc::opt opts(sc::AW64);
    sc::opt_ptr opts_ptr;
    sc::map_area_set ma_set;
    sc::worker_pool wpool;
    sc::ptrscan ptrscan;


    /*
     *  FIXTURE: Setup minimum options & construct a pointer scanner.
     */

    //setup a target
    ret = _target_helper::clean_targets();
    CHECK_EQ(ret, 0);

    pid = _target_helper::start_target();
    CHECK_NE(pid, 0);

    //setup MemCry
    _memcry_helper::setup(mcry_args, pid, 2);


    //setup generic options
    ret = opts.set_map(&mcry_args.map);
    CHECK_EQ(ret, 0);

    //TODO DEBUG scan only the `[heap]`
    /*std::vector<const cm_lst_node *> excl_objs;
    cm_lst_node * obj_node
        = mc_get_obj_by_basename(&mcry_args.map, "[heap]");
    excl_objs.push_back(obj_node);
    ret = opts.set_exclusive_objs(excl_objs);
    *///TODO END DEBUG

    //setup ptrscan options
    ret = opts_ptr.set_alignment(4);
    CHECK_EQ(ret, 0);

    ret = opts_ptr.set_max_depth(3);
    CHECK_EQ(ret, 0);

    ret = opts_ptr.set_max_obj_sz(0x20);
    CHECK_EQ(ret, 0);


    //setup a scan on the entire map
    ret = ma_set.update_set(opts);
    CHECK_EQ(ret, 0);

    
    SUBCASE(test_cc_ptrscan_subtests[1]) {
        title(CC, "ptrscan", "Perform pointer scans");

        //setup sessions
        std::vector<const mc_session *> session_ptrs = {
            &mcry_args.sessions[0]
        };
        ret = opts.set_sessions(session_ptrs);
        CHECK_EQ(ret, 0);


        //first test: scan for player 2's armour

        //dump map
        subtitle("target - player 2's armour", "target memory map");
        _memcry_helper::print_map(&mcry_args.map);

        //set the target address to player 2's armour
        std::vector<off_t> offs_0 = {
            game_off,
            entity_off * 1,
            stats_off,
            armour_off
        };
        target_addr = _set_target(opts_ptr, mcry_args.sessions[0],
                                  mcry_args.map, offs_0);

        //perform the scan
        ret = ptrscan.scan(opts, opts_ptr, ma_set, wpool, 0x0);
        CHECK_EQ(ret, 0);

        //fetch the scan results
        const std::vector<struct sc::ptrscan_chain> & chains_0
            = ptrscan.get_chains();

        //display results
        subtitle("target - player 2's armour", "pointer chains");
        _print_chains(chains_0);

    
        //second test: scan for player 1's name

        //dump map
        subtitle("target - player 1's name", "target memory map");
        _memcry_helper::print_map(&mcry_args.map);

        //set the target address to player 1's name
        std::vector<off_t> offs_1 = {
            game_off,
            entity_off * 0
        };
        target_addr = _set_target(opts_ptr, mcry_args.sessions[0],
                                  mcry_args.map, offs_1);

        //perform the scan
        ret = ptrscan.scan(opts, opts_ptr, ma_set, wpool, 0x0);
        CHECK_EQ(ret, 0);

        //fetch the scan results
        const std::vector<struct sc::ptrscan_chain> & chains_1
            = ptrscan.get_chains();

        //display results
        subtitle("target - player 1's name", "pointer chains");
        _print_chains(chains_1);
    

    } //end test



    //free workers
    ret = wpool.free_workers();
    CHECK_EQ(ret, 0);

    //reset the map area set
    ret = ma_set.reset();
    CHECK_EQ(ret, 0);

    //teardown MemCry
    _memcry_helper::teardown(mcry_args);

    //teardown target
    _target_helper::end_target(pid);
}
