#pragma once

//system headers
#include <unistd.h>

//test target headers
#include "../lib/scancry.h"


const constexpr enum sc::addr_width test_cc_addr_width = sc::AW64;
const constexpr enum sc_addr_width test_c_addr_width = AW64;
const constexpr useconds_t thread_wait_usec_time = 100000;
