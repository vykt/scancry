#pragma once


extern "C" {
//external
void sc_perror(const char * prefix);
const char * sc_strerror(const int mc_errnum);
}
