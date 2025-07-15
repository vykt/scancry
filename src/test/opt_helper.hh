#pragma once

//standard template library
#include <functional>

//system headers
#include <unistd.h>

//external libraries
#include <memcry.h>

//local headers
#include "common.hh"
#include "memcry_helper.hh"

//test target headers
#include "../lib/scancry.h"



namespace _opt_helper {

    struct args {

        sc::opt opts;
        sc::opt_ptr opts_ptr;
        sc::opt_map_area opts_ma; 
    };

    void setup(_opt_helper::args & opt_args,
               const _memcry_helper::args & mcry_args,
               std::function<void()> setup_cb);
    void teardown(_opt_helper::args & opt_args);
}
