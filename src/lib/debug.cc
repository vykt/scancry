#pragma once

//C standard library
#include <cstdio>
#include <cstdarg>

//local headers
#include "debug.hh"


//local headers
void dbg::print_trace(const char * fmt, ...) noexcept {

    //setup variable arguments
    va_list va_args;
    va_start(va_args, fmt);

    //print trace prefix
    std::printf("[scry][trace] ");
    std::vprintf(fmt, va_args);

    //cleanup variable arguments
    va_end(va_args);

    return;
}
