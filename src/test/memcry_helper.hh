#pragma once

//standard template library
#include <vector>

//system headers
#include <unistd.h>

//external libraries
#include <memcry.h>

//local headers
#include "common.hh"


namespace _memcry_helper {

    struct args {

        mc_vm_map map;
        cm_vct /* <mc_session> */ sessions;
    };

    void setup(args & mcry_args, const pid_t pid, const int session_num);
    void teardown(args & mcry_args);
    void print_area(const mc_vm_area * area);
    void print_map(const mc_vm_map * map);

}
