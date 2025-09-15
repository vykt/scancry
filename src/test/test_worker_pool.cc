//standard template library
#include <string>
#include <variant>
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
#include "scan_helper.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/worker.hh"



/*
 *  --- [SHARED] ---
 */

namespace _shared {

//assert scan arg
static void _assert_scan_arg(
    const sc::_scan_arg & scan_arg,
    const uintptr_t addr,
    const off_t area_off,
    const size_t buf_left) {

    #ifdef SC_DEBUG
    REQUIRE_EQ(scan_arg.addr, addr);    
    REQUIRE_EQ(scan_arg.area_off, area_off);
    REQUIRE_EQ(scan_arg.buf_left, buf_left);
    #endif

    return;
}


//assert worker concurrency
static void _assert_worker_concurrency(
    const sc::_worker_concurrency & concur,
    const int release_count,
    const int alive_count,
    const int flags,
    const int exit_uids_len,
    const int wkr_errno) {

    #ifdef SC_DEBUG
    REQUIRE_EQ(concur.release_count, release_count);
    REQUIRE_EQ(concur.alive_count, alive_count);
    REQUIRE_EQ(concur.flags, flags);
    REQUIRE_EQ(concur.exit_uids.len,  exit_uids_len);
    REQUIRE_EQ(concur.wkr_errno, wkr_errno);
    #endif

    return;
}


//assert worker pool cache
static void _assert_worker_pool_cache(
    const sc::_worker_pool_cache & cache,
    const bool is_locked,
    const sc::opt * opts,
    const sc::_opt_scan * opts_scan,
    const sc::_scan * scan) {

    #ifdef SC_DEBUG
    REQUIRE_EQ(cache.is_locked, is_locked);
    #endif

    REQUIRE_EQ(cache.get_opts(), opts);
    REQUIRE_EQ(cache.get_opts_scan(), opts_scan);
    REQUIRE_EQ(cache.get_scan(), scan);

    return;
}


//assert worker
static void _assert_worker(
    const sc::_worker & wkr,
    const int uid,
    const cm_vct & scan_area_subset,
    const int session_idx,
    const mc_session * cached_session,
    const sc::_worker_pool_cache & cache,
    const sc::_worker_concurrency & concur,
    const bool is_buf_null) {

    #ifdef SC_DEBUG
    REQUIRE_EQ(wkr.uid, uid);
    REQUIRE_EQ(&wkr.scan_area_subset, &scan_area_subset);
    REQUIRE_EQ(wkr.session_idx, session_idx);
    REQUIRE_EQ(wkr.cached_session, cached_session);
    REQUIRE_EQ(&wkr.pool_cache, &cache);
    REQUIRE_EQ(&wkr.concur, &concur);
    if (is_buf_null == false) { REQUIRE_NE(wkr.buf, nullptr); }
    #endif

    return;
}


//assert worker bundle
static void _assert_worker_bundle(
    const sc::_worker_bundle & wkr_bundle) {

    #ifdef SC_DEBUG
    REQUIRE_EQ(wkr_bundle.scan_area_subset.is_init, true);
    #endif

    return;
}


//assert worker pool
static void _assert_worker_pool(
    const sc::worker_pool & w_pool,
    const int wkr_bundles_len,
    const int sorted_areas_cache_len) {

    #ifdef SC_DEBUG
    REQUIRE_EQ(w_pool.wkr_bundles.len, wkr_bundles_len);
    REQUIRE_EQ(w_pool.sorted_areas_cache.len, sorted_areas_cache_len);
    #endif
}


//assert everything
static void _assert_all_state(
    //inspection target
    const sc::worker_pool & w_pool,
    //parameters
    const int wkr_bundles_len,
    const std::vector<int> wkr_uids,
    const std::vector<int> wkr_session_idxs,
    const cm_vct & wkr_sessions,
    const int sorted_areas_cache_len,
    const bool cache_is_locked,
    const sc::opt * cache_opts,
    const sc::_opt_scan * cache_opts_scan,
    const sc::_scan * cache_scan,
    const int concur_release_count,
    const int concur_alive_count,
    const cm_byte concur_flags,
    const int concur_exit_uids_len,
    const int concur_wkr_errno) {

    sc::_worker_bundle * wkr_bundle;
    sc::_worker * wkr;


    //assert the worker pool
    _shared::_assert_worker_pool(
        w_pool, wkr_bundles_len, sorted_areas_cache_len);

    #ifdef SC_DEBUG
    //assert all workers
    for (int i = 0; i < wkr_bundles_len; ++i) {

        //fetch the next worker bundle
        wkr_bundle = (sc::_worker_bundle *)
                         cm_lst_get_p(&w_pool.wkr_bundles, i);
        _shared::_assert_worker_bundle(*wkr_bundle);

        //fetch the next worker
        wkr = &wkr_bundle->wkr;
        _shared::_assert_worker(
            *wkr,
            wkr_uids[i],
            wkr_bundle->scan_area_subset,
            wkr_session_idxs[i],
            &((mc_session *) wkr_sessions.data)[i],
            w_pool.cache,
            w_pool.concur,
            false);
    }

    //assert the worker pool cache
    _shared::_assert_worker_pool_cache(
        w_pool.cache,
        cache_is_locked,
        cache_opts,
        cache_opts_scan,
        cache_scan);

    //assert concurrency
    _shared::_assert_worker_concurrency(
        w_pool.concur,
        concur_release_count,
        concur_alive_count,
        concur_flags,
        concur_exit_uids_len,
        concur_wkr_errno);
    #endif

    return;
}

} //end namespace `_shared`




