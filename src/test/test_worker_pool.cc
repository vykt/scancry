//standard template library
#include <optional>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <utility>

//C standard library
#include <cstdlib>
#include <cstring>
#ifdef VERBOSE_DEBUG
#include <cstdio>
#endif

//system headers
#include <unistd.h>
    
//external libraries
#include <cmore.h>
#include <memcry.h>
#include <doctest/doctest.h>

//local headers
#include "filters.hh"
#include "common.hh"
#include "scan_helper.hh"
#include "target_helper.hh"
#include "memcry_helper.hh"
#include "opt_helper.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/worker.hh"



      /* ===================== * 
 ===== *  C++ INTERFACE TESTS  * =====
       * ===================== */

/*
 *  --- [HELPERS] ---
 */

static void _print_scan_area_sets(sc::worker_pool & wp,
                                  std::string header) {
#ifdef DEBUG
    int ret, count;
    mc_vm_area * area;


    //print header
    subtitle("scan area sets", header);

    //for every scan area set
    count = 0;
    for (auto iter = wp.scan_area_sets.cbegin();
         iter != wp.scan_area_sets.cend(); ++iter) {

        //calculate the size of this scan area set
        size_t sz = 0;
        for (auto inner_iter = iter->cbegin();
             inner_iter != iter->cend(); ++inner_iter) {

            area = MC_GET_NODE_AREA((*inner_iter));
            sz += (area->end_addr - area->start_addr);
        }

        //print the header for this set
        std::stringstream subtitle_ss;
        subtitle_ss << "set: " << count << ", sz: 0x"
                    << std::hex << sz << std::dec;
        subtitle("scan_area_sets", subtitle_ss.str());

        //for every entry in a scan area set
        for (auto inner_iter = iter->cbegin();
             inner_iter != iter->cend(); ++inner_iter) {

            area = MC_GET_NODE_AREA((*inner_iter));
            _memcry_helper::print_area(area);
                 
        } //end for each member of a scan area set
             
    } //end for each scan area set
    
#else
    std::cout << "<Only available in debug builds.>\n" << std::endl;
#endif
    return;
}


static void _assert_worker_count(sc::worker_pool & wp, int worker_count) {
#ifdef DEBUG
    CHECK_EQ(wp.workers.size(), worker_count);
    CHECK_EQ(wp.worker_ids.size(), worker_count);
#endif
    return;
}


static void _assert_worker_concurrency(
    sc::worker_pool & wp, int release_count, int alive_count) {
#ifdef DEBUG
    CHECK_EQ(wp.concur.release_count, release_count);
    CHECK_EQ(wp.concur.alive_count, alive_count);
#endif
    return;
}



/*
 *  --- [TESTS] ---
 */

