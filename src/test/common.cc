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