/*
 *  --- [WORKER_POOL] ---
 */


// -- ctor & dtor

namespace _worker_pool {

    namespace _ctor_dtor {

    //ctor asserts
    static void _ctor_asserts(const sc::worker_pool & w_pool) {

        //assert constructor succeeded
        REQUIRE_EQ(w_pool.get_ctor_failed(), false);

        //assert worker pool
        _shared::_assert_worker_pool(w_pool, 0, 0);

        #ifdef SC_DEBUG
        _shared::_assert_worker_pool_cache(
            w_pool.cache, false, nullptr, nullptr, nullptr);
        _shared::_assert_worker_concurrency(w_pool.concur, 0, 0, 0b0, 0, 0);
        #endif
    }


    //fixture
    static void _fixture(const sc::worker_pool & w_pool) {

        #ifdef SC_DEBUG
        _class_helper::lst::setup_stub(w_pool.wkr_bundles);
        _class_helper::vct::setup_stub(w_pool.sorted_areas_cache);
        _class_helper::vct::setup_stub(w_pool.concur.exit_uids);
        #endif
    }


    //dtor asserts
    static void _dtor_asserts(const sc::worker_pool & w_pool) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(w_pool.wkr_bundles.is_init, false);
        REQUIRE_EQ(w_pool.sorted_areas_cache.is_init, false);
        REQUIRE_EQ(w_pool.concur.exit_uids.is_init, false);
        #endif
    }
    

    } //end namespace `_ctor_dtor`
    
} //end namespace `_worker_pool`


