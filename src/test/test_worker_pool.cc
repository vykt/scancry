//standard template library
#include <optional>
#include <vector>
#include <iostream>
#include <iomanip>
#include <utility>

//C standard library
#include <cstdlib>
    
//external libraries
#include <cmore.h>
#include <memcry.h>
#include <doctest/doctest.h>

//local headers
#include "filters.hh"
#include "common.hh"
#include "target_helper.hh"
#include "util_helper.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/worker.hh"



//worker thread fixture scan class
class _fixture_scan : public sc::_scan {

    public:        
        //[attributes]
        cm_byte expected_byte;
        off_t read_off;
        int mod;

        //[methods]
        /* internal */ [[nodiscard]] virtual _SC_DBG_INLINE int
            _process_addr(
                const struct sc::_scan_arg arg, const sc::opt * const opts,
                const sc::_opt_scan * const opts_scan);

        /* internal */ [[nodiscard]] virtual int _generate_body(
                std::vector<cm_byte> & buf, off_t hdr_off) { return 0; }
        /* internal */ [[nodiscard]] virtual int _process_body(
                const std::vector<cm_byte> & buf, off_t hdr_off,
                const mc_vm_map & map) { return 0; }
        /* internal */ [[nodiscard]] virtual int _read_body(
                const std::vector<cm_byte> & buf, off_t hdr_off) { return 0; }

        _fixture_scan(int mod) : expected_byte(0), read_off(0), mod(mod) {}

        [[nodiscard]] int scan(
                    sc::opt & opts,
                    sc::opt_ptrscan & opts_ptrscan,
                    sc::map_area_set & ma_set,
                    sc::worker_pool & w_pool,
                    cm_byte flags);

        [[nodiscard]] virtual int reset() { return 0; };
};


//worker thread fixture scan options class
class _fixture_opts : public sc::_opt_scan {

    public:
        //[methods]
        ~_fixture_opts() {}
        [[nodiscard]] virtual int reset() { return 0; }
};


/*
 *  NOTE: To test if workers read memory correctly, worker thread(s) are
 *        directed to read a mmap'ed `patternN.bin` file, which consists
 *        of an incrementing byte pattern for the first kilobyte, followed
 *        by a decrementing pattern for the next kilobyte. Checks are
 *      
 */
 
[[nodiscard]] _SC_DBG_INLINE int _fixture_scan::_process_addr(
                                    const struct sc::_scan_arg arg,
                                    const sc::opt * const opts,
                                    const sc::_opt_scan * const opts_scan) {

    //check byte pattern is correct
    CHECK_EQ(this->expected_byte, *arg.cur_byte);

    //advance state
    read_off += std::abs(this->mod);

    //apply modifier for each scanned byte
    if (read_off < 0x1000) {
        this->expected_byte += this->mod;
    } else {
        this->mod *= -1;
        read_off = 0x0;
    }

    return 0;
}



struct _setup_params {

    //ScanCry parameters
    sc::opt opts;
    _fixture_opts t_opts;
    _fixture_scan t_scan;
    sc::map_area_set ma_set;

    //MemCry parameters
    mc_session ses[8]; //test up to 8 workers
    mc_vm_map map;


    _setup_params(int mod)
     : opts(test_cc_addr_width),
       t_scan(_fixture_scan(mod)) {}
};


static pid_t _setup_memcry(_setup_params & p, int thread_num) {

    int ret;
    pid_t pid;


    //spawn target
    _target_helper::clean_targets();
    pid = _target_helper::start_target();
    REQUIRE_NE(pid, -1);

    //start MemCry sessions on the target
    for (int i = 0; i < thread_num; ++i) {
        ret = mc_open(&p.ses[i], PROCFS, pid);
        REQUIRE_EQ(ret, 0);
    }

    //initialise a MemCry map of the target
    ret = mc_update_map(&p.ses[0], &p.map);
    REQUIRE_EQ(ret, 0);

    return pid;   
}


static void _setup_scancry(_setup_params & p, int thread_num) {

    int ret;

    //assign the map to the options
    ret = p.opts.set_map(&p.map);
    REQUIRE_EQ(ret, 0);

    //create a map area set
    ret = p.ma_set.update_set(p.opts);
    REQUIRE_EQ(ret, 0);

    return;
}


static void _teardown_memcry(_setup_params & p, int thread_num) {

    int ret;
    

    //close MemCry sessions
    for (int i = 0; i < thread_num; ++i) {

        ret = mc_close(&p.ses[i]);
        CHECK_EQ(ret, 0);
    }

    //destroy the MemCry map
    ret = mc_del_vm_map(&p.map);
    CHECK_EQ(ret ,0);

    //kill the target
    _target_helper::clean_targets();

    return;
}


