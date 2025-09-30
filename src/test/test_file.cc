//standard template library
#include <variant>
#include <functional>

//C standard library
#include <cstdio>

//external libraries
#include <cmore.h>
#include <doctest/doctest.h>

//local headers
#include "filters.hh"
#include "common.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/file.hh"



/*
 *  --- [SHARED] ---
 */

namespace _shared {

//test files
namespace _test_files {

    //only the scancry header
    const constexpr char * _scancry_hdr_file_pathname = "scancry_hdr.scb";

} //end namespace `_test_files`

} //end namespace `_shared`



/*
 *  --- [FILE] ---
 */

namespace _file {

    namespace _get_metadata {

    static void _setup_input_file(
        const char * pathname,
        std::variant<
            sc::opt *, sc_opt *> opts) {

        int ret;


        //add an input file path
        if (std::holds_alternative<sc::opt *>(opts)) {
            auto opts_cast = std::get<sc::opt *>(opts);
            ret = opts_cast->set_file_pathname_in(pathname);
            REQUIRE_EQ(ret, 0);
            
        } else {
            auto opts_cast = std::get<sc_opt *>(opts);
            ret = sc_opt_set_file_pathname_in(opts_cast, pathname);
            REQUIRE_EQ(ret, 0);
        }
    }
        
    } //end namespace `_get_metadata`
    
} //end namespace `_file`


//C++ test
TEST_CASE(test_cc_file_subtests[0]) {
    _common::title(_common::CC, "file", "`get_metadata()`");

    int ret;
    sc::opt opts;
    sc::file::metadata mdata;


    //setup options
    _file::_get_metadata::_setup_input_file(
        _shared::_test_files::_scancry_hdr_file_pathname, &opts);

    //get metadata
    ret = sc::file::get_metadata(opts, mdata);
    REQUIRE_EQ(ret, 0);
    REQUIRE_EQ(mdata.ver, sc::file::VER_0_1);
    REQUIRE_EQ(mdata.type, sc::file::PTRSCAN_TYPE);

    return;
}


//C test
TEST_CASE(test_c_file_subtests[0]) {
    _common::title(_common::C, "file", "`sc_file_get_metadata()`");

    int ret;
    sc_opt * opts = nullptr;
    sc_file_metadata mdata;


    //setup options
    opts = sc_new_opt();
    REQUIRE_NE(opts, nullptr);
    _file::_get_metadata::_setup_input_file(
        _shared::_test_files::_scancry_hdr_file_pathname, opts);

    //get metadata
    ret = sc_file_get_metadata(opts, &mdata);
    REQUIRE_EQ(ret, 0);
    REQUIRE_EQ(mdata.ver, SC_FILE_VER_0_1);
    REQUIRE_EQ(mdata.type, SC_FILE_PTRSCAN_TYPE);

    return;
}
