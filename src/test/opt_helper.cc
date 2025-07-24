//standard template library
#include <cstddef>
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
 *  NOTE: This setup populates the `map` and `sessions` fields of `opts`.
 *        The user can specify further fields to populate by providing
 *        a non-empty setup callback.
 */

// -- C++ interface

void _opt_helper::cc::setup(
    _opt_helper::cc::args & opt_args,
    const _memcry_helper::args & mcry_args,
    std::function<void(_opt_helper::cc::args &)> setup_cb) {

    int ret;


    //assign the MemCry map to ScanCry options
    ret = opt_args.opts.set_map(&mcry_args.map);
    REQUIRE_EQ(ret, 0);

    //assign MemCry sessions to ScanCry options
    ret = opt_args.opts.set_sessions(mcry_args.sessions);
    REQUIRE_EQ(ret, 0);

    //call the setup callback
    setup_cb(opt_args);

    return;
}


void _opt_helper::cc::teardown(_opt_helper::cc::args & opt_args) {

    int ret;


    //reset options
    ret = opt_args.opts.reset();
    REQUIRE_EQ(ret, 0);

    //reset ptrscan options
    ret = opt_args.opts_ptr.reset();
    REQUIRE_EQ(ret, 0);

    //reset map area options
    ret = opt_args.opts_ma.reset();
    REQUIRE_EQ(ret, 0);

    return;
}


// -- C interface

void _opt_helper::c::setup(
    _opt_helper::c::args & opt_args,
    const _memcry_helper::args & mcry_args,
    std::function<void(_opt_helper::c::args &)> setup_cb) {

    int ret;


    //create handles
    opt_args.opts = sc_new_opt();
    REQUIRE_NE(opt_args.opts, nullptr);

    opt_args.opts_ma = sc_new_opt_ma();
    REQUIRE_NE(opt_args.opts_ma, nullptr);

    /* TODO: allocate new pointer scan options */

    
    //assign the MemCry map to ScanCry options
    ret = sc_opt_set_map(opt_args.opts, &mcry_args.map);
    REQUIRE_EQ(ret, 0);

    //assign MemCry sessions to ScanCry options
    ret = sc_opt_set_sessions(opt_args.opts, &mcry_args.sessions);
    REQUIRE_EQ(ret, 0);

    //call the setup callback
    setup_cb(opt_args);

    return;
}


void _opt_helper::c::teardown(_opt_helper::c::args & opt_args) {

    int ret;


    //destroy handles
    sc_del_opt(opt_args.opts);
    sc_del_opt_ma(opt_args.opts_ma);

    return;
}
