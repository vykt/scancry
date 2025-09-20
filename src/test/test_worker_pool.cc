//standard template library
#include <optional>
#include <variant>
#include <string>
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
    const std::optional<int> sorted_areas_cache_len) {

    #ifdef SC_DEBUG
    REQUIRE_EQ(w_pool.wkr_bundles.len, wkr_bundles_len);
    if (sorted_areas_cache_len.has_value()) {
        REQUIRE_EQ(w_pool.sorted_areas_cache.len, sorted_areas_cache_len);
    } else {
        REQUIRE_EQ(w_pool.sorted_areas_cache.is_init, false);
    }
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


//assert everything
static void _to_sess_ptr_vct(
    const cm_vct & sess_vct, cm_vct & sess_ptr_vct) {

    int ret;
    mc_session * sess;


    //initialise the session pointers vector
    ret = cm_new_vct(&sess_ptr_vct, sizeof(mc_session *));
    REQUIRE_EQ(ret, 0);

    //populate the session pointers vector
    for (int i = 0; i < sess_vct.len; ++i) {
        sess = (mc_session *) cm_vct_get_p(&sess_vct, i);
        REQUIRE_NE(sess, nullptr);
    }

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
        _shared::_assert_worker_pool(w_pool, 0, std::nullopt);

        #ifdef SC_DEBUG
        _shared::_assert_worker_pool_cache(
            w_pool.cache, false, nullptr, nullptr, nullptr);
        _shared::_assert_worker_concurrency(w_pool.concur, 0, 0, 0b0, 0, 0);
        #endif
    }


    //fixture
    static void _fixture(sc::worker_pool & w_pool) {

        #ifdef SC_DEBUG
        _class_helper::vct::setup_stub(w_pool.sorted_areas_cache);
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

    namespace _scan {

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

            //skip areas without a basename
            if (area->basename == nullptr)
                goto _populate_pattern_constraints_skip;

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

            _populate_pattern_constraints_skip:
            //advance iteration
            node = node->next;
            
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


    static void _setup_test(
        sc::worker_pool & w_pool,
        const int session_num,
        const _memcry_helper::args & mcry_args,
        std::function<void(sc::worker_pool & w_pool)> setup_cb,
        std::function<void(
            const sc::worker_pool & w_pool,
            const cm_vct /* mc_session * */ & sessions)> assert_ready_cb,
        std::function<void(sc::worker_pool & w_pool)> teardown_cb) {
        
        int ret;
        cm_vct sessions;


        //setup N memcry sessions
        ret = cm_vct_cpy(&sessions, &mcry_args.session_ptrs);
        REQUIRE_EQ(ret, 0);
        ret = cm_vct_rsz(&sessions, session_num);
        REQUIRE_EQ(ret, 0);

        //setup the worker pool & fixture object
        setup_cb(w_pool);

        //assert ready state
        assert_ready_cb(w_pool, sessions);

        //teardown the setup
        cm_del_vct(&sessions);
        teardown_cb(w_pool);

        return;
    }


    static void _scan_test(
        sc::worker_pool & w_pool,
        const int session_num,
        const _memcry_helper::args & mcry_args,
        const bool do_cancel,
        std::function<void(sc::worker_pool & w_pool)> setup_cb,
        std::function<void(
            const sc::worker_pool & w_pool,
            const cm_vct /* mc_session * */ & sessions)> assert_inprog_cb,
        std::function<void(
            const sc::worker_pool & w_pool,
            const cm_vct /* mc_session * */ & sessions)> assert_fin_cb,
        std::function<void(sc::worker_pool & w_pool)> teardown_cb) {
        
        int ret;
        cm_vct sessions;


        //setup N memcry sessions
        ret = cm_vct_cpy(&sessions, &mcry_args.session_ptrs);
        REQUIRE_EQ(ret, 0);
        ret = cm_vct_rsz(&sessions, session_num);
        REQUIRE_EQ(ret, 0);

        //setup the worker pool & fixture object
        setup_cb(w_pool);


        //start a run
        ret = w_pool._single_run();
        REQUIRE_EQ(ret, 0);

        //assert in-progress state
        assert_inprog_cb(w_pool, sessions);

        //cancel if requested
        if (do_cancel == true) {
            ret = w_pool._cancel();
            REQUIRE_EQ(ret, 0);
        }

        //finish a run
        ret = w_pool._await_run();
        REQUIRE_EQ(ret, 0);

        //assert finished state
        assert_fin_cb(w_pool, sessions);


        //teardown the setup
        cm_del_vct(&sessions);
        teardown_cb(w_pool);

        return;
    }
        
    } //end namespace `_scan`
    
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
    _worker_pool::_scan::_populate_pattern_constraints(
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

    //test spawning and terminating workers
    SUBCASE(test_cc_worker_pool_subtests[2]) {

        // -- case 1: a single worker

        _worker_pool::_scan::_setup_test(
            //non-fn arguments
            w_pool, 1, mcry_args,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);

                return;
            },

            //assert ready state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
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
            },

            //teardown
            [](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
                ret = _w_pool.reset();
                REQUIRE_EQ(ret, 0);
            }
        );

        
        // -- case 2: multiple workers

        _worker_pool::_scan::_setup_test(
            //non-fn arguments
            w_pool, 8, mcry_args,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert ready state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
                    8,
                    {1, 2, 3, 4, 5, 6, 7, 8},
                    {0, 1, 2, 3, 4, 5, 6, 7},
                    sessions,
                    2,
                    true,
                    &opt_args.opts,
                    &opts_fxt,
                    &scan_fxt,
                    8,
                    8,
                    sc::_worker_flag::release_ready,
                    0,
                    0
                );
            },

            //teardown
            [](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
            }
        );

        
        // -- case 3: decrease workers

        _worker_pool::_scan::_setup_test(
            //non-fn arguments
            w_pool, 4, mcry_args,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert ready state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
                    4,
                    {1, 2, 3, 4},
                    {0, 1, 2, 3},
                    sessions,
                    2,
                    true,
                    &opt_args.opts,
                    &opts_fxt,
                    &scan_fxt,
                    4,
                    4,
                    sc::_worker_flag::release_ready,
                    0,
                    0
                );
            },

            //teardown
            [](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
            }
        );

        
        // -- case 4: increase workers

        _worker_pool::_scan::_setup_test(
            //non-fn arguments
            w_pool, 8, mcry_args,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert ready state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
                    8,
                    {1, 2, 3, 4, 9, 10, 11, 12},
                    {0, 1, 2, 3, 4, 5, 6, 7},
                    sessions,
                    8,
                    true,
                    &opt_args.opts,
                    &opts_fxt,
                    &scan_fxt,
                    8,
                    8,
                    sc::_worker_flag::release_ready,
                    0,
                    0
                );
            },

            //teardown
            [](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
            }
        );
    }


    //test applying flags during setup
    SUBCASE(test_cc_worker_pool_subtests[3]) {

        // -- case 1: no flags

        _worker_pool::_scan::_setup_test(
            //non-fn arguments
            w_pool, 2, mcry_args,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert finished state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
                    2,
                    {1, 2},
                    {0, 1},
                    sessions,
                    2,
                    true,
                    &opt_args.opts,
                    &opts_fxt,
                    &scan_fxt,
                    2,
                    2,
                    sc::_worker_flag::release_ready,
                    0,
                    0
                );
            },

            //teardown
            [](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
            }
        );
    

        // -- case 2: keep scan set

        _worker_pool::_scan::_setup_test(
            //non-fn arguments
            w_pool, 2, mcry_args,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert finished state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
                    2,
                    {1, 2},
                    {0, 1},
                    sessions,
                    2,
                    true,
                    &opt_args.opts,
                    &opts_fxt,
                    &scan_fxt,
                    2,
                    2,
                    sc::_worker_flag::release_ready,
                    0,
                    0
                );
            },

            //teardown
            [](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
            }
        );
    }


    //test scanning
    SUBCASE(test_cc_worker_pool_subtests[4]) {

        _common::concur_warning("worker_pool - scan");

        // -- case 1: single-threaded scan - every byte

        _worker_pool::_scan::_scan_test(
            //non-fn arguments
            w_pool, 1, mcry_args, false,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {

                //setup the fixture scan object
                scan_fxt.set_mod(1);
                scan_fxt.set_do_checks(true);

                //setup the work pool
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert in-prog state
            [](const sc::worker_pool & _w_pool,
               const cm_vct /* mc_session * */ & sessions) { return; },

            //assert finished state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
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
            },

            //teardown
            [&scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
                ret = scan_fxt.reset();
                REQUIRE_EQ(ret, 0);
            }
        );

        
        // -- case 2: single-threaded scan - every 4th byte

        _worker_pool::_scan::_scan_test(
            //non-fn arguments
            w_pool, 1, mcry_args, false,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {

                //setup the fixture scan object
                scan_fxt.set_mod(4);
                scan_fxt.set_do_checks(true);

                //setup the work pool
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert in-prog state
            [](const sc::worker_pool & _w_pool,
               const cm_vct /* mc_session * */ & sessions) { return; },

            //assert finished state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
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
            },

            //teardown
            [&scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
                ret = scan_fxt.reset();
                REQUIRE_EQ(ret, 0);
            }
        );

        
        // -- case 3: single-threaded scan - crash

        _worker_pool::_scan::_scan_test(
            //non-fn arguments
            w_pool, 1, mcry_args, false,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {

                //setup the fixture scan object
                scan_fxt.set_do_crash_all(true);
                scan_fxt.set_do_checks(true);

                //setup the work pool
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert in-prog state
            [](const sc::worker_pool & _w_pool,
               const cm_vct /* mc_session * */ & sessions) { return; },

            //assert finished state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
                    1,
                    {0},
                    {0},
                    sessions,
                    2,
                    true,
                    &opt_args.opts,
                    &opts_fxt,
                    &scan_fxt,
                    0,
                    0,
                    sc::_worker_flag::error | sc::_worker_flag::exit,
                    0,
                    0
                );
            },

            //teardown
            [&scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
                ret = scan_fxt.reset();
                REQUIRE_EQ(ret, 0);
            }
        );


        // -- case 4: single-threaded scan - cancel run

        _worker_pool::_scan::_scan_test(
            //non-fn arguments
            w_pool, 1, mcry_args, true,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {

                //setup the fixture scan object
                scan_fxt.set_do_delay(true);
                scan_fxt.set_do_checks(true);

                //setup the work pool
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert in-prog state
            [](const sc::worker_pool & _w_pool,
               const cm_vct /* mc_session * */ & sessions) {},

            //assert finished state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
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
                    sc::_worker_flag::cancel,
                    0,
                    0
                );
            },

            //teardown
            [&scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
                ret = _w_pool.reset();
                REQUIRE_EQ(ret, 0);
                ret = scan_fxt.reset();
                REQUIRE_EQ(ret, 0);
            }
        );


        /*
         * -------------------
         */


        // -- case 5: multi-threaded scan - every byte

        _worker_pool::_scan::_scan_test(
            //non-fn arguments
            w_pool, 2, mcry_args, false,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {

                //setup the fixture scan object
                scan_fxt.set_mod(1);
                scan_fxt.set_do_checks(true);

                //setup the work pool
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert in-prog state
            [](const sc::worker_pool & _w_pool,
               const cm_vct /* mc_session * */ & sessions) { return; },

            //assert finished state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
                    2,
                    {0},
                    {0},
                    sessions,
                    2,
                    true,
                    &opt_args.opts,
                    &opts_fxt,
                    &scan_fxt,
                    2,
                    2,
                    sc::_worker_flag::release_ready,
                    0,
                    0
                );
            },

            //teardown
            [&scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
                ret = scan_fxt.reset();
                REQUIRE_EQ(ret, 0);
            }
        );

        
        // -- case 6: multi-threaded scan - every 4th byte

        _worker_pool::_scan::_scan_test(
            //non-fn arguments
            w_pool, 2, mcry_args, false,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {

                //setup the fixture scan object
                scan_fxt.set_mod(4);
                scan_fxt.set_do_checks(true);

                //setup the work pool
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert in-prog state
            [](const sc::worker_pool & _w_pool,
               const cm_vct /* mc_session * */ & sessions) { return; },

            //assert finished state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
                    2,
                    {0},
                    {0},
                    sessions,
                    2,
                    true,
                    &opt_args.opts,
                    &opts_fxt,
                    &scan_fxt,
                    2,
                    2,
                    sc::_worker_flag::release_ready,
                    0,
                    0
                );
            },

            //teardown
            [&scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
                ret = scan_fxt.reset();
                REQUIRE_EQ(ret, 0);
            }
        );

        
        // -- case 7: multi-threaded scan - crash one

        _worker_pool::_scan::_scan_test(
            //non-fn arguments
            w_pool, 2, mcry_args, false,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {

                //setup the fixture scan object
                scan_fxt.set_do_crash_one(true);
                scan_fxt.set_do_checks(true);

                //setup the work pool
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert in-prog state
            [](const sc::worker_pool & _w_pool,
               const cm_vct /* mc_session * */ & sessions) { return; },

            //assert finished state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
                    0,
                    {0},
                    {0},
                    sessions,
                    2,
                    true,
                    &opt_args.opts,
                    &opts_fxt,
                    &scan_fxt,
                    0,
                    0,
                    sc::_worker_flag::error | sc::_worker_flag::exit,
                    0,
                    0
                );
            },

            //teardown
            [&scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
                ret = scan_fxt.reset();
                REQUIRE_EQ(ret, 0);
            }
        );


        // -- case 8: multi-threaded scan - crash all

        _worker_pool::_scan::_scan_test(
            //non-fn arguments
            w_pool, 2, mcry_args, false,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {

                //setup the fixture scan object
                scan_fxt.set_do_crash_all(true);
                scan_fxt.set_do_checks(true);

                //setup the work pool
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert in-prog state
            [](const sc::worker_pool & _w_pool,
               const cm_vct /* mc_session * */ & sessions) { return; },

            //assert finished state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
                    0,
                    {0},
                    {0},
                    sessions,
                    2,
                    true,
                    &opt_args.opts,
                    &opts_fxt,
                    &scan_fxt,
                    0,
                    0,
                    sc::_worker_flag::error | sc::_worker_flag::exit,
                    0,
                    0
                );
            },

            //teardown
            [&scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
                ret = scan_fxt.reset();
                REQUIRE_EQ(ret, 0);
            }
        );


        // -- case 9: multi-threaded scan - cancel run

        _worker_pool::_scan::_scan_test(
            //non-fn arguments
            w_pool, 2, mcry_args, true,

            //setup
            [&opt_args, &opts_fxt, &scan_fxt](sc::worker_pool & _w_pool) {

                //setup the fixture scan object
                scan_fxt.set_do_delay(true);
                scan_fxt.set_do_checks(true);

                //setup the work pool
                int ret = _w_pool._setup(
                              opt_args.opts, opts_fxt, scan_fxt, 0b0);
                REQUIRE_EQ(ret, 0);
                return;
            },

            //assert in-prog state
            [](const sc::worker_pool & _w_pool,
               const cm_vct /* mc_session * */ & sessions) {},

            //assert finished state
            [&opt_args, &opts_fxt, &scan_fxt](
                const sc::worker_pool & _w_pool,
                const cm_vct /* mc_session * */ & sessions) {

                _shared::_assert_all_state(
                    _w_pool,
                    2,
                    {0},
                    {0},
                    sessions,
                    2,
                    true,
                    &opt_args.opts,
                    &opts_fxt,
                    &scan_fxt,
                    2,
                    2,
                    sc::_worker_flag::cancel,
                    0,
                    0
                );
            },

            //teardown
            [&scan_fxt](sc::worker_pool & _w_pool) {
                int ret = _w_pool._teardown();
                REQUIRE_EQ(ret, 0);
                ret = _w_pool.reset();
                REQUIRE_EQ(ret, 0);
                ret = scan_fxt.reset();
                REQUIRE_EQ(ret, 0);
            }
        );
    }

    
    // -- fixture teardown

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}
