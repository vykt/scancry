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

    namespace cc {
    
        struct args {
            sc::opt opts;
            sc::opt_ptrscan opts_ptr;
            sc::opt_map_area opts_ma;
        };

        void setup(_opt_helper::cc::args & opt_args,
                   const _memcry_helper::args & mcry_args,
                   std::function<void(_opt_helper::cc::args &)> setup_cb);
        void teardown(_opt_helper::cc::args & opt_args);
    }


    namespace c {

        struct args {
            sc_opt * opts;
            sc_opt_ptrscan * opts_ptr;
            sc_opt_map_area * opts_ma;
        };

        void setup(_opt_helper::c::args & opt_args,
                   const _memcry_helper::args & mcry_args,
                   std::function<void(_opt_helper::c::args &)> setup_cb);
        void teardown(_opt_helper::c::args & opt_args);
    }

}
