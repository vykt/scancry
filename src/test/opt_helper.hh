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
    sc::opt_ptrscan opts_ptrscan;
    sc::map_area_set ma_set;

    args(sc::addr_width aw) : opts(sc::opt(aw)) {}
};

void setup(args & opt_args,
           _memcry_helper::args & mcry_args,
           std::function<void()> setup_cb);
void teardown(args & opt_args);

}
