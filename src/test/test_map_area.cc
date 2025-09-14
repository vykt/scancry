//standard template library
#include <string>
#include <variant>
#include <iostream>
#include <functional>

//C standard library
#include <cstring>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <doctest/doctest.h>

//system headers
#include <unistd.h>

//local headers
#include "filters.hh"
#include "common.hh"
#include "class_helper.hh"
#include "memcry_helper.hh"
#include "opt_helper.hh"
#include "target_helper.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/map_area.hh"



/*
 *  --- [SHARED] ---
 */

namespace _shared {

//compare cmore node pointers (vector)
static void _cm_node_elem_eq(cm_lst_node * const & elem_0,
                             cm_lst_node * const & elem_1) {

    REQUIRE_EQ(elem_0, elem_1);
    return;    
}


//compare C++ address ranges (vector)
static void _cc_addr_range_elem_eq(const sc::addr_range & addr_range_0,
                                   const sc::addr_range & addr_range_1) {

    REQUIRE_EQ(addr_range_0.get_start_addr(),
               addr_range_1.get_start_addr());

    REQUIRE_EQ(addr_range_0.get_end_addr(),
               addr_range_1.get_end_addr());

    return;
}


//compare C address ranges (vector)
static void _c_addr_range_elem_eq(const sc_addr_range & addr_range_0,
                                  const sc_addr_range & addr_range_1) {

    REQUIRE_EQ(addr_range_0.start_addr,
               addr_range_1.start_addr);

    REQUIRE_EQ(addr_range_0.end_addr,
               addr_range_1.end_addr);

    return;
}


//compare cmore node pointers (red-black tree)
static void _cm_node_key_data_eq(
    cm_lst_node * const & node_0, mc_vm_area * const & area_0,
    cm_lst_node * const & node_1, mc_vm_area * const & area_1) {

    REQUIRE_EQ(node_0, node_1);
    REQUIRE_EQ(area_0, area_1);

    return;
}


//fully populate a `opt_map_area`
static void _populate_opt_map_area(sc::opt_map_area & opts_ma) {

    int ret;

    cm_vct new_nodes;
    cm_vct new_addr_ranges;
    cm_byte new_access = 0b111;

    cm_lst_node * nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };
    sc::addr_range addr_ranges[4] = {
        sc::addr_range(0x1000, 0x2000),
        sc::addr_range(0x2000, 0x3000),
        sc::addr_range(0x3000, 0x4000),
        sc::addr_range(0x4000, 0x5000)
    };



    //setup new vectors
    _class_helper::vct::populate<cm_lst_node *>(
        new_nodes, nodes, 4);
    _class_helper::vct::populate<sc::addr_range>(
        new_addr_ranges, addr_ranges, 4);


    //call setters
    ret = opts_ma.set_omit_areas(new_nodes);
    REQUIRE_EQ(ret, 0);

    ret = opts_ma.set_omit_objs(new_nodes);
    REQUIRE_EQ(ret, 0);
    
    ret = opts_ma.set_exclusive_areas(new_nodes);
    REQUIRE_EQ(ret, 0);

    ret = opts_ma.set_exclusive_objs(new_nodes);
    REQUIRE_EQ(ret, 0);

    ret = opts_ma.set_omit_addr_ranges(new_addr_ranges);
    REQUIRE_EQ(ret, 0);

    ret = opts_ma.set_exclusive_addr_ranges(new_addr_ranges);
    REQUIRE_EQ(ret, 0);

    ret = opts_ma.set_access(new_access);
    REQUIRE_EQ(ret, 0);

    //cleanup
    cm_del_vct(&new_nodes);
    cm_del_vct(&new_addr_ranges);

    return;
}


namespace _opt_map_area {

    //data for setters of `cm_vct<cm_lst_node *>`
    static cm_vct _new_nodes_vct;
    static cm_lst_node * _nodes[4] = {
        (cm_lst_node *) 0x10101010,
        (cm_lst_node *) 0x20202020,
        (cm_lst_node *) 0x30303030,
        (cm_lst_node *) 0x40404040
    };


    //setup new nodes vector
    static void _setup_new_nodes_vct() {

        _class_helper::vct::populate<cm_lst_node *>(
            _new_nodes_vct, _nodes, 4);    
        return;
    }


    //teardown new nodes vector
    static void _teardown_new_nodes_vct() {

        cm_del_vct(&_new_nodes_vct);
        return;
    }

} //end namespace `_opt_map_area`
    
} //end namespace `_shared`



/*
 *  --- [OPT_MAP_AREA] ---
 */

// -- ctor & dtor

namespace _opt_map_area {

    namespace _ctor_dtor {

    //ctor asserts
    static void _ctor_asserts(const sc::opt_map_area & opts_ma) {

        //assert constructor succeeded
        REQUIRE_EQ(opts_ma._get_ctor_failed(), false);

        #ifdef SC_DEBUG
        //assert vector attributes
        REQUIRE_EQ(opts_ma.omit_areas.is_init, false);
        REQUIRE_EQ(opts_ma.omit_objs.is_init, false);
        REQUIRE_EQ(opts_ma.exclusive_areas.is_init, false);
        REQUIRE_EQ(opts_ma.exclusive_objs.is_init, false);
        REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, false);
        REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, false);

        //assert access
        REQUIRE_EQ(opts_ma.access, sc::val_unset::access);
        #endif
    }

    //fixture
    static void _fixture(sc::opt_map_area & opts_ma) {

        #ifdef SC_DEBUG
        //(fixture) setup vectors
        _class_helper::vct::setup_stub(opts_ma.omit_areas);
        _class_helper::vct::setup_stub(opts_ma.omit_objs);
        _class_helper::vct::setup_stub(opts_ma.exclusive_areas);
        _class_helper::vct::setup_stub(opts_ma.exclusive_objs);
        _class_helper::vct::setup_stub(opts_ma.omit_addr_ranges);
        _class_helper::vct::setup_stub(opts_ma.exclusive_addr_ranges);
        #endif
    }

    //dtor asserts
    static void _dtor_asserts(const sc::opt_map_area & opts_ma) {

        #ifdef SC_DEBUG
        //assert vector attributes
        REQUIRE_EQ(opts_ma.omit_areas.is_init, false);
        REQUIRE_EQ(opts_ma.omit_objs.is_init, false);
        REQUIRE_EQ(opts_ma.exclusive_areas.is_init, false);
        REQUIRE_EQ(opts_ma.exclusive_objs.is_init, false);
        REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, false);
        REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, false);
        #endif
    }        

    } //end namespace `_ctor_dtor`
    
} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[0]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - ctor & dtor");
    #endif

    //run test helper
    _class_helper::cc::test_ctor_dtor<sc::opt_map_area>(

        //ctor asserts
        _opt_map_area::_ctor_dtor::_ctor_asserts,        

        //fixture
        _opt_map_area::_ctor_dtor::_fixture,

        //dtor asserts
        _opt_map_area::_ctor_dtor::_dtor_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[0]) {
    
    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - ctor & dtor");
    #endif

    //run test helper
    _class_helper::c::test_ctor_dtor<
        sc_opt_map_area, sc::opt_map_area>(

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,


        //ctor asserts
        _opt_map_area::_ctor_dtor::_ctor_asserts,
        
        //fixture
        _opt_map_area::_ctor_dtor::_fixture
    );

    return;
}


// -- `omit_areas` setter & getter

namespace _opt_map_area {

    namespace _omit_areas {

