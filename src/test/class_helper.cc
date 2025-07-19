//C standard library
#include <cstdlib>

//external libraries
#include "cmore.h"
#include <doctest/doctest.h>

//local headers
#include "class_helper.hh"


void _class_helper::setup_vct_stub(cm_vct & vct) {

    //allocate a data buffer
    vct.data = malloc(0x10);
    REQUIRE_NE(vct.data, nullptr);

    //set vector as initialised
    vct.is_init = true;

    return;
}
