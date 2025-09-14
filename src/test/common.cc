//standard template library
#include <iostream>
#include <iomanip>

//local headers
#include "common.hh"


//toggle colour mode
bool _common::use_colour;


//print a test title 
void _common::title(const enum test_iface_type t_iface,
                    const std::string test, const std::string subtest) {

    //print the language tag
    if (use_colour == true) {
        std::cout << "[" << ((t_iface == CC) ? colour::RED : colour::GRAY)
                  << ((t_iface == CC) ? "C++" : "C") << colour::RESET << "] ";
    } else {
        std::cout << ((t_iface == CC) ? "[C++] " : "[C] ");
    }

    //print the test & subtest
    if (use_colour == true) {
        std::cout << colour::BLUE << test << colour::RESET
                  << " - " << colour::CYAN << subtest << colour::RESET;
    } else {
        std::cout << test << " - " << subtest;
    }

    std::cout << std::endl;
    return;
}


//print a test subtitle
void _common::subtitle(const std::string tag, const std::string subtitle) {
    
    //print tag
    if (use_colour == true) {
        std::cout << "<" << colour::YELLOW << tag << colour::RESET << "> "
                  << colour::GREEN << subtitle << colour::RESET
                  << std::endl;
    } else {
        std::cout << "<" << tag << "> " << subtitle << std::endl;
    }
    
    return;
}


//print a warning
void _common::release_warning(const std::string _class) {

    //print tag
    if (use_colour == true) {
        std::cout << "[" << colour::RED << _common::release_warn_tag
                  << colour::RESET << "]<" << colour::YELLOW
                  << _class << colour::RESET << "> "
                  << _common::release_warn << std::endl;
        
    } else {
        std::cout << "[" << _common::release_warn_tag << "]<"
                  << _class << "> "
                  << _common::release_warn << std::endl;
    }
}


//dump buffer contents 16 bytes per line
void _common::hexdump(const cm_byte * buf, const size_t sz) {

    const constexpr int line_bytes = 16;
    int line_num = ((sz - 1) / 16) + 1;
    off_t buf_off = 0x0;

    //for every line
    std::cout << std::hex;
    for (const cm_byte * line_start = buf;
         line_start < (line_start + (line_bytes * line_num));
         line_start += line_bytes) {

        //print buffer offset
        std::cout << "0x" << std::setw(8)
                  << std::setfill('0') << buf_off << ":";

        //for every byte on a line
        for (const cm_byte * line_byte = line_start;
             line_byte < (line_start + line_bytes);
             line_byte += 1) {

            //display bytes
            std::cout << (((uintptr_t) line_byte % 2) ? "" : " ")
                      << *line_byte;
        }
        std::cout << std::endl;
    }
    
    std::cout << std::dec;
    return;
}