    //setter asserts
    static void _setter_asserts(const sc::opt_map_area & opts_ma) {

        #ifdef SC_DEBUG
        _class_helper::vct::assert_eq<cm_lst_node *>(
            _shared::_opt_map_area::_new_nodes_vct,
            opts_ma.omit_areas,
            _shared::_cm_node_elem_eq);
        #endif

        return;
    }

    } //end namespace `_omit_areas`

} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[1]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - omit_areas");
    #endif

    //setup new omit areas
    _shared::_opt_map_area::_setup_new_nodes_vct();

    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        _shared::_opt_map_area::_new_nodes_vct,
        &sc::opt_map_area::set_omit_areas,
        &sc::opt_map_area::get_omit_areas,

        //setter assert
        _opt_map_area::_omit_areas::_setter_asserts,

        //element assert
        _shared::_cm_node_elem_eq
    );

    //delete new omit areas
    _shared::_opt_map_area::_teardown_new_nodes_vct();

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[1]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - omit_areas");
    #endif

    //setup new omit areas
    _shared::_opt_map_area::_setup_new_nodes_vct();

    //run test helper
    _class_helper::c::test_vct_setter_getter<
        sc_opt_map_area, sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        _shared::_opt_map_area::_new_nodes_vct,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_omit_areas,
        sc_opt_ma_get_omit_areas,


        //setter assert
        _opt_map_area::_omit_areas::_setter_asserts,

        //element assert
        _shared::_cm_node_elem_eq
    );

    //delete new omit areas
    _shared::_opt_map_area::_teardown_new_nodes_vct();

    return;
}


// -- `omit_objs` setter & getter

namespace _opt_map_area {

    namespace _omit_objs {

    //setter asserts
    static void _setter_asserts(const sc::opt_map_area & opts_ma) {

        #ifdef SC_DEBUG
        _class_helper::vct::assert_eq<cm_lst_node *>(
            _shared::_opt_map_area::_new_nodes_vct,
            opts_ma.omit_objs,
            _shared::_cm_node_elem_eq);
        #endif

        return;
    }

    } //end namespace `_omit_objs`

} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[2]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - omit_objs");
    #endif

    //setup new omit objs
    _shared::_opt_map_area::_setup_new_nodes_vct();

    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        _shared::_opt_map_area::_new_nodes_vct,
        &sc::opt_map_area::set_omit_objs,
        &sc::opt_map_area::get_omit_objs,

        //setter assert
        _opt_map_area::_omit_objs::_setter_asserts,

        //element assert
        _shared::_cm_node_elem_eq
    );

    //delete new omit areas
    _shared::_opt_map_area::_teardown_new_nodes_vct();

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[2]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - omit_objs");
    #endif

    //setup new omit areas
    _shared::_opt_map_area::_setup_new_nodes_vct();

    //run test helper
    _class_helper::c::test_vct_setter_getter<
        sc_opt_map_area, sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        _shared::_opt_map_area::_new_nodes_vct,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_omit_objs,
        sc_opt_ma_get_omit_objs,


        //setter assert
        _opt_map_area::_omit_objs::_setter_asserts,

        //element assert
        _shared::_cm_node_elem_eq
    );

    //delete new omit areas
    _shared::_opt_map_area::_teardown_new_nodes_vct();

    return;
}


// -- `exclusive_areas` setter & getter

namespace _opt_map_area {

    namespace _exclusive_areas {

    //setter asserts
    static void _setter_asserts(const sc::opt_map_area & opts_ma) {

        #ifdef SC_DEBUG
        _class_helper::vct::assert_eq<cm_lst_node *>(
            _shared::_opt_map_area::_new_nodes_vct,
            opts_ma.exclusive_areas,
            _shared::_cm_node_elem_eq);
        #endif

        return;
    }

    } //end namespace `_exclusive_areas`

} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[3]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - exclusive_areas");
    #endif

    //setup new omit areas
    _shared::_opt_map_area::_setup_new_nodes_vct();

    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        _shared::_opt_map_area::_new_nodes_vct,
        &sc::opt_map_area::set_exclusive_areas,
        &sc::opt_map_area::get_exclusive_areas,

        //setter assert
        _opt_map_area::_exclusive_areas::_setter_asserts,

        //element assert
        _shared::_cm_node_elem_eq
    );

    //delete new omit areas
    _shared::_opt_map_area::_teardown_new_nodes_vct();

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[3]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - exclusive_areas");
    #endif

    //setup new omit areas
    _shared::_opt_map_area::_setup_new_nodes_vct();

    //run test helper
    _class_helper::c::test_vct_setter_getter<
        sc_opt_map_area, sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        _shared::_opt_map_area::_new_nodes_vct,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_exclusive_areas,
        sc_opt_ma_get_exclusive_areas,


        //setter assert
        _opt_map_area::_exclusive_areas::_setter_asserts,

        //element assert
        _shared::_cm_node_elem_eq
    );

    //delete new omit areas
    _shared::_opt_map_area::_teardown_new_nodes_vct();

    return;
}


// -- `exclusive_objs` setter & getter

namespace _opt_map_area {

    namespace _exclusive_objs {

    //setter asserts
    static void _setter_asserts(const sc::opt_map_area & opts_ma) {

        #ifdef SC_DEBUG
        _class_helper::vct::assert_eq<cm_lst_node *>(
            _shared::_opt_map_area::_new_nodes_vct,
            opts_ma.exclusive_objs,
            _shared::_cm_node_elem_eq);
        #endif

        return;
    }

    } //end namespace `_exclusive_objs`

} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[4]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - exclusive_objs");
    #endif

    //setup new omit areas
    _shared::_opt_map_area::_setup_new_nodes_vct();

    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        _shared::_opt_map_area::_new_nodes_vct,
        &sc::opt_map_area::set_exclusive_objs,
        &sc::opt_map_area::get_exclusive_objs,

        //setter assert
        _opt_map_area::_exclusive_objs::_setter_asserts,

        //element assert
        _shared::_cm_node_elem_eq
    );

    //delete new omit areas
    _shared::_opt_map_area::_teardown_new_nodes_vct();

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[4]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - exclusive_objs");
    #endif

    //setup new omit areas
    _shared::_opt_map_area::_setup_new_nodes_vct();

    //run test helper
    _class_helper::c::test_vct_setter_getter<
        sc_opt_map_area, sc::opt_map_area, cm_lst_node *>(

        //provide test helper requirements
        _shared::_opt_map_area::_new_nodes_vct,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_exclusive_objs,
        sc_opt_ma_get_exclusive_objs,


        //setter assert
        _opt_map_area::_exclusive_objs::_setter_asserts,

        //element assert
        _shared::_cm_node_elem_eq
    );

    //delete new omit areas
    _shared::_opt_map_area::_teardown_new_nodes_vct();

    return;
}


// -- `omit_addr_ranges` setter & getter

namespace _opt_map_area {

    namespace _omit_addr_ranges {

    //setter data
    static cm_vct _new_omit_addr_ranges;
    static sc::addr_range _addr_ranges[4] = {
        sc::addr_range(0x1000, 0x2000),
        sc::addr_range(0x2000, 0x3000),
        sc::addr_range(0x3000, 0x4000),
        sc::addr_range(0x4000, 0x5000)
    };

    //setup data
    static void _setup_data() {
        _class_helper::vct::populate<sc::addr_range>(
            _new_omit_addr_ranges, _addr_ranges, 4);
    }

