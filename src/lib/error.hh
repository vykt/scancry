#pragma once

//standard template library
#include <exception>


//internal
void exception_sc_errno(const std::exception & excp);


extern "C" {
//external
void ps_perror(const char * prefix);
const char * ps_strerror(const int mc_errnum);
}
