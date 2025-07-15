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
 *  NOTE: This setup populates the `map` and `sessions` fields of `opts`.
 *        The user can specify further fields to populate by providing
 *        a non-empty setup callback.
 */

void _opt_helper::setup(_opt_helper::args & opt_args,
                        const _memcry_helper::args & mcry_args,
                        std::function<void()> setup_cb) {

    int ret;


    //assign the MemCry map to ScanCry options
    ret = opt_args.opts.set_map(&mcry_args.map);
    REQUIRE_EQ(ret, 0);

    //assign MemCry sessions to ScanCry options
    ret = opt_args.opts.set_sessions(mcry_args.sessions);
    REQUIRE_EQ(ret, 0);

    //call the setup callback
    setup_cb();

    return;
}


void _opt_helper::teardown(args & opt_args) {

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