    //teardown data
    static void _teardown_data() {
        cm_del_vct(&_new_omit_addr_ranges);
    }

    } //end namespace `_omit_addr_ranges`
    
} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[5]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - omit_addr_ranges");
    #endif

    //setup new omit addr ranges
    _opt_map_area::_omit_addr_ranges::_setup_data();
    
    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_map_area, sc::addr_range>(

        //provide test helper requirements
        _opt_map_area::_omit_addr_ranges::_new_omit_addr_ranges,
        &sc::opt_map_area::set_omit_addr_ranges,
        &sc::opt_map_area::get_omit_addr_ranges,

        //setter assert
        [](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<sc::addr_range>(
                _opt_map_area::_omit_addr_ranges::_new_omit_addr_ranges,
                opts_ma.omit_addr_ranges, _shared::_cc_addr_range_elem_eq);
            #endif
        },

        //element assert
        _shared::_cc_addr_range_elem_eq
    );

    //delete new omit addr ranges
    _opt_map_area::_omit_addr_ranges::_teardown_data();

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[5]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - omit_addr_ranges");
    #endif

    //setup new omit addr ranges
    _opt_map_area::_omit_addr_ranges::_setup_data();
    
    //run test helper
    _class_helper::c::test_vct_conv_setter_getter<
        sc_opt_map_area, sc::opt_map_area, sc_addr_range>(

        //provide test helper requirements
        _opt_map_area::_omit_addr_ranges::_new_omit_addr_ranges,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_omit_addr_ranges,
        sc_opt_ma_get_omit_addr_ranges,


        //setter assert
        [](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<sc_addr_range>(
                _opt_map_area::_omit_addr_ranges::_new_omit_addr_ranges,
                opts_ma.omit_addr_ranges, _shared::_c_addr_range_elem_eq);
            #endif

        },

        //element assert
        _shared::_c_addr_range_elem_eq
    );

    //delete new omit addr ranges
    _opt_map_area::_omit_addr_ranges::_teardown_data();

    return;
}


// -- `exclusive_addr_ranges` setter & getter

namespace _opt_map_area {

    namespace _exclusive_addr_ranges {

    //setter data
    static cm_vct _new_exclusive_addr_ranges;
    static sc::addr_range _addr_ranges[4] = {
        sc::addr_range(0x1000, 0x2000),
        sc::addr_range(0x2000, 0x3000),
        sc::addr_range(0x3000, 0x4000),
        sc::addr_range(0x4000, 0x5000)
    };

    //setup data
    static void _setup_data() {
        _class_helper::vct::populate<sc::addr_range>(
            _new_exclusive_addr_ranges, _addr_ranges, 4);
    }

    //teardown data
    static void _teardown_data() {
        cm_del_vct(&_new_exclusive_addr_ranges);
    }

    } //end namespace `_exclusive_addr_ranges`
    
} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[6]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - exclusive_addr_ranges");
    #endif

    //setup new exclusive addr ranges
    _opt_map_area::_exclusive_addr_ranges::_setup_data();

    //run test helper
    _class_helper::cc::test_vct_setter_getter<
        sc::opt_map_area, sc::addr_range>(

        //provide test helper requirements
        _opt_map_area::_exclusive_addr_ranges::_new_exclusive_addr_ranges,
        &sc::opt_map_area::set_exclusive_addr_ranges,
        &sc::opt_map_area::get_exclusive_addr_ranges,

        //setter assert
        [](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<sc::addr_range>(
                _opt_map_area::_exclusive_addr_ranges
                    ::_new_exclusive_addr_ranges,
                opts_ma.exclusive_addr_ranges,
                _shared::_cc_addr_range_elem_eq);
            #endif
        },

        //element assert
        _shared::_cc_addr_range_elem_eq
    );

    //delete new exclusive addr ranges
    _opt_map_area::_exclusive_addr_ranges::_teardown_data();

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[6]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - exclusive_addr_ranges");
    #endif

    //setup new exclusive addr ranges
    _opt_map_area::_exclusive_addr_ranges::_setup_data();
    
    //run test helper
    _class_helper::c::test_vct_conv_setter_getter<
        sc_opt_map_area, sc::opt_map_area, sc_addr_range>(

        //provide test helper requirements
        _opt_map_area::_exclusive_addr_ranges::_new_exclusive_addr_ranges,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_exclusive_addr_ranges,
        sc_opt_ma_get_exclusive_addr_ranges,


        //setter assert
        [](const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            _class_helper::vct::assert_eq<sc_addr_range>(
                _opt_map_area::_exclusive_addr_ranges
                    ::_new_exclusive_addr_ranges,
                opts_ma.exclusive_addr_ranges,
                _shared::_c_addr_range_elem_eq);
            #endif

        },

        //element assert
        _shared::_c_addr_range_elem_eq
    );

    //delete new omit addr ranges
    _opt_map_area::_exclusive_addr_ranges::_teardown_data();

    return;
}


// -- `access` setter & getter

namespace _opt_map_area {

    namespace _access {

    //setter value
    static const cm_byte _new_access = 0b111;


    //default value asserts
    static void _default_getter_asserts(
        const sc::opt_map_area & opts_ma, const cm_byte access) {

        REQUIRE_EQ(access, sc::val_unset::access);
    }

    //new value setter asserts
    static void _new_setter_asserts(const sc::opt_map_area & opts_ma) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(opts_ma.access, _new_access);
        #endif
    }

    //new value getter asserts
    static void _new_getter_asserts(
        const sc::opt_map_area & opts_ma, const cm_byte access) {

        REQUIRE_EQ(access, _new_access);
    }

    } //end namespace `_access`
    
} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[7]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - access");
    #endif
    
    //run test helper
    _class_helper::cc::test_value_setter_getter<
        sc::opt_map_area, cm_byte>(

        //provide test helper requirements
        _opt_map_area::_access::_new_access,
        &sc::opt_map_area::set_access,
        &sc::opt_map_area::get_access,

        //default value asserts
        _opt_map_area::_access::_default_getter_asserts,

        //new value setter asserts
        _opt_map_area::_access::_new_setter_asserts,

        //new value getter asserts
        _opt_map_area::_access::_new_getter_asserts
    );
    
    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[7]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - access");
    #endif

    //run test helper
    _class_helper::c::test_value_setter_getter<
        sc_opt_map_area, sc::opt_map_area, cm_byte>(

        //provide test helper requirements
         _opt_map_area::_access::_new_access,

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_set_access,
        sc_opt_ma_get_access,


        //default value asserts
        _opt_map_area::_access::_default_getter_asserts,

        //new value setter asserts
        _opt_map_area::_access::_new_setter_asserts,

        //new value getter asserts
        _opt_map_area::_access::_new_getter_asserts
    );
    
    return;
}


// -- copy ctor

namespace _opt_map_area {

    namespace _copy_ctor {

    //assert values
    static size_t _old_vct_len[6];
    static cm_byte _old_access;

    
    //copy ctor asserts 
    static void _copy_ctor_asserts(
        const sc::opt_map_area & dst_opts_ma,
        const sc::opt_map_area & src_opts_ma) {

        //assert the constructor succeeded
        REQUIRE_EQ(dst_opts_ma._get_ctor_failed(), false);

        #ifdef SC_DEBUG    
        //save old vector lengths
        _old_vct_len[0] = dst_opts_ma.omit_areas.len;
        _old_vct_len[1] = dst_opts_ma.omit_objs.len;
        _old_vct_len[2] = dst_opts_ma.exclusive_areas.len;
        _old_vct_len[3] = dst_opts_ma.exclusive_objs.len;
        _old_vct_len[4] = dst_opts_ma.omit_addr_ranges.len;
        _old_vct_len[5] = dst_opts_ma.exclusive_addr_ranges.len;
        _old_access = dst_opts_ma.access;

        _class_helper::vct::assert_eq<cm_lst_node *>(
            dst_opts_ma.omit_areas,
            src_opts_ma.omit_areas,
            _shared::_cm_node_elem_eq);
        _class_helper::vct::assert_eq<cm_lst_node *>(
            dst_opts_ma.omit_objs,
            src_opts_ma.omit_objs,
            _shared::_cm_node_elem_eq);
        _class_helper::vct::assert_eq<cm_lst_node *>(
            dst_opts_ma.exclusive_areas,
            src_opts_ma.exclusive_areas,
            _shared::_cm_node_elem_eq);
        _class_helper::vct::assert_eq<cm_lst_node *>(
            dst_opts_ma.exclusive_objs,
            src_opts_ma.exclusive_objs,
            _shared::_cm_node_elem_eq);
        _class_helper::vct::assert_eq<sc::addr_range>(
            dst_opts_ma.omit_addr_ranges,
            src_opts_ma.omit_addr_ranges,
            _shared::_cc_addr_range_elem_eq);
        _class_helper::vct::assert_eq<sc::addr_range>(
            dst_opts_ma.exclusive_addr_ranges,
            src_opts_ma.exclusive_addr_ranges,
            _shared::_cc_addr_range_elem_eq);
        REQUIRE_EQ(dst_opts_ma.access, src_opts_ma.access);
        #endif
    }