//C++ test
TEST_CASE(test_cc_worker_pool_subtests[0]) {
    
    #ifndef SC_DEBUG
    _common::release_warning("worker_pool - ctor & dtor");
    #endif

    //run test helper
    _class_helper::cc::test_ctor_dtor<sc::worker_pool>(

        //ctor asserts
        _worker_pool::_ctor_dtor::_ctor_asserts,

        //fixture
        _worker_pool::_ctor_dtor::_fixture,

        //dtor asserts
        _worker_pool::_ctor_dtor::_dtor_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_worker_pool_subtests[0]) {

    #ifndef SC_DEBUG
    _common::release_warning("worker_pool - ctor & dtor");
    #endif

    //run test helper
    _class_helper::c::test_ctor_dtor<sc_worker_pool, sc::worker_pool>(

        //fn pointers
        sc_new_w_pool,
        sc_del_w_pool,


        //ctor asserts
        _worker_pool::_ctor_dtor::_ctor_asserts,

        //fixture
        _worker_pool::_ctor_dtor::_fixture
    );

    return;
}


// -- setup & free workers

namespace _worker_pool {

    namespace _setup {

    static void _populate_pattern_constraints(
        _memcry_helper::args & mcry_args,
        std::variant<
            _opt_helper::cc::args *, _opt_helper::c::args *> args) {

            int ret;
            bool match;

            mc_vm_map * m = &mcry_args.map;
            cm_lst_node * node;
            mc_vm_area * area;

            cm_vct exclusive_areas;


            //initialise constraint vectors
            ret = cm_new_vct(&exclusive_areas, sizeof(cm_lst_node *));
            REQUIRE_EQ(ret, 0);

            //for all areas in the map
            node = m->vm_areas.head;
            for (int i = 0; i < m->vm_areas.len; ++i) {

                //fetch area
                area = MC_GET_NODE_AREA(node);
                REQUIRE_NE(area, nullptr);

                //check if this area's name matches a pattern file
                match = false;
                ret = strcmp(area->basename,
                             _target_helper::pattern_1_basename);
                if (ret == 0) match = true;

                ret = strcmp(area->basename,
                             _target_helper::pattern_2_basename);
                if (ret == 0) match = true;

                //if this is a match, add this area to exclusive areas
                if (match == true) {
                    ret = cm_vct_apd(&exclusive_areas, &node);
                    REQUIRE_EQ(ret, 0);
                }
                
            } //end for all areas in the map


            //add exclusive areas
            if (std::holds_alternative<_opt_helper::cc::args *>(args)) {
                auto args_cast = std::get<_opt_helper::cc::args *>(args);
                ret = args_cast
                          ->opts_ma.set_exclusive_areas(exclusive_areas);
                REQUIRE_EQ(ret, 0);

            } else {
                auto args_cast = std::get<_opt_helper::c::args *>(args);
                ret = sc_opt_ma_set_exclusive_areas(
                          args_cast->opts_ma, &exclusive_areas);
                REQUIRE_EQ(ret, 0);
            }


            //cleanup constraint vectors
            cm_del_vct(&exclusive_areas);
        }
        
    } //end namespace `_setup_free_workers`
    
} //end namespace `_worker_pool`


//C++ test
TEST_CASE(test_cc_worker_pool_subtests[1]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::cc::args opt_args;

    sc::map_area_set ma_set;

    sc::worker_pool w_pool;
    _scan_helper::_fixture_opts opts_fxt;
    _scan_helper::_fixture_scan scan_fxt;

    cm_vct sessions;
    


    // -- fixture

    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 8);

    //manually setup options
    ret = opt_args.opts.set_addr_width((sc::addr_width) sizeof(uintptr_t));
    REQUIRE_EQ(ret, 0);
    ret = opt_args.opts.set_map(&mcry_args.map);
    REQUIRE_EQ(ret, 0);

    //setup scan set options
    _worker_pool::_setup::_populate_pattern_constraints(
        mcry_args, &opt_args);

    //update the set
    ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
    REQUIRE_EQ(ret, 0);

    //set the updated set as the scan set
    ret = opt_args.opts.set_scan_set(&ma_set);
    REQUIRE_EQ(ret, 0);


    // -- subtests

    /*
     *  NOTE: To quickly manage sessions, we create a duplicate of the
     *        `mcry_args` vector containing 8 open sessions, and resize
     *        it to the needed number of sessions.
     */

    //spawn and terminate workers
    SUBCASE(test_cc_worker_pool_subtests[2]) {

        // -- case 1: a single worker

        //setup a single memcry session
        ret = cm_vct_cpy(&sessions, &mcry_args.sessions);
        REQUIRE_EQ(ret, 0);
        ret = cm_vct_rsz(&sessions, 1);
        REQUIRE_EQ(ret, 0);

        //setup the worker pool
        ret = w_pool._setup(opt_args.opts, opts_fxt, scan_fxt, 0b0);
        REQUIRE_EQ(ret, 0);

        //assert state
        _shared::_assert_all_state(
            w_pool,
            1,
            {0},
            {0},
            sessions,
            2,
            true,
            &opt_args.opts,
            &opts_fxt,
            &scan_fxt,
            1,
            1,
            sc::_worker_flag::release_ready,
            0,
            0
        );

        //teardown the setup
        ret = w_pool._teardown();
        REQUIRE_EQ(ret, 0);
        cm_del_vct(&sessions);
    }

    
    // -- fixture teardown

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}
