/* specify a custom main() implementation in doctest */
#include <cstdint>
#define DOCTEST_CONFIG_IMPLEMENT


//standard template library
#include <iostream>

//C standard library
#include <stdint.h>

//system headers
#include <unistd.h>
#include <getopt.h>

//external libraries
#include <cmore.h>
#include <doctest/doctest.h>

//local headers
#include "filters.hh"



//tests bitmask
const constexpr uint16_t cc_opt_test      = 0x1;
const constexpr uint16_t c_opt_test       = 0x2;
const constexpr uint16_t cc_scan_set_test = 0x4;
const constexpr uint16_t c_scan_set_test  = 0x8;



//determine which tests to run
static uint16_t _get_test_mode(int argc, char ** argv) {

    const struct option long_opts[] = {
        {"all", no_argument, NULL, 'a'},
        {"cc-opt", no_argument, NULL, 'o'},
        {"c-opt", no_argument, NULL, 'O'},
        {"cc-scan_set", no_argument, NULL, 's'},
        {"c-scan_set", no_argument, NULL, 'S'},
        {0,0,0,0}
    };

    int opt;
    uint16_t test_mask = 0;

    
    while((opt = getopt_long(argc, argv, "aoOsS", long_opts, NULL)) != -1 
          && opt != 0) {

        //determine parsed argument
        switch (opt) {

            case 'a':
                test_mask = UINT16_MAX;
                break;

            case 'o':
                test_mask |= cc_opt_test;
                break;
                
            case 'O':
                test_mask |= c_opt_test;
                break;

            case 's':
                test_mask |= cc_scan_set_test;
                break;
                
            case 'S':
                test_mask |= c_scan_set_test;
                break;
        }
    }

    return test_mask;
}


//run unit tests
static void _run_unit_tests(cm_byte test_mask) {

    int ret;
    doctest::Context context;


    //add selected filters
    if (test_mask & cc_opt_test)      add_cc_opt(context); 
    if (test_mask & c_opt_test)       add_c_opt(context); 
    if (test_mask & cc_scan_set_test) add_cc_scan_set(context); 
    if (test_mask & c_scan_set_test)  add_c_scan_set(context); 

    //run selected tests
    ret = context.run();

    return;
}


//dispatch tests
int main(int argc, char ** argv) {

    uint16_t test_mask = _get_test_mode(argc, argv);
    _run_unit_tests(test_mask);
    
    return 0;
}