TEST_CASE(test_cc_worker_pool_subtests[0]) {

    int ret;
    cm_lst_node * node;

    pid_t pid;
    _memcry_helper::args mcry_args;
    _opt_helper::args opt_args(sc::AW64);
    
    _scan_helper::_fixture_opts fixt_opts;
    _scan_helper::_fixture_scan fixt_scan;


    //launch target
    _target_helper::clean_targets();
    pid = _target_helper::start_target();


    //test 0: construct worker pools

    //call regular constructor
    sc::worker_pool wp;


    //test 1 - setup & free workers
    SUBCASE(test_cc_worker_pool_subtests[1]) {
        title(CC, "worker_pool", "Setup & free workers");

        //first test - entire target

        //perform setup
        _memcry_helper::setup(mcry_args, pid, 1);
        _opt_helper::setup(opt_args, mcry_args, [&]{});
        fixt_scan.set_do_checks(false);

        //setup the worker pool
        ret = wp._setup(opt_args.opts, fixt_opts,
                        fixt_scan, opt_args.ma_set, 0b0);
        CHECK_EQ(ret, 0);
        usleep(thread_wait_usec_time);
        _assert_worker_count(wp, 1);
        _assert_worker_concurrency(wp, 1, 1);

        //display scan area sets
        _print_scan_area_sets(wp, "everything - 1 worker");

        //reset the worker pool
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);
        _assert_worker_count(wp, 0);
        _assert_worker_concurrency(wp, 0, 0);

        //teardown setup
        _opt_helper::teardown(opt_args);
        _memcry_helper::teardown(mcry_args);


        //second test - only setup a main executable scan

        //re-do setup
        _memcry_helper::setup(mcry_args, pid, 1);
        _opt_helper::setup(opt_args, mcry_args, [&]{
        fixt_scan.set_do_checks(false);

            //fetch target's object
            node = mc_get_obj_by_basename(&mcry_args.map,
                                          _target_helper::target_name);
            CHECK_NE(node, nullptr);

            std::vector<const cm_lst_node *> exclusive_objs = { node };

            ret = opt_args.opts.set_exclusive_objs(exclusive_objs);
            CHECK_EQ(ret, 0);
        });

        //setup worker pool
        ret = wp._setup(opt_args.opts, fixt_opts,
                        fixt_scan, opt_args.ma_set, 0b0);
        CHECK_EQ(ret, 0);
        usleep(thread_wait_usec_time);
        _assert_worker_count(wp, 1);
        _assert_worker_concurrency(wp, 1, 1);

        //display scan area sets
        _print_scan_area_sets(wp, "main executable only - 1 worker");

        //reset the worker pool
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);
        _assert_worker_count(wp, 0);
        _assert_worker_concurrency(wp, 0, 0);

        //teardown setup
        _opt_helper::teardown(opt_args);
        _memcry_helper::teardown(mcry_args);

        DOCTEST_INFO("WARNING: This test requires a debug build (`-DDEBUG`).");
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");

    } //end test
    
    
    //test 2 - setup & free workers (multithreaded)
    SUBCASE(test_cc_worker_pool_subtests[2]) {
        title(CC, "worker_pool", "Setup & free workers (multithreaded)");

        //first test - entire target - 8 threads

        //perform setup
        _memcry_helper::setup(mcry_args, pid, 8);
        _opt_helper::setup(opt_args, mcry_args, [&]{});
        fixt_scan.set_do_checks(false);

        //setup the worker pool
        ret = wp._setup(opt_args.opts, fixt_opts,
                        fixt_scan, opt_args.ma_set, 0b0);
        CHECK_EQ(ret, 0);
        usleep(thread_wait_usec_time);
        _assert_worker_count(wp, 8);
        _assert_worker_concurrency(wp, 8, 8);

        //display scan area sets
        _print_scan_area_sets(wp, "everything - 8 workers");

        //reset the worker pool
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);
        _assert_worker_count(wp, 0);
        _assert_worker_concurrency(wp, 0, 0);

        //teardown setup
        _opt_helper::teardown(opt_args);
        _memcry_helper::teardown(mcry_args);


        //second test - only scan the main executable

        //re-do setup
        _memcry_helper::setup(mcry_args, pid, 8);
        _opt_helper::setup(opt_args, mcry_args, [&]{
        fixt_scan.set_do_checks(false);

            //fetch target's object
            node = mc_get_obj_by_basename(&mcry_args.map,
                                          _target_helper::target_name);
            CHECK_NE(node, nullptr);

            std::vector<const cm_lst_node *> exclusive_objs = { node };

            ret = opt_args.opts.set_exclusive_objs(exclusive_objs);
            CHECK_EQ(ret, 0);
        });

        //setup worker pool
        ret = wp._setup(opt_args.opts, fixt_opts,
                        fixt_scan, opt_args.ma_set, 0b0);
        CHECK_EQ(ret, 0);
        usleep(thread_wait_usec_time);
        _assert_worker_count(wp, 8);
        _assert_worker_concurrency(wp, 8, 8);

        //display scan area sets
        _print_scan_area_sets(wp, "main executable only - 8 workers");

        //reset the worker pool
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);
        _assert_worker_count(wp, 0);
        _assert_worker_concurrency(wp, 0, 0);

        //teardown setup
        _opt_helper::teardown(opt_args);
        _memcry_helper::teardown(mcry_args);

        DOCTEST_INFO(
            "WARNING: This test requires a debug build (`-DDEBUG`).");
        DOCTEST_INFO(
            "WARNING: This test is incomplete, use a debugger to inspect state.");

    } //end test


    //test 3 - flag tests
    SUBCASE(test_cc_worker_pool_subtests[3]) {
        title(CC, "worker_pool", "Flags");

        //only test - keep workers & keep the scan set

        /*
         *  NOTE: The underlying type of pthread_t varies across
         *        implementations. For this test, it's assumed
         *        a `memcmp()` is sufficient to prove the two IDs
         *        are identical.
         */

        //store for old pthread IDs
        pthread_t old_pthread_ids[2] = {0};

        //perform setup
        _memcry_helper::setup(mcry_args, pid, 2);
        _opt_helper::setup(opt_args, mcry_args, [&]{});

        //setup the worker pool
        ret = wp._setup(opt_args.opts, fixt_opts,
                        fixt_scan, opt_args.ma_set, 0b0);
        CHECK_EQ(ret, 0);
        usleep(thread_wait_usec_time);
        _assert_worker_count(wp, 2);
        _assert_worker_concurrency(wp, 2, 2);

        #ifdef DEBUG
        //copy old pthread IDs
        std::memcpy(&old_pthread_ids[0],
                    &wp.worker_ids[0], sizeof(old_pthread_ids[0]));
        std::memcpy(&old_pthread_ids[1],
                    &wp.worker_ids[1], sizeof(old_pthread_ids[1]));
        #endif

        //change map area set
        _opt_helper::teardown(opt_args);
        _opt_helper::setup(opt_args, mcry_args, [&]{});

        //re-initialise the worker pool
        ret = wp._setup(opt_args.opts, fixt_opts,
                        fixt_scan, opt_args.ma_set,
                        sc::WORKER_POOL_KEEP_WORKERS
                        | sc::WORKER_POOL_KEEP_SCAN_SET);
        CHECK_EQ(ret, 0);
        usleep(thread_wait_usec_time);

        #ifdef DEBUG
        //check pthread IDs are the same
        CHECK_EQ(std::memcmp(&old_pthread_ids[0],
                 &wp.worker_ids[0], sizeof(old_pthread_ids[0])), 0);
        CHECK_EQ(std::memcmp(&old_pthread_ids[1],
                 &wp.worker_ids[1], sizeof(old_pthread_ids[1])), 0);
        #endif

        //reset the worker pool
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);
        _assert_worker_count(wp, 0);
        _assert_worker_concurrency(wp, 0, 0);

        //teardown setup
        _opt_helper::teardown(opt_args);
        _memcry_helper::teardown(mcry_args);

        DOCTEST_INFO("WARNING: This test requires a debug build (`-DDEBUG`).");
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");

    } //end test


    //test 4 - worker scan test
    SUBCASE(test_cc_worker_pool_subtests[4]) {
        title(CC, "worker_pool", "Perform scan");

        //first test - read patterned memory from mmap'ed file

        //perform setup
        _memcry_helper::setup(mcry_args, pid, 1);
        _opt_helper::setup(opt_args, mcry_args, [&]{
            std::vector<const cm_lst_node *> exclusive_objs;
            
            //fetch pattern objects
            node = mc_get_obj_by_basename(&mcry_args.map,
                                          _target_helper::pattern_1_basename);
            CHECK_NE(node, nullptr);
            exclusive_objs.push_back(node);

            node = mc_get_obj_by_basename(&mcry_args.map,
                                          _target_helper::pattern_2_basename);
            CHECK_NE(node, nullptr);
            exclusive_objs.push_back(node);

            ret = opt_args.opts.set_exclusive_objs(exclusive_objs);
            CHECK_EQ(ret, 0);
        });
        ret = fixt_scan.reset();
        CHECK_EQ(ret, 0);
        fixt_scan.set_do_checks(true);
        fixt_scan.set_mod(1);

        //setup the worker pool
        ret = wp._setup(opt_args.opts, fixt_opts,
                        fixt_scan, opt_args.ma_set, 0b0);
        CHECK_EQ(ret, 0);
        usleep(thread_wait_usec_time);
        _assert_worker_count(wp, 1);
        _assert_worker_concurrency(wp, 1, 1);

        //run the scan
        ret = wp._single_run();
        CHECK_EQ(ret, 0);        
        CHECK_EQ(fixt_scan.get_call_count(),
                _target_helper::pattern_1_sz + _target_helper::pattern_2_sz);

        //reset the worker pool
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);
        _assert_worker_count(wp, 0);
        _assert_worker_concurrency(wp, 0, 0);

        //teardown setup
        _opt_helper::teardown(opt_args);
        _memcry_helper::teardown(mcry_args);


        //second test - read patterned memory from mmap'ed file, every 4th byte

        //perform setup
        _memcry_helper::setup(mcry_args, pid, 1);
        _opt_helper::setup(opt_args, mcry_args, [&]{
            std::vector<const cm_lst_node *> exclusive_objs;
            
            //fetch pattern objects
            node = mc_get_obj_by_basename(&mcry_args.map,
                                          _target_helper::pattern_1_basename);
            CHECK_NE(node, nullptr);
            exclusive_objs.push_back(node);

            node = mc_get_obj_by_basename(&mcry_args.map,
                                          _target_helper::pattern_2_basename);
            CHECK_NE(node, nullptr);
            exclusive_objs.push_back(node);

            ret = opt_args.opts.set_exclusive_objs(exclusive_objs);
            CHECK_EQ(ret, 0);
        });
        ret = fixt_scan.reset();
        CHECK_EQ(ret, 0);
        fixt_scan.set_do_checks(true);
        fixt_scan.set_mod(4);

        //setup the worker pool
        ret = wp._setup(opt_args.opts, fixt_opts,
                        fixt_scan, opt_args.ma_set, 0b0);
        CHECK_EQ(ret, 0);
        usleep(thread_wait_usec_time);
        _assert_worker_count(wp, 1);
        _assert_worker_concurrency(wp, 1, 1);

        //run the scan
        ret = wp._single_run();
        CHECK_EQ(ret, 0);        
        CHECK_EQ(fixt_scan.get_call_count(),
                 (_target_helper::pattern_1_sz + _target_helper::pattern_2_sz) / 4);

        //reset the worker pool
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);
        _assert_worker_count(wp, 0);
        _assert_worker_concurrency(wp, 0, 0);

        //teardown setup
        _opt_helper::teardown(opt_args);
        _memcry_helper::teardown(mcry_args);

        DOCTEST_INFO("WARNING: This test requires a debug build (`-DDEBUG`).");
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");

    } //end test


    //test 5 - worker scan test (multithreaded)
    SUBCASE(test_cc_worker_pool_subtests[5]) {
        title(CC, "worker_pool", "Perform scan (multithreaded)");

        //only test - read patterned memory from mmap'ed file

        //perform setup
        _memcry_helper::setup(mcry_args, pid, 8);
        _opt_helper::setup(opt_args, mcry_args, [&]{
            std::vector<const cm_lst_node *> exclusive_objs;
            
            //fetch pattern objects
            node = mc_get_obj_by_basename(&mcry_args.map,
                                          _target_helper::pattern_1_basename);
            CHECK_NE(node, nullptr);
            exclusive_objs.push_back(node);

            node = mc_get_obj_by_basename(&mcry_args.map,
                                          _target_helper::pattern_2_basename);
            CHECK_NE(node, nullptr);
            exclusive_objs.push_back(node);

            ret = opt_args.opts.set_exclusive_objs(exclusive_objs);
            CHECK_EQ(ret, 0);
        });
        ret = fixt_scan.reset();
        CHECK_EQ(ret, 0);
        fixt_scan.set_do_checks(false);
        fixt_scan.set_mod(1);

        //setup the worker pool
        ret = wp._setup(opt_args.opts, fixt_opts,
                        fixt_scan, opt_args.ma_set, 0b0);
        CHECK_EQ(ret, 0);
        usleep(thread_wait_usec_time);
        _assert_worker_count(wp, 8);
        _assert_worker_concurrency(wp, 8, 8);

        //run the scan
        ret = wp._single_run();
        CHECK_EQ(ret, 0);        

        //reset the worker pool
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);
        _assert_worker_count(wp, 0);
        _assert_worker_concurrency(wp, 0, 0);

        //teardown setup
        _opt_helper::teardown(opt_args);
        _memcry_helper::teardown(mcry_args);

        DOCTEST_INFO(
            "WARNING: This test requires a debug build (`-DDEBUG`).");
        DOCTEST_INFO(
            "WARNING: This test is incomplete, use a debugger to inspect state.");

    } //end test


    //test 6 - force one / all threads to crash
    SUBCASE(test_cc_worker_pool_subtests[5]) {
        title(CC, "worker_pool", "Force threads to crash");

        DOCTEST_INFO(
            "NOTE: Expect worker errors during this test.");

        //first test - force one worker to crash

        //perform setup
        _memcry_helper::setup(mcry_args, pid, 2);
        _opt_helper::setup(opt_args, mcry_args, [&]{});
        ret = fixt_scan.reset();
        CHECK_EQ(ret, 0);
        fixt_scan.set_do_checks(false);
        fixt_scan.set_mod(1);
        fixt_scan.set_do_crash_one(true);

        //setup the worker pool
        ret = wp._setup(opt_args.opts, fixt_opts,
                        fixt_scan, opt_args.ma_set, 0b0);
        CHECK_EQ(ret, 0);
        usleep(thread_wait_usec_time);
        _assert_worker_count(wp, 2);
        _assert_worker_concurrency(wp, 2, 2);

        //run the scan
        ret = wp._single_run();
        CHECK_EQ(ret, -1);        

        //reset the worker pool
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);
        _assert_worker_count(wp, 0);
        _assert_worker_concurrency(wp, 0, 0);

        //teardown setup
        _opt_helper::teardown(opt_args);
        _memcry_helper::teardown(mcry_args);


        //second test - force all workers to crash

        //perform setup
        _memcry_helper::setup(mcry_args, pid, 2);
        _opt_helper::setup(opt_args, mcry_args, [&]{});
        ret = fixt_scan.reset();
        CHECK_EQ(ret, 0);
        fixt_scan.set_do_checks(false);
        fixt_scan.set_mod(1);
        fixt_scan.set_do_crash_all(true);

        //setup the worker pool
        ret = wp._setup(opt_args.opts, fixt_opts,
                        fixt_scan, opt_args.ma_set, 0b0);
        CHECK_EQ(ret, 0);
        usleep(thread_wait_usec_time);
        _assert_worker_count(wp, 2);
        _assert_worker_concurrency(wp, 2, 2);

        //run the scan
        ret = wp._single_run();
        CHECK_EQ(ret, -1);        

        //reset the worker pool
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);
        _assert_worker_count(wp, 0);
        _assert_worker_concurrency(wp, 0, 0);

        //teardown setup
        _opt_helper::teardown(opt_args);
        _memcry_helper::teardown(mcry_args);

        DOCTEST_INFO("WARNING: This test requires a debug build (`-DDEBUG`).");
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");

        DOCTEST_INFO(
            "NOTE: Worker errors no longer expected.");

    } //end test


    //destroy the target
    _target_helper::end_target(pid);

    return;

} //end TEST_CASE


      /* =================== * 
 ===== *  C INTERFACE TESTS  * =====
       * =================== */

