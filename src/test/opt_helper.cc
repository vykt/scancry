//standard template library
#include <optional>
#include <vector>
#include <functional>

//system headers
#include <unistd.h>

//external libraries
#include <memcry.h>
#include <doctest/doctest.h>

//local headers
#include "common.hh"
#include "opt_helper.hh"
#include "memcry_helper.hh"



/*
 *  NOTE: The `setup()` function will automatically populate the `map` and
 *        `sessions` members of the ScanCry `opts` object in `opt_args`.
 *        Before generating the map area set, the `setup_cb()` is called. It  
 *        is here that you can pass a lambda to set members of various
 *        option objects (e.g.: `exclusive_areas`).
 */

void _opt_helper::setup(_opt_helper::args & opt_args,
                        _memcry_helper::args & mcry_args,
                        std::function<void()> setup_cb) {

    int ret;

    //assign the MemCry map to ScanCry options
    ret = opt_args.opts.set_map(&mcry_args.map);
    CHECK_EQ(ret, 0);

    //assign MemCry sessions to ScanCry options
    std::vector<const mc_session *> session_vct;
    for (auto iter = mcry_args.sessions.cbegin();
         iter != mcry_args.sessions.cend(); ++iter) {
        session_vct.push_back(&(*iter));
    }

    ret = opt_args.opts.set_sessions(session_vct);
    CHECK_EQ(ret, 0);

    //call the setup callback
    setup_cb();

    //create a map area set
    ret = opt_args.ma_set.update_set(opt_args.opts);
    CHECK_EQ(ret, 0);

    return;
}


void _opt_helper::teardown(args & opt_args) {

    int ret;


    //reset options
    ret = opt_args.opts.reset();
    CHECK_EQ(ret, 0);

    //reset ptrscan options
    ret = opt_args.opts_ptrscan.reset();
    CHECK_EQ(ret, 0);

    //reset map area set
    ret = opt_args.ma_set.reset();
    CHECK_EQ(ret, 0);

    return;
}