    //post source dtor asserts
    static void _src_dtor_asserts(const sc::opt_map_area & opts_ma) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(opts_ma.omit_areas.is_init, true);
        REQUIRE_EQ(opts_ma.omit_areas.len, _old_vct_len[0]);

        REQUIRE_EQ(opts_ma.omit_objs.is_init, true);
        REQUIRE_EQ(opts_ma.omit_objs.len, _old_vct_len[1]);

        REQUIRE_EQ(opts_ma.exclusive_areas.is_init, true);      
        REQUIRE_EQ(opts_ma.exclusive_areas.len, _old_vct_len[2]);

        REQUIRE_EQ(opts_ma.exclusive_objs.is_init, true);            
        REQUIRE_EQ(opts_ma.exclusive_objs.len, _old_vct_len[3]);

        REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, true);
        REQUIRE_EQ(opts_ma.omit_addr_ranges.len, _old_vct_len[4]);

        REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, true);
        REQUIRE_EQ(opts_ma.exclusive_addr_ranges.len, _old_vct_len[5]);

        REQUIRE_EQ(opts_ma.access, _old_access);
        #endif
    }

    //dtor asserts
    static void _dtor_asserts(const sc::opt_map_area & opts_ma) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(opts_ma.omit_areas.is_init, false);
        REQUIRE_EQ(opts_ma.omit_objs.is_init, false);
        REQUIRE_EQ(opts_ma.exclusive_areas.is_init, false);
        REQUIRE_EQ(opts_ma.exclusive_objs.is_init, false);
        REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, false);
        REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, false);
        #endif
    }        

    } //end namespace `_copy_ctor`
    
} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[8]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - copy ctor");
    #endif

    //run test helper
    _class_helper::cc::test_copy_ctor<sc::opt_map_area>(

        //source object setup
        _shared::_populate_opt_map_area,

        //copy ctor asserts 
        _opt_map_area::_copy_ctor::_copy_ctor_asserts,

        //post source dtor asserts
        _opt_map_area::_copy_ctor::_src_dtor_asserts,

        //dtor asserts
        _opt_map_area::_copy_ctor::_dtor_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[8]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - copy ctor");
    #endif

    //run test helper
    _class_helper::c::test_copy_ctor<
        sc_opt_map_area, sc::opt_map_area>(

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_copy_opt_ma,


        //source object setup
        _shared::_populate_opt_map_area,
    
        //copy ctor asserts 
        _opt_map_area::_copy_ctor::_copy_ctor_asserts,

        //post source dtor asserts
        _opt_map_area::_copy_ctor::_src_dtor_asserts
    );

    return;
}


// -- copy assign

namespace _opt_map_area {

    namespace _copy_assign {

    //destination object setup
    static void _dst_obj_setup(sc::opt_map_area & opts_ma) {

        #ifdef SC_DEBUG
        //(fixture) setup vectors
        _class_helper::vct::setup_stub(opts_ma.omit_areas);
        _class_helper::vct::setup_stub(opts_ma.omit_objs);
        _class_helper::vct::setup_stub(opts_ma.exclusive_areas);
        _class_helper::vct::setup_stub(opts_ma.exclusive_objs);
        _class_helper::vct::setup_stub(opts_ma.omit_addr_ranges);
        _class_helper::vct::setup_stub(opts_ma.exclusive_addr_ranges);
        #endif
    }

    //post copy assignment asserts
    static void _copy_assign_asserts(
        const sc::opt_map_area & dst_opts_ma,
        const sc::opt_map_area & src_opts_ma) {

        //assert the constructor succeeded
        REQUIRE_EQ(dst_opts_ma._get_ctor_failed(), false);

        #ifdef SC_DEBUG
        _class_helper::vct::assert_eq<cm_lst_node *>(
            dst_opts_ma.omit_areas,
            src_opts_ma.omit_areas,
            _shared::_cm_node_elem_eq);
        _class_helper::vct::assert_eq<cm_lst_node *>(
            dst_opts_ma.omit_objs,
            src_opts_ma.omit_objs,
            _shared::_cm_node_elem_eq);
        _class_helper::vct::assert_eq<cm_lst_node *>(
            dst_opts_ma.exclusive_areas,
            src_opts_ma.exclusive_areas,
            _shared::_cm_node_elem_eq);
        _class_helper::vct::assert_eq<cm_lst_node *>(
            dst_opts_ma.exclusive_objs,
            src_opts_ma.exclusive_objs,
            _shared::_cm_node_elem_eq);
        _class_helper::vct::assert_eq<sc::addr_range>(
            dst_opts_ma.omit_addr_ranges,
            src_opts_ma.omit_addr_ranges,
            _shared::_cc_addr_range_elem_eq);
        _class_helper::vct::assert_eq<sc::addr_range>(
            dst_opts_ma.exclusive_addr_ranges,
            src_opts_ma.exclusive_addr_ranges,
            _shared::_cc_addr_range_elem_eq);
        REQUIRE_EQ(dst_opts_ma.access, src_opts_ma.access);
        #endif
    }

    } //end namespace `_copy_assign`
    
} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[9]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - copy assign");
    #endif

    //run test helper
    _class_helper::cc::test_copy_assign<sc::opt_map_area>(

        //source object setup
        _shared::_populate_opt_map_area,

        //destination object setup
        _opt_map_area::_copy_assign::_dst_obj_setup,

        //post copy assignment asserts
        _opt_map_area::_copy_assign::_copy_assign_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[9]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - copy assign");
    #endif

    //run test helper
    _class_helper::c::test_copy_assign<
        sc_opt_map_area, sc::opt_map_area>(

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_copy_assign_opt_ma,


        //source object setup
        _shared::_populate_opt_map_area,

        //destination object setup
        _opt_map_area::_copy_assign::_dst_obj_setup,

        //post copy assignment asserts
        _opt_map_area::_copy_assign::_copy_assign_asserts
    );

    return;
}


// -- reset

namespace _opt_map_area {

    namespace _reset {