/*
 *  --- [TESTS] ---
 */

TEST_CASE(test_c_worker_pool_subtests[0]) {

    int ret;
    cm_lst_node * node;

    pid_t pid;
    _memcry_helper::args mcry_args;
    _opt_helper::args opt_args(sc::AW64);
    
    _scan_helper::_fixture_opts fixt_opts;
    _scan_helper::_fixture_scan fixt_scan;


    //launch target
    pid = _target_helper::start_target();
    CHECK_NE(pid, -1);


    //test 0: construct worker pools

    //create a C worker pool
    sc_worker_pool wp = sc_new_worker_pool();
    REQUIRE_NE(wp, nullptr);
    sc::worker_pool * wp_cc = (sc::worker_pool *) wp;


    //test 1 - setup & free workers
    SUBCASE(test_cc_worker_pool_subtests[1]) {

        //only test - entire target

        //perform setup
        _memcry_helper::setup(mcry_args, pid, 1);
        _opt_helper::setup(opt_args, mcry_args, [&]{});

        //setup the worker pool
        ret = wp_cc->_setup(opt_args.opts, fixt_opts,
                            fixt_scan, opt_args.ma_set, 0b0);
        CHECK_EQ(ret, 0);
        _assert_worker_count(*wp_cc, 1);
        _assert_worker_concurrency(*wp_cc, 1, 1);

        //reset the worker pool
        ret = sc_wp_free_workers(wp);
        CHECK_EQ(ret, 0);
        _assert_worker_count(*wp_cc, 0);
        _assert_worker_concurrency(*wp_cc, 0, 0);

        //teardown setup
        _opt_helper::teardown(opt_args);
        _memcry_helper::teardown(mcry_args);

        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");

    } //end test

    ///delete the C worker pool
    ret = sc_del_worker_pool(wp);
    CHECK_EQ(ret, 0);

    //destroy the target
    _target_helper::end_target(pid);

    return;
    
} //end TEST_CASE
