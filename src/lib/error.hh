#pragma once

//standard template library
#include <bits/c++config.h>
#include <exception>
#include <string>


//internal
void exception_sc_errno(const std::exception & excp);
void print_warning(const std::string msg);
void print_critical(const std::string msg);


extern "C" {
//external
void ps_perror(const char * prefix);
const char * ps_strerror(const int mc_errnum);
}
