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


static void _set_static_areas(sc::opt_ptr & opts_ptr, mc_vm_map * map) {

    int ret;
    cm_lst_node * area_node, * obj_node;
    mc_vm_obj * obj;
    std::vector<const cm_lst_node *> static_areas;

    
    //build static areas - unit target's rw- area
    obj_node = mc_get_obj_by_basename(map,
                                      _target_helper::target_name);
    obj = MC_GET_NODE_OBJ(obj_node);
    area_node = obj->vm_area_node_ps.head->prev;
    static_areas.push_back(area_node);

    //build static areas - `[heap]` area
    obj_node = mc_get_obj_by_basename(map, "[heap]");
    obj = MC_GET_NODE_OBJ(obj_node);
    area_node = obj->vm_area_node_ps.head;
    static_areas.push_back(area_node);

    //set static areas
    ret = opts_ptr.set_static_areas(static_areas);
    CHECK_EQ(ret, 0);

    return;
}


/*
 *  NOTE: The offset of `game_off` depends on the build of `unit_target`.
 *        If tests fail, verify this offset is correct.
 */

const constexpr off_t game_off   = 0xb0; //NOTE: Depends on build
const constexpr off_t entity_off = sizeof(uintptr_t);
const constexpr off_t stats_off  = 0x10;
const constexpr off_t pos_off    = 0x14;
const constexpr off_t health_off = 0x0;
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
    std::string pathname, basename;

    //for every pointer chain
    for (auto iter = chains.cbegin(); iter != chains.cend(); ++iter) {

        //fetch basename
        obj_node = iter->get_obj_node();
        if (obj_node.has_value()) {
            obj = MC_GET_NODE_OBJ(obj_node.value());
            basename = obj->basename;
        } else {
            pathname = iter->get_pathname().value();
            basename = mc_pathname_to_basename(pathname.c_str());
        }

        //fetch offsets
        const std::vector<off_t> & offs = iter->get_offsets();

        //print this entry
        std::cout << basename << ":" << std::hex;
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
    _memcry_helper::setup(mcry_args, pid, 8);


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


        //third test: specify preset offsets & static areas

        //dump map
        subtitle("target - player 3's health", "target memory map");
        _memcry_helper::print_map(&mcry_args.map);

        //set the target address to player 3's health
        std::vector<off_t> offs_2 = {
            game_off,
            entity_off * 2,
            stats_off
        };
        target_addr = _set_target(opts_ptr, mcry_args.sessions[0],
                                  mcry_args.map, offs_2);

        //set preset offsets
        std::vector<off_t> preset_offs = {0x10};
        ret = opts_ptr.set_preset_offsets(preset_offs);
        CHECK_EQ(ret, 0);

        //set static areas
        _set_static_areas(opts_ptr, &mcry_args.map);

        //perform the scan
        ret = ptrscan.scan(opts, opts_ptr, ma_set, wpool, 0x0);
        CHECK_EQ(ret, 0);

        //fetch the scan results
        const std::vector<struct sc::ptrscan_chain> & chains_2
            = ptrscan.get_chains();

        //display results
        subtitle("target - player 3's health", "pointer chains");
        _print_chains(chains_2);


    } //end test


    SUBCASE(test_cc_ptrscan_subtests[2]) {
        title(CC, "ptrscan", "Perform pointer scans (threaded)");

        //setup sessions
        std::vector<const mc_session *> session_ptrs = {
            &mcry_args.sessions[0],
            &mcry_args.sessions[1],
            &mcry_args.sessions[2],
            &mcry_args.sessions[3],
            &mcry_args.sessions[4],
            &mcry_args.sessions[5],
            &mcry_args.sessions[6],
            &mcry_args.sessions[7]
            
        };
        ret = opts.set_sessions(session_ptrs);
        CHECK_EQ(ret, 0);


        //first test: scan for player 4's armour

        //dump map
        subtitle("target - player 4's armour (threaded)",
                 "target memory map");
        _memcry_helper::print_map(&mcry_args.map);

        //set the target address to player 4's armour
        std::vector<off_t> offs_0 = {
            game_off,
            entity_off * 3,
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
        subtitle("target - player 4's armour (threaded)",
                 "pointer chains");
        _print_chains(chains_0);


        //second test: scan for player 2's name

        //dump map
        subtitle("target - player 2's name (threaded)",
                 "target memory map");
        _memcry_helper::print_map(&mcry_args.map);

        //set the target address to player 2's name
        std::vector<off_t> offs_1 = {
            game_off,
            entity_off * 1
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
        subtitle("target - player 2's name (threaded)", "pointer chains");
        _print_chains(chains_1);


        //third test: specify preset offsets & static areas

        //dump map
        subtitle("target - player 1's health (threaded)",
                 "target memory map");
        _memcry_helper::print_map(&mcry_args.map);

        //set the target address to player 1's health
        std::vector<off_t> offs_2 = {
            game_off,
            entity_off * 0,
            stats_off,
        };
        target_addr = _set_target(opts_ptr, mcry_args.sessions[0],
                                  mcry_args.map, offs_2);

        //set preset offsets
        std::vector<off_t> preset_offs = {0x10, 0x0};
        ret = opts_ptr.set_preset_offsets(preset_offs);
        CHECK_EQ(ret, 0);

        //set static areas
        _set_static_areas(opts_ptr, &mcry_args.map);

        //perform the scan
        ret = ptrscan.scan(opts, opts_ptr, ma_set, wpool, 0x0);
        CHECK_EQ(ret, 0);

        //fetch the scan results
        const std::vector<struct sc::ptrscan_chain> & chains_2
            = ptrscan.get_chains();

        //display results
        subtitle("target - player 1's health (threaded)", "pointer chains");
        _print_chains(chains_2);

    } //end test


    SUBCASE(test_cc_ptrscan_subtests[3]) {
        title(CC, "ptrscan", "Save & load scan results");

        //setup serialiser
        sc::serialiser serialiser;

        ret = opts.set_file_path_out(test_file);
        CHECK_EQ(ret, 0);
        ret = opts.set_file_path_in(test_file);
        CHECK_EQ(ret, 0);

        //setup sessions
        std::vector<const mc_session *> session_ptrs = {
            &mcry_args.sessions[0]
        };
        ret = opts.set_sessions(session_ptrs);
        CHECK_EQ(ret, 0);


        //only test: scan for player 2's armour, save & load the scan

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


        //fetch the scan results - original
        const std::vector<struct sc::ptrscan_chain> & chains_0
            = ptrscan.get_chains();

        //display results - original
        subtitle("target - player 2's armour", "original pointer chains");
        _print_chains(chains_0);

        //save scan results
        ret = serialiser.save_scan(ptrscan, opts);
        CHECK_EQ(ret, 0);


        //load scan results - shallow
        ret = serialiser.load_scan(ptrscan, opts, true);
        CHECK_EQ(ret, 0);

        //display results - shallow        
        const std::vector<struct sc::ptrscan_chain> & chains_1
            = ptrscan.get_chains();
        subtitle(
            "target - player 2's armour", "shallow read pointer chains");
        _print_chains(chains_1);


        //load scan results - deep
        ret = serialiser.load_scan(ptrscan, opts, false);
        CHECK_EQ(ret, 0);

        //display results - deep
        const std::vector<struct sc::ptrscan_chain> & chains_2
            = ptrscan.get_chains();
        subtitle("target - player 2's armour", "deep read pointer chains");
        _print_chains(chains_2);

    } //end test


    SUBCASE(test_cc_ptrscan_subtests[4]) {
        title(CC, "ptrscan", "Verify scan results");

        //setup serialiser
        sc::serialiser serialiser;

        ret = opts.set_file_path_out(test_file);
        CHECK_EQ(ret, 0);
        ret = opts.set_file_path_in(test_file);
        CHECK_EQ(ret, 0);

        //setup sessions
        std::vector<const mc_session *> session_ptrs = {
            &mcry_args.sessions[0]
        };
        ret = opts.set_sessions(session_ptrs);
        CHECK_EQ(ret, 0);


        //only test: scan for player 2's armour, save & load the scan

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


        //fetch the scan results - original
        const std::vector<struct sc::ptrscan_chain> & chains_0
            = ptrscan.get_chains();

        //display results - original
        subtitle("target - player 2's armour", "original pointer chains");
        _print_chains(chains_0);

        //save scan results
        ret = serialiser.save_scan(ptrscan, opts);
        CHECK_EQ(ret, 0);

        //load scan results
        ret = serialiser.load_scan(ptrscan, opts, false);
        CHECK_EQ(ret, 0);

        //verify chains
        ret = ptrscan.verify(opts, opts_ptr);
        CHECK_EQ(ret, 0);

        //display results
        const std::vector<struct sc::ptrscan_chain> & chains_1
            = ptrscan.get_chains();
        subtitle("target - player 2's armour", "verified pointer chains");
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