        //reset asserts
        static void _reset_asserts(const sc::opt_map_area & opts_ma) {

            #ifdef SC_DEBUG
            REQUIRE_EQ(opts_ma.omit_areas.is_init, false);
            REQUIRE_EQ(opts_ma.omit_objs.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_areas.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_objs.is_init, false);
            REQUIRE_EQ(opts_ma.omit_addr_ranges.is_init, false);
            REQUIRE_EQ(opts_ma.exclusive_addr_ranges.is_init, false);
            REQUIRE_EQ(opts_ma.access, sc::val_unset::access);
            #endif
        }

    } //end namespace `_reset`
    
} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[10]) {

    #ifndef SC_DEBUG
    _common::release_warning("opt_map_area - reset");
    #endif

    //run test helper
    _class_helper::cc::test_reset<sc::opt_map_area>(

        //setup
        _shared::_populate_opt_map_area,

        //reset asserts
        _opt_map_area::_reset::_reset_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[10]) {

    #ifndef SC_DEBUG
    _common::release_warning(" (C) opt_map_area - reset");
    #endif

    //run test helper
    _class_helper::c::test_reset<
        sc_opt_map_area, sc::opt_map_area>(

        //fn pointers
        sc_new_opt_ma,
        sc_del_opt_ma,
        sc_opt_ma_reset,


        //setup
        _shared::_populate_opt_map_area,
        
        //reset asserts
        _opt_map_area::_reset::_reset_asserts
    );

    return;
}


/*
 *  --- [MAP_AREA_SET - TESTS] ---
 */

// -- ctor & dtor

namespace _map_area_set {

    namespace _ctor_dtor {

    //post-constructor assertions
    static void _ctor_asserts(const sc::map_area_set & ma_set) {

        //assert the constructor succeeded
        REQUIRE_EQ(ma_set._get_ctor_failed(), false);

        #ifdef SC_DEBUG
        REQUIRE_EQ(ma_set.set.is_init, false);
        #endif
    }

    //fixture
    static void _fixture(sc::map_area_set & ma_set) {

        #ifdef SC_DEBUG
        _class_helper::rbt::setup_stub(ma_set.set);
        #endif
    }

    //post-destructor assertions
    static void _dtor_asserts(const sc::map_area_set & ma_set) {

        #ifdef SC_DEBUG
        REQUIRE_EQ(ma_set.set.is_init, false);
        #endif
    }

    } //end namespace `_ctor_dtor`
    
} //end namespace `_map_area_set`


//C++ test
TEST_CASE(test_cc_map_area_subtests[11]) {

    #ifndef SC_DEBUG
    _common::release_warning("map_area_set - ctor & dtor");
    #endif

    //run test helper
    _class_helper::cc::test_ctor_dtor<sc::map_area_set>(

        //post-constructor assertions
        _map_area_set::_ctor_dtor::_ctor_asserts,

        //fixture
        _map_area_set::_ctor_dtor::_fixture,

        //post-destructor assertions
        _map_area_set::_ctor_dtor::_dtor_asserts
    );

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[11]) {
    
    #ifndef SC_DEBUG
    _common::release_warning(" (C) map_area_set - ctor & dtor");
    #endif

    //run test helper
    _class_helper::c::test_ctor_dtor<
        sc_map_area_set, sc::map_area_set>(

        //fn pointers
        sc_new_ma_set,
        sc_del_ma_set,


        //post-constructor assertions
        _map_area_set::_ctor_dtor::_ctor_asserts,

        //fixture
        _map_area_set::_ctor_dtor::_fixture
    );

    return;
}


// -- produce a set

namespace _map_area_set {

    namespace _update_set {

    /*
     *  NOTE: Setting up constraints for this test is very
     *        tedious. As a result, this test attempts to
     *        to achieve complete code coverage from a
     *        single set of constraints. This is a futile
     *        effort, but it will do for now.
     */

    //populate all constraints
    static void _populate_all_constraints(
        _memcry_helper::args & mcry_args,
        std::variant<
            _opt_helper::cc::args *, _opt_helper::c::args *> args) {
        
            int ret;

            mc_vm_map * m = &mcry_args.map;
            cm_lst_node * node;
            mc_vm_area * area;
            mc_vm_obj * obj;
            uintptr_t start_addr, end_addr;
            sc::addr_range range(0x0, 0x0);

            cm_vct omit_areas;
            cm_vct omit_objs;
            cm_vct exclusive_areas;
            cm_vct exclusive_objs;
            cm_vct omit_addr_ranges;
            cm_vct exclusive_addr_ranges;
            cm_byte access = MC_ACCESS_READ | MC_ACCESS_WRITE;


            //initialise constraint vectors
            ret = cm_new_vct(&omit_areas, sizeof(cm_lst_node *));
            REQUIRE_EQ(ret, 0);
            ret = cm_new_vct(&omit_objs, sizeof(cm_lst_node *));
            REQUIRE_EQ(ret, 0);
            ret = cm_new_vct(&exclusive_areas, sizeof(cm_lst_node *));
            REQUIRE_EQ(ret, 0);
            ret = cm_new_vct(&exclusive_objs, sizeof(cm_lst_node *));
            REQUIRE_EQ(ret, 0);
            ret = cm_new_vct(&omit_addr_ranges,
                             sizeof(sc::addr_range));
            REQUIRE_EQ(ret, 0);
            ret = cm_new_vct(&exclusive_addr_ranges,
                             sizeof(sc::addr_range));
            REQUIRE_EQ(ret, 0);


            //exclusive address range - range of unit_target object
            node = mc_get_obj_by_basename(
                       m, _target_helper::target_name);
            REQUIRE_NE(node, nullptr);
            obj = MC_GET_NODE_OBJ(node);
            REQUIRE_NE(obj, nullptr);

            range = sc::addr_range(obj->start_addr, obj->end_addr);
            ret = cm_vct_apd(&exclusive_addr_ranges, &range);
            REQUIRE_EQ(ret, 0);

            //add exclusive address ranges
            if (std::holds_alternative<_opt_helper::cc::args *>(args)) {
                auto args_cast = std::get<_opt_helper::cc::args *>(args);
                ret = args_cast->opts_ma.set_exclusive_addr_ranges(
                        exclusive_addr_ranges);
                REQUIRE_EQ(ret, 0);

            } else {
                auto args_cast = std::get<_opt_helper::c::args *>(args);
                ret = sc_opt_ma_set_exclusive_addr_ranges(
                          args_cast->opts_ma, &exclusive_addr_ranges);
                REQUIRE_EQ(ret, 0);
            }


            //exclusive objects - `[heap]` & `libc.so.6`
            node = mc_get_obj_by_basename(m, "[heap]");
            REQUIRE_NE(node, nullptr);
            ret = cm_vct_apd(&exclusive_objs, &node);
            REQUIRE_EQ(ret, 0);

            node = mc_get_obj_by_basename(m, "libc.so.6");
            REQUIRE_NE(node, nullptr);
            ret = cm_vct_apd(&exclusive_objs, &node);
            REQUIRE_EQ(ret, 0);

            //add exclusive objs
            if (std::holds_alternative<_opt_helper::cc::args *>(args)) {
                auto args_cast = std::get<_opt_helper::cc::args *>(args);
                ret = args_cast->opts_ma.set_exclusive_objs(
                        exclusive_objs);
                REQUIRE_EQ(ret, 0);

            } else {
                auto args_cast = std::get<_opt_helper::c::args *>(args);
                ret = sc_opt_ma_set_exclusive_objs(
                          args_cast->opts_ma, &exclusive_objs);
                REQUIRE_EQ(ret, 0);

            }


            //exclusive areas - `pattern1.bin` & `pattern2.bin`
            node = mc_get_obj_by_basename(
                       m, _target_helper::pattern_1_basename);
            REQUIRE_NE(node, nullptr);
            obj = MC_GET_NODE_OBJ(node);
            REQUIRE_NE(obj, nullptr);
            node = MC_GET_NODE_PTR(obj->vm_area_node_ps.head);
            REQUIRE_NE(node, nullptr);
            ret = cm_vct_apd(&exclusive_areas, &node);
            REQUIRE_EQ(ret, 0);

            node = mc_get_obj_by_basename(
                       m, _target_helper::pattern_2_basename);
            REQUIRE_NE(node, nullptr);
            obj = MC_GET_NODE_OBJ(node);
            REQUIRE_NE(obj, nullptr);
            node = MC_GET_NODE_PTR(obj->vm_area_node_ps.head);
            REQUIRE_NE(node, nullptr);
            ret = cm_vct_apd(&exclusive_areas, &node);
            REQUIRE_EQ(ret, 0);

            //add exclusive areas
            if (std::holds_alternative<_opt_helper::cc::args *>(args)) {
                auto args_cast = std::get<_opt_helper::cc::args *>(args);
                ret = args_cast->opts_ma.set_exclusive_areas(
                        exclusive_areas);
                REQUIRE_EQ(ret, 0);

            } else {
                auto args_cast = std::get<_opt_helper::c::args *>(args);
                ret = sc_opt_ma_set_exclusive_areas(
                          args_cast->opts_ma, &exclusive_areas);
                REQUIRE_EQ(ret, 0);

            }


            //omit address ranges - subset of libc
            node = mc_get_obj_by_basename(m, "libc.so.6");
            REQUIRE_NE(node, nullptr);
            obj = MC_GET_NODE_OBJ(node);
            REQUIRE_NE(node, nullptr);
            
            range = sc::addr_range(obj->start_addr + 0x800,
                                   obj->end_addr - 0x800);
            ret = cm_vct_apd(&omit_addr_ranges, &range);
            REQUIRE_EQ(ret, 0);

            //add omit addr ranges
            if (std::holds_alternative<_opt_helper::cc::args *>(args)) {
                auto args_cast = std::get<_opt_helper::cc::args *>(args);
                ret = args_cast->opts_ma.set_omit_addr_ranges(
                        omit_addr_ranges);
                REQUIRE_EQ(ret, 0);

            } else {
                auto args_cast = std::get<_opt_helper::c::args *>(args);
                ret = sc_opt_ma_set_omit_addr_ranges(
                          args_cast->opts_ma, &omit_addr_ranges);
                REQUIRE_EQ(ret, 0);

            }


            //omit objects - `pattern2.bin`
            node = mc_get_obj_by_basename(
                       m, _target_helper::pattern_2_basename);
            REQUIRE_NE(node, nullptr);
            ret = cm_vct_apd(&omit_objs, &node);
            REQUIRE_EQ(ret, 0);

            //add omit objs
            if (std::holds_alternative<_opt_helper::cc::args *>(args)) {
                auto args_cast = std::get<_opt_helper::cc::args *>(args);
                ret = args_cast->opts_ma.set_omit_objs(omit_objs);
                REQUIRE_EQ(ret, 0);

            } else {
                auto args_cast = std::get<_opt_helper::c::args *>(args);
                ret = sc_opt_ma_set_omit_objs(
                          args_cast->opts_ma, &omit_objs);
                REQUIRE_EQ(ret, 0);

            }


            //omit areas - 2nd & 3rd area of main executable
            node = mc_get_obj_by_basename(
                       m, _target_helper::target_name);
            REQUIRE_NE(node, nullptr);
            obj = MC_GET_NODE_OBJ(node);
            REQUIRE_NE(node, nullptr);
            
            node = MC_GET_NODE_PTR(
                       obj->vm_area_node_ps.head->next);
            REQUIRE_NE(node, nullptr);
            ret = cm_vct_apd(&omit_areas, &node);
            REQUIRE_EQ(ret, 0);

            node = MC_GET_NODE_PTR(
                       obj->vm_area_node_ps.head->next->next);
            REQUIRE_NE(node, nullptr);
            ret = cm_vct_apd(&omit_areas, &node);
            REQUIRE_EQ(ret, 0);

            //add omit areas
            if (std::holds_alternative<_opt_helper::cc::args *>(args)) {
                auto args_cast = std::get<_opt_helper::cc::args *>(args);
                ret = args_cast->opts_ma.set_omit_areas(omit_areas);
                REQUIRE_EQ(ret, 0);

            } else {
                auto args_cast = std::get<_opt_helper::c::args *>(args);
                ret = sc_opt_ma_set_omit_areas(
                          args_cast->opts_ma, &omit_areas);
                REQUIRE_EQ(ret, 0);

            }

            //access - read & write
            if (std::holds_alternative<_opt_helper::cc::args *>(args)) {
                auto args_cast = std::get<_opt_helper::cc::args *>(args);
                ret = args_cast->opts_ma.set_access(access);
                REQUIRE_EQ(ret, 0);

            } else {
                auto args_cast = std::get<_opt_helper::c::args *>(args);
                ret = sc_opt_ma_set_access(args_cast->opts_ma, access);
                REQUIRE_EQ(ret, 0);
            }


            //cleanup constraint vectors
            cm_del_vct(&omit_areas);
            cm_del_vct(&omit_objs);
            cm_del_vct(&exclusive_areas);
            cm_del_vct(&exclusive_objs);
            cm_del_vct(&omit_addr_ranges);
            cm_del_vct(&exclusive_addr_ranges);
        }

    } //end namespace `_update_set`
    
} //end namespace `_map_area_set`


//C++ test
TEST_CASE(test_cc_map_area_subtests[12]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::cc::args opt_args;

    sc::map_area_set ma_set;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);


    //no constraints
    SUBCASE("no constraints") {

        //setup map area options
        _opt_helper::cc::setup(opt_args, mcry_args, [](auto & args){});

        //update the set
        ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
        REQUIRE_EQ(ret, 0);

        //dump the set
        _common::title(_common::CC, "update_set", "no constraints");
        const char * explanation
         = "\nFor this test, expect the complete map of the target\n"
           "process minus any blacklisted areas (like `[vvar]`)";
        std::cout << explanation << std::endl;
        _common::subtitle("update_set - no constraints", "map dump:");
        _class_helper::ma_set::print_set(ma_set);
    }


    //all constraints
    SUBCASE("all constraints") {

        //setup map area options
        _opt_helper::cc::setup(opt_args, mcry_args,

            //provide all constraints
            [&mcry_args](_opt_helper::cc::args & args) {
                _map_area_set::_update_set::_populate_all_constraints(
                    mcry_args, &args);
            }
        );

        //update the set
        ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
        REQUIRE_EQ(ret, 0);

        //dump the set
        _common::title(_common::CC, "update_set", "all constraints");
        const char * explanation
         = "For this test, expect:\n"
           " > a read & write area from the main executable\n"
           " > a `[heap]` area\n"
           " > a read & write area from `libc.so.6`\n"
           "For the exact constraints used, consult the sources.";
        std::cout << explanation << std::endl;
        _common::subtitle("update_set - all constraints", "map dump:");
        _class_helper::ma_set::print_set(ma_set);
    }

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[12]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::c::args opt_args;

    sc_map_area_set * ma_set;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup a map area set
    ma_set = sc_new_ma_set();
    REQUIRE_NE(ma_set, nullptr);


    //no constraints
    SUBCASE("no constraints") {

        //setup map area options
        _opt_helper::c::setup(opt_args, mcry_args, [](auto & args){});

        //update the set
        ret = sc_ma_set_update_set(
                  ma_set, opt_args.opts_ma, &mcry_args.map);
        REQUIRE_EQ(ret, 0);

        //dump the set
        _common::title(_common::C, "update_set", "no constraints");
        const char * explanation
         = "\nFor this test, expect the complete map of the target\n"
           "process minus any blacklisted areas (like `[vvar]`)";
        std::cout << explanation << std::endl;
        _common::subtitle("update_set - no constraints", "map dump:");
        _class_helper::ma_set::print_set(*(sc::map_area_set *) ma_set);

    }


    //all constraints
    SUBCASE("all constraints") {

        //setup map area options
        _opt_helper::c::setup(opt_args, mcry_args,

            //provide all constraints
            [&mcry_args](_opt_helper::c::args & args) {
                _map_area_set::_update_set::_populate_all_constraints(
                    mcry_args, &args);
            }
        );

        //update the set
        ret = sc_ma_set_update_set(ma_set,
                                   opt_args.opts_ma, &mcry_args.map);
        REQUIRE_EQ(ret, 0);

        //dump the set
        _common::title(_common::C, "update_set", "all constraints");
        const char * explanation
         = "For this test, expect:\n"
           " > a read & write area from the main executable\n"
           " > a `[heap]` area\n"
           " > a read & write area from `libc.so.6`\n"
           "For the exact constraints used, consult the sources.";
        std::cout << explanation << std::endl;
        _common::subtitle("update_set - all constraints", "map dump:");
        _class_helper::ma_set::print_set(*(sc::map_area_set *) ma_set);
    }

    //teardown the map area set
    sc_del_ma_set(ma_set);

    //teardown options
    _opt_helper::c::teardown(opt_args);

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


// -- to an address ordered vector

namespace _map_area_set {

    namespace _to_addr_ord_vct {

    //populate all constraints
    static void _dump_ord_vct(const cm_vct & vct) {

        int ret;

        cm_lst_node * area_node;
        mc_vm_area * area;


        //for all areas in the sorted vector
        for (int i = 0; i < vct.len; ++i) {

            //fetch area
            ret = cm_vct_get(&vct, i, &area_node);
            REQUIRE_EQ(ret, 0);
            area = MC_GET_NODE_AREA(area_node);
            REQUIRE_NE(area, nullptr);

            //dump area
            _memcry_helper::print_area(area);
        }

        return;
    }

    } //end namespace `build_sorted_vct`

} //end namespace '_map_area_set'


//C++ test
TEST_CASE(test_cc_map_area_subtests[13]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::cc::args opt_args;

    sc::map_area_set ma_set;
    cm_vct addr_vct;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);


    //setup map area options
    _opt_helper::cc::setup(opt_args, mcry_args, [](auto & args){});

    //update the set
    ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
    REQUIRE_EQ(ret, 0);

    //build a sorted vector of areas from the set
    ret = ma_set.to_addr_ord_vct(addr_vct);
    REQUIRE_EQ(ret, 0);

    //show test header
    _common::title(_common::CC, "build_sorted_vct", "set vs. address-ordered vector");
    const char * explanation
     = "\nFor this test, expect the regular set to show map areas in\n"
       "any order. The address ordered vector should show them in the\n"
       "correct order with area addresses ascending.";
    std::cout << explanation << std::endl;

    //dump the set
    _common::subtitle("build_sorted_set - set", "map dump:");
    _class_helper::ma_set::print_set(ma_set);
    
    //dump the sorted vector
    _common::subtitle("build_sorted_set - sorted_vct", "map dump:");
    _map_area_set::_to_addr_ord_vct::_dump_ord_vct(addr_vct);


    //teardown the sorted vector
    cm_del_vct(&addr_vct);

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);
}


