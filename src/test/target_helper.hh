#pragma once

//system headers
#include <unistd.h>

//test target headers
#include "../lib/scancry.h"



/*
 *  NOTE: The target helper is directly tied to `target/unit_target.c`,
 *        changing the target helper means you likely have to change the
 *        unit target and vice versa.
 */

namespace _target_helper {

//the state of the target's memory map
enum target_map_state {
    UNINIT, //waiting for child to start
    INIT,   //child initialisation complete
};

//target metadata
const constexpr char * target_name = "unit_target";
const constexpr char * pattern_1_basename = "pattern1.bin";
const constexpr char * pattern_2_basename = "pattern2.bin";


//target helpers
int clean_targets();
pid_t start_target();
void end_target(pid_t pid);

}