static void _teardown_worker_pool(_setup_params & p) {

    int ret;


    //reset options
    ret = p.opts.reset();
    CHECK_EQ(ret, 0);

    //reset the fixture scan's options
    ret = p.t_opts.reset();
    CHECK_EQ(ret, 0);

    //reset the fixture scan
    ret = p.t_scan.reset();
    CHECK_EQ(ret, 0);

    //reset the map area set
    ret = p.ma_set.reset();

    return;
}


#ifdef DEBUG
static void _print_scan_area_sets(sc::worker_pool & wp) {

    int ret, count;
    mc_vm_area * area;


    //for every scan area set
    count = 0;
    for (auto iter = wp.scan_area_sets.cbegin();
         iter != wp.scan_area_sets.cend(); ++iter) {

        //print the header for this set
        std::cout << " --- [" << "Scan area set: " << count << "] --- " << std::endl;

        //for every entry in a scan area set
        for (auto inner_iter = iter->cbegin();
             inner_iter != iter->cend(); ++iter) {

            area = MC_GET_NODE_AREA((*inner_iter));
            _util_helper::print_area(area);
                 
        } //end for each member of a scan area set
             
    } //end for each scan area set
}
#endif


/*
 *  --- [TESTS - WORKER_POOL] ---
 */

TEST_CASE(test_cc_opt_subtests[0]) {

    int ret;
    pid_t pid;
    
    cm_byte flags;
    cm_lst_node * obj_node_1, * obj_node_2;


    //test 0: construct worker pools

    //call regular constructor
    sc::worker_pool wp;


    //test 1 - setup & free workers
    SUBCASE(test_cc_worker_pool_subtests[1]) {

        //first test - one worker thread

        //setup MemCry
        _setup_params params(1);
        pid = _setup_memcry(params, 1);
        CHECK_NE(pid, 0);

        //setup ScanCry
        _setup_scancry(params, 1);

        //setup the worker pool
        ret = wp.setup(params.opts, params.t_opts,
                       params.t_scan, params.ma_set, 0x0);
        CHECK_EQ(ret, 0);

        #ifdef DEBUG
        //display scan area sets
        std::cout << "\nSCAN AREA SETS - ALL - 1 WORKER:" << std::endl;
        _print_scan_area_sets(wp);
        #endif

        //free workers while keeping scan area sets
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);

        //setup the worker pool again
        ret = wp.setup(params.opts, params.t_opts, params.t_scan,
                       params.ma_set, sc::WORKER_POOL_KEEP_SCAN_SET);
        CHECK_EQ(ret, 0);

        #ifdef DEBUG
        //display scan area sets
        std::cout << "\nSCAN AREA SETS - 1 - WORKER_POOL_KEEP_SCAN_SET:"
                  << std::endl;
        _print_scan_area_sets(wp);
        #endif

        //teardown
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);
        _teardown_worker_pool(params);
        _teardown_memcry(params, 1);
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
        

        //second test - three worker threads

        //setup memcry
        pid = _setup_memcry(params, 3);
        CHECK_NE(pid, 0);

        //setup ScanCry
        _setup_scancry(params, 3);

        //setup the worker pool
        ret = wp.setup(params.opts, params.t_opts,
                       params.t_scan, params.ma_set, 0x0);
        CHECK_EQ(ret, 0);

        #ifdef DEBUG
        //display scan area sets
        std::cout << "\nSCAN AREA SETS - ALL - 3 WORKERS:" << std::endl;
        _print_scan_area_sets(wp);
        #endif


        //add new constraints constraints, find nodes of patterned mmap'ed files
        obj_node_1 = mc_get_obj_by_basename(
                        &params.map, _target_helper::pattern_1_basename);
        obj_node_2 = mc_get_obj_by_basename(
                        &params.map, _target_helper::pattern_2_basename);

        //exclusively scan the fetched patterned mmap'ed objects
        std::vector<const cm_lst_node *> exclusive_objs
            = {obj_node_1, obj_node_2 };
        ret = params.opts.set_exclusive_objs(exclusive_objs);
        REQUIRE_EQ(ret, 0);
        

        //setup the worker pool
        ret = wp.setup(params.opts, params.t_opts, params.t_scan,
                       params.ma_set, sc::WORKER_POOL_KEEP_WORKERS);
        CHECK_EQ(ret, 0);

        #ifdef DEBUG
        //display scan area sets
        std::cout << "\nSCAN AREA SETS - MMAP PATTERNS - 3 WORKERS"
                  << " - WORKER_POOL_KEEP_WORKERS:" << std::endl;
        _print_scan_area_sets(wp);
        #endif
        
        //teardown
        ret = wp.free_workers();
        CHECK_EQ(ret, 0);
        _teardown_worker_pool(params);
        _teardown_memcry(params, 1);
        DOCTEST_INFO("WARNING: This test is incomplete, use a debugger to inspect state.");
    
    } //end test

}