//C test
TEST_CASE(test_c_map_area_subtests[13]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::c::args opt_args;

    sc_map_area_set * ma_set;
    cm_vct addr_vct;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup a map area set
    ma_set = sc_new_ma_set();
    REQUIRE_NE(ma_set, nullptr);


    //setup map area options
    _opt_helper::c::setup(opt_args, mcry_args,

        //provide all constraints
        [&mcry_args](_opt_helper::c::args & args) {
            _map_area_set::_update_set::_populate_all_constraints(
                mcry_args, &args);
        }
    );

    //update the set
    ret = sc_ma_set_update_set(ma_set,
                               opt_args.opts_ma, &mcry_args.map);
    REQUIRE_EQ(ret, 0);

    //build a sorted vector of areas from the set
    ret = sc_ma_set_to_addr_ord_vct(ma_set, &addr_vct);
    REQUIRE_EQ(ret, 0);


    //show test header
    _common::title(_common::C, "build_sorted_vct", "set vs. address-ordered vector");
    const char * explanation
     = "\nFor this test, expect the regular set to show map areas in\n"
       "any order. The address ordered vector should show them in the\n"
       "correct order with area addresses ascending.";
    std::cout << explanation << std::endl;

    //dump the set
    _common::subtitle("build_sorted_set - set", "map dump:");
    _class_helper::ma_set::print_set(*(sc::map_area_set *) ma_set);
    
    //dump the sorted vector
    _common::subtitle("build_sorted_set - sorted_vct", "map dump:");
    _map_area_set::_to_addr_ord_vct::_dump_ord_vct(addr_vct);


    //teardown the sorted vector
    cm_del_vct(&addr_vct);

    //teardown the map area set
    sc_del_ma_set(ma_set);

    //teardown options
    _opt_helper::c::teardown(opt_args);

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


// -- to a size ordered vector

namespace _map_area_set {

    namespace _to_size_ord_vct {

    //populate all constraints
    static void _dump_ord_vct(const cm_vct & vct) {

        int ret;

        cm_lst_node * area_node;
        mc_vm_area * area;


        //for all areas in the sorted vector
        for (int i = 0; i < vct.len; ++i) {

            //fetch area
            ret = cm_vct_get(&vct, i, &area_node);
            REQUIRE_EQ(ret, 0);
            area = MC_GET_NODE_AREA(area_node);
            REQUIRE_NE(area, nullptr);

            //dump area
            _memcry_helper::print_area(area);
        }

        return;
    }

    } //end namespace `build_sorted_vct`

} //end namespace '_map_area_set'


//C++ test
TEST_CASE(test_cc_map_area_subtests[14]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::cc::args opt_args;

    sc::map_area_set ma_set;
    cm_vct size_vct;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);


    //setup map area options
    _opt_helper::cc::setup(opt_args, mcry_args, [](auto & args){});

    //update the set
    ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
    REQUIRE_EQ(ret, 0);

    //build a sorted vector of areas from the set
    ret = ma_set.to_size_ord_vct(size_vct);
    REQUIRE_EQ(ret, 0);

    //show test header
    _common::title(_common::CC, "build_sorted_vct", "set vs. size-ordered");
    const char * explanation
     = "\nFor this test, expect the regular set to show map areas in\n"
       "any order. The size ordered vector should show them in order of\n"
       "size, largerst to smallest.";
    std::cout << explanation << std::endl;

    //dump the set
    _common::subtitle("build_sorted_set - set", "map dump:");
    _class_helper::ma_set::print_set(ma_set);
    
    //dump the sorted vector
    _common::subtitle("build_sorted_set - sorted_vct", "map dump:");
    _map_area_set::_to_size_ord_vct::_dump_ord_vct(size_vct);


    //teardown the sorted vector
    cm_del_vct(&size_vct);

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);
}


