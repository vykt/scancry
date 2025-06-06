#pragma once

//standard template library
#include <string>
#include <iostream>

//system headers
#include <unistd.h>

//test target headers
#include "../lib/scancry.h"


const constexpr enum sc::addr_width test_cc_addr_width = sc::AW64;
const constexpr enum sc_addr_width test_c_addr_width = AW64;
const constexpr useconds_t thread_wait_usec_time = 100000;


extern bool use_colour;
namespace colour {

    const constexpr char * RESET   = "\033[0m";
    const constexpr char * RED     = "\033[31m";
    const constexpr char * GREEN   = "\033[32m";
    const constexpr char * YELLOW  = "\033[33m";
    const constexpr char * BLUE    = "\033[34m";
    const constexpr char * MAGENTA = "\033[35m";
    const constexpr char * CYAN    = "\033[36m";
    const constexpr char * GRAY    = "\033[90m";
};


enum test_iface {
    CC = 0,
    C = 1
};


void title(const enum test_iface t_iface,
           const std::string test, const std::string subtest);
void subtitle(const std::string tag, const std::string subtitle);
