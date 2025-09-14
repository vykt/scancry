#pragma once

//standard template library
#include <string>
#include <iostream>

//system headers
#include <unistd.h>

//test target headers
#include "../lib/scancry.h"


namespace _common {

    //misc. constants
    const constexpr enum sc::addr_width cc_addr_width = sc::AW64;
    const constexpr enum sc_addr_width c_addr_width = SC_AW64;
    const constexpr useconds_t thread_wait_usec_time = 100000;

    //test files
    const constexpr char * test_file = "testfile.sc";

    //preset warnings
    const constexpr char * release_warn_tag
        = "release build";
    const constexpr char * release_warn =
        "performing an incomplete test";

    //specify C/C++ interface
    enum test_iface_type {
        CC = 0,
        C = 1
    };

    //ANSI colours
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

    //print title/subtitle
    void title(const enum _common::test_iface_type t_iface,
               const std::string test, const std::string subtest);
    void subtitle(const std::string tag, const std::string subtitle);
    void release_warning(const std::string _class);

    //dump a buffer
    void hexdump(const cm_byte * buf, const size_t sz);
}