//C test
TEST_CASE(test_c_map_area_subtests[14]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::c::args opt_args;

    sc_map_area_set * ma_set;
    cm_vct size_vct;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup a map area set
    ma_set = sc_new_ma_set();
    REQUIRE_NE(ma_set, nullptr);


    //setup map area options
    _opt_helper::c::setup(opt_args, mcry_args,

        //provide all constraints
        [&mcry_args](_opt_helper::c::args & args) {
            _map_area_set::_update_set::_populate_all_constraints(
                mcry_args, &args);
        }
    );

    //update the set
    ret = sc_ma_set_update_set(ma_set,
                               opt_args.opts_ma, &mcry_args.map);
    REQUIRE_EQ(ret, 0);

    //build a sorted vector of areas from the set
    ret = sc_ma_set_to_size_ord_vct(ma_set, &size_vct);
    REQUIRE_EQ(ret, 0);


    //show test header
    _common::title(_common::C, "build_sorted_vct", "set vs. size-ordered vector");
    const char * explanation
     = "\nFor this test, expect the regular set to show map areas in\n"
       "any order. The size ordered vector should show them in order of\n"
       "size, largerst to smallest.";
    std::cout << explanation << std::endl;

    //dump the set
    _common::subtitle("build_sorted_set - set", "map dump:");
    _class_helper::ma_set::print_set(*(sc::map_area_set *) ma_set);
    
    //dump the sorted vector
    _common::subtitle("build_sorted_set - sorted_vct", "map dump:");
    _map_area_set::_to_size_ord_vct::_dump_ord_vct(size_vct);


    //teardown the sorted vector
    cm_del_vct(&size_vct);

    //teardown the map area set
    sc_del_ma_set(ma_set);

    //teardown options
    _opt_helper::c::teardown(opt_args);

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


// -- copy ctor

namespace _map_area_set {

    namespace _copy_ctor {

    //copy ctor asserts 
    static void _copy_ctor_asserts(
        const sc::map_area_set & dst_map_set,
        const sc::map_area_set & src_map_set) {

        //assert the constructor succeeded
        REQUIRE_EQ(dst_map_set._get_ctor_failed(), false);

        //assert both sets are equal
        _class_helper::rbt::assert_eq<cm_lst_node *, mc_vm_area *>(
            dst_map_set.get_set(),
            src_map_set.get_set(),
            _shared::_cm_node_key_data_eq);
    }

    //post source dtor asserts
    static void _src_dtor_asserts(const sc::map_area_set & ma_set) {
    
        REQUIRE_EQ(ma_set.get_set().is_init, true);
        return;
    }

    //post dtor asserts
    static void _dtor_asserts(const sc::map_area_set & ma_set) {
    
        REQUIRE_EQ(ma_set.get_set().is_init, false);
        return;
    }

    } //end namespace `_copy_ctor`
    
} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[14]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::cc::args opt_args;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup default map area options
    _opt_helper::cc::setup(opt_args, mcry_args, [](auto & args){});


    //run test helper
    _class_helper::cc::test_copy_ctor<sc::map_area_set>(

        //setup source object
        [&opt_args, &mcry_args](sc::map_area_set & ma_set) {

            int ret;

            //update the set
            ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
            REQUIRE_EQ(ret, 0);
        },

        //copy ctor asserts 
        _map_area_set::_copy_ctor::_copy_ctor_asserts,

        //post source dtor asserts
        _map_area_set::_copy_ctor::_src_dtor_asserts,

        //post dtor asserts
        _map_area_set::_copy_ctor::_dtor_asserts
    );


    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[14]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::c::args opt_args;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup default map area options
    _opt_helper::c::setup(opt_args, mcry_args, [](auto & args){});


    //run test helper
    _class_helper::c::test_copy_ctor<sc_map_area_set, sc::map_area_set>(

        //fn pointers
        sc_new_ma_set,
        sc_del_ma_set,
        sc_copy_ma_set,


        //setup source object
        [&opt_args, &mcry_args](sc::map_area_set & ma_set) {

            int ret;

            //update the set
            ret = ma_set.update_set(
                *(sc::opt_map_area *) opt_args.opts_ma, mcry_args.map);
            REQUIRE_EQ(ret, 0);
        },


        //copy ctor asserts 
        _map_area_set::_copy_ctor::_copy_ctor_asserts,

        //post source dtor asserts
        _map_area_set::_copy_ctor::_src_dtor_asserts
    );


    //teardown options
    _opt_helper::c::teardown(opt_args);

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


// -- copy assign

namespace _map_area_set {

    namespace _copy_assign {

    //destination object setup
    static void _dst_obj_setup(sc::map_area_set & ma_set) {

        #ifdef SC_DEBUG
        _class_helper::rbt::setup_stub(ma_set.set);
        #endif
    }

    //copy assign asserts 
    static void _copy_assign_asserts(
        const sc::map_area_set & dst_map_set,
        const sc::map_area_set & src_map_set) {

        //assert the constructor succeeded
        REQUIRE_EQ(dst_map_set._get_ctor_failed(), false);

        //assert both sets are equal
        _class_helper::rbt::assert_eq<cm_lst_node *, mc_vm_area *>(
            dst_map_set.get_set(), src_map_set.get_set(),
            [](cm_lst_node * const & node_0,
               mc_vm_area * const & area_0,
               cm_lst_node * const & node_1,
               mc_vm_area * const & area_1) {

                REQUIRE_EQ(node_0, node_1);
                REQUIRE_EQ(area_0, area_1);
            }
        );
    }

    } //end namespace `_copy_assign`
    
} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[15]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::cc::args opt_args;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup default map area options
    _opt_helper::cc::setup(opt_args, mcry_args, [](auto & args){});


    //run test helper
    _class_helper::cc::test_copy_assign<sc::map_area_set>(

        //source object setup
        [&opt_args, &mcry_args](sc::map_area_set & ma_set) {

            int ret;

            //update the set
            ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
            REQUIRE_EQ(ret, 0);
        },

        //destination object setup
        _map_area_set::_copy_assign::_dst_obj_setup,

        //copy assign asserts 
        _map_area_set::_copy_assign::_copy_assign_asserts
    );


    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[15]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::c::args opt_args;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup default map area options
    _opt_helper::c::setup(opt_args, mcry_args, [](auto & args){});


    //run test helper
    _class_helper::c::test_copy_assign<sc_map_area_set, sc::map_area_set>(

        //fn pointers
        sc_new_ma_set,
        sc_del_ma_set,
        sc_copy_assign_ma_set,


        //setup source object
        [&opt_args, &mcry_args](sc::map_area_set & ma_set) {

            int ret;

            //update the set
            ret = ma_set.update_set(
                *(sc::opt_map_area *) opt_args.opts_ma, mcry_args.map);
            REQUIRE_EQ(ret, 0);

            return;
        },

        //setup destination object
        _map_area_set::_copy_assign::_dst_obj_setup,

        //copy ctor asserts 
        _map_area_set::_copy_assign::_copy_assign_asserts
    );


    //teardown options
    _opt_helper::c::teardown(opt_args);

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


// -- reset

namespace _map_area_set {

    namespace _reset {

    //reset asserts
    static void _reset_asserts(const sc::map_area_set & ma_set) {
        
        REQUIRE_EQ(ma_set.get_set().is_init, false);
    }

    } //end namespace `_reset`
    
} //end namespace `_opt_map_area`


//C++ test
TEST_CASE(test_cc_map_area_subtests[16]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::cc::args opt_args;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup default map area options
    _opt_helper::cc::setup(opt_args, mcry_args, [](auto & args){});


    //run test helper
    _class_helper::cc::test_reset<sc::map_area_set>(

        //setup
        [&opt_args, &mcry_args](sc::map_area_set & ma_set) {

            int ret;

            //update the set
            ret = ma_set.update_set(opt_args.opts_ma, mcry_args.map);
            REQUIRE_EQ(ret, 0);
        },

        //reset asserts
        _map_area_set::_reset::_reset_asserts
    );


    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}


//C test
TEST_CASE(test_c_map_area_subtests[16]) {

    int ret;

    pid_t target_pid;
    _memcry_helper::args mcry_args;
    _opt_helper::c::args opt_args;


    //setup a clean target
    _target_helper::clean_targets();
    target_pid = _target_helper::start_target();

    //setup memcry
    _memcry_helper::setup(mcry_args, target_pid, 1);

    //setup default map area options
    _opt_helper::c::setup(opt_args, mcry_args, [](auto & args){});


    //run test helper
    _class_helper::c::test_reset<sc_map_area_set, sc::map_area_set>(

        //fn pointers
        sc_new_ma_set,
        sc_del_ma_set,
        sc_ma_set_reset,


        //setup
        [&opt_args, &mcry_args](sc::map_area_set & ma_set) {

            int ret;


            //update the set
            ret = ma_set.update_set(
                *(sc::opt_map_area *) opt_args.opts_ma, mcry_args.map);
            REQUIRE_EQ(ret, 0);
        },

        //reset asserts
        _map_area_set::_reset::_reset_asserts
    );


    //teardown options
    _opt_helper::c::teardown(opt_args);

    //teardown memcry
    _memcry_helper::teardown(mcry_args);

    //cleanup the target
    _target_helper::end_target(target_pid);

    return;
}
