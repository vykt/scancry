#pragma once

//standard template library
#include <optional>

//system headers
#include <unistd.h>

//test target headers
#include "../lib/scancry.h"



/*
 * The target helper is directly tied to `target/unit_target.c`, changing
 * the target helper means you likely have to change the unit target and 
 * vice versa.
 */


//the state of the target's memory map
enum target_map_state {
    UNINIT, //waiting for child to start
    INIT,   //child initialisation complete
};

//target metadata
const constexpr char * target_name = "unit_target";


//target helpers
std::optional<int> clean_targets();
pid_t start_target();
void end_target(pid_t pid);
