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
#include "common.hh"
#include "filters.hh"



//tests bitmask
const constexpr uint16_t cc_opt_test          = 1 << 0;
const constexpr uint16_t c_opt_test           = 1 << 1;
const constexpr uint16_t cc_opt_ptrscan_test  = 1 << 2;
const constexpr uint16_t c_opt_ptrscan_test   = 1 << 3;
const constexpr uint16_t cc_map_area_set_test = 1 << 4;
const constexpr uint16_t c_map_area_set_test  = 1 << 5;
const constexpr uint16_t cc_worker_pool_test  = 1 << 6;
const constexpr uint16_t c_worker_pool_test   = 1 << 7;
const constexpr uint16_t cc_serialiser_test   = 1 << 8;
const constexpr uint16_t c_serialiser_test    = 1 << 9;
const constexpr uint16_t cc_ptrscan_test      = 1 << 10;
const constexpr uint16_t c_ptrscan_test       = 1 << 11;


//determine which tests to run
static uint16_t _get_test_mode(int argc, char ** argv) {

    const struct option long_opts[] = {
        {"colour", no_argument, NULL, 'c'},
        {"all", no_argument, NULL, 'a'},
        {"cc-opt", no_argument, NULL, 'o'},
        {"c-opt", no_argument, NULL, 'O'},
        {"cc-opt_ptr", no_argument, NULL, 'p'},
        {"c-opt_ptr", no_argument, NULL, 'P'},
        {"cc-map_area_set", no_argument, NULL, 's'},
        {"c-map_area_set", no_argument, NULL, 'S'},
        {"cc-worker_pool", no_argument, NULL, 'w'},
        {"c-worker_pool", no_argument, NULL, 'W'},
        {"cc-serialiser", no_argument, NULL, 'r'},
        {"c-serialiser", no_argument, NULL, 'R'},
        {"cc-ptrscan", no_argument, NULL, 'q'},
        {"c-ptrscan", no_argument, NULL, 'Q'},
        {0,0,0,0}
    };

    int opt;
    uint16_t test_mask = 0;

    
    while((opt = getopt_long(argc, argv, "caoOpPsSwWrRqQ", long_opts, NULL)) != -1 && opt != 0) {

        //determine parsed argument
        switch (opt) {

            case 'c':
                use_colour = true;
                break;

            case 'a':
                test_mask = UINT16_MAX;
                break;

            case 'o':
                test_mask |= cc_opt_test;
                break;
                
            case 'O':
                test_mask |= c_opt_test;
                break;

            case 'p':
                test_mask |= cc_opt_ptrscan_test;
                break;
                
            case 'P':
                test_mask |= c_opt_ptrscan_test;
                break;

            case 's':
                test_mask |= cc_map_area_set_test;
                break;
                
            case 'S':
                test_mask |= c_map_area_set_test;
                break;

            case 'w':
                test_mask |= cc_worker_pool_test;
                break;

            case 'W':
                test_mask |= c_worker_pool_test;
                break;

            case 'r':
                test_mask |= cc_serialiser_test;
                break;

            case 'R':
                test_mask |= c_serialiser_test;
                break;

            case 'q':
                test_mask |= cc_ptrscan_test;
                break;

            case 'Q':
                test_mask |= c_ptrscan_test;
                break;
        }
    }

    return test_mask;
}


//run unit tests
static void _run_unit_tests(uint16_t test_mask) {

    int ret;
    doctest::Context context;


    //add selected filters
    if (test_mask & cc_opt_test) add_cc_opt(context); 
    if (test_mask & c_opt_test)  add_c_opt(context);
    if (test_mask & cc_opt_ptrscan_test) add_cc_opt_ptr(context);
    if (test_mask & c_opt_ptrscan_test)  add_c_opt_ptr(context);
    if (test_mask & cc_map_area_set_test) add_cc_map_area_set(context); 
    if (test_mask & c_map_area_set_test)  add_c_map_area_set(context); 
    if (test_mask & cc_worker_pool_test) add_cc_worker_pool(context);
    if (test_mask & c_worker_pool_test) add_c_worker_pool(context);
    if (test_mask & cc_serialiser_test) add_cc_serialiser(context);
    if (test_mask & c_serialiser_test) add_c_serialiser(context);
    if (test_mask & cc_ptrscan_test) add_cc_ptrscan(context);
    if (test_mask & c_ptrscan_test) add_c_ptrscan(context);

    //run selected tests
    ret = context.run();

    return;
}


//dispatch tests
int main(int argc, char ** argv) {

    //initialise miscellaneous state
    use_colour = false;

    //setup & run tests
    uint16_t test_mask = _get_test_mode(argc, argv);
    _run_unit_tests(test_mask);
    
    return 0;
}
