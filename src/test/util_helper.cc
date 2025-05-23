//standard template library
#include <iostream>
#include <iomanip>

//external libraries
#include <cmore.h>
#include <memcry.h>

//local headers
#include "common.hh"
#include "util_helper.hh"


void _util_helper::print_area(mc_vm_area * area) {

    char str_buf[5];

    /*
     *  NOTE: Format: <start_addr> - <end_addr> - <perms> - <basename:12>
     */
     
    std::cout << std::hex;
    std::cout << "0x" << area->start_addr << " - 0x" << area->end_addr;
    std::cout << " | " << std::string(str_buf) << " | ";
    if (area->basename != nullptr) {
        std::cout << std::left << std::setw(10);
        std::cout << std::string(area->basename).substr(0, 12);
    }
    std::cout << std::endl; 
}
