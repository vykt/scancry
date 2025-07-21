//C standard library
#include <cstdlib>

//external libraries
#include "cmore.h"
#include <doctest/doctest.h>

//local headers
#include "class_helper.hh"


void _class_helper::vct::setup_stub(cm_vct & vct) {

    int num;


    //allocate a data buffer
    vct.data = malloc(0x10);
    REQUIRE_NE(vct.data, nullptr);

    //set a false number of elements
    num = (rand() % 9) + 8;
    vct.len = num;
    vct.sz = num;

    //set vector as initialised
    vct.is_init = true;

    return;
}
