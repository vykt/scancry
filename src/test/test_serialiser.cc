//C standard library
#include <cstdio>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <doctest/doctest.h>

//local headers
#include "filters.hh"
#include "common.hh"
#include "scan_helper.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/serialiser.hh"


      /* ===================== * 
 ===== *  C++ INTERFACE TESTS  * =====
       * ===================== */

/*
 *  --- [TESTS] ---
 */

TEST_CASE(test_cc_serialiser_subtests[0]) {

    int ret;

    _scan_helper::_fixture_scan fix_scan;
    sc::opt opts(sc::AW64);
    sc::serialiser serialiser;

    const char * test_file = "test_file.sc";


    /*
     *  FIXTURE: Set the output & input files.
     */
    
    ret = opts.set_file_path_out(test_file);
    CHECK_EQ(ret, 0);

    ret = opts.set_file_path_in(test_file);
    CHECK_EQ(ret, 0);


    //test 1: save & load a file
    SUBCASE(test_cc_serialiser_subtests[1]) {

        /*
         *  NOTE: Assertions carried out inside the fixture class.
         */

        ret = serialiser.save_scan(fix_scan, opts);
        CHECK_EQ(ret, 0);

        ret = serialiser.load_scan(fix_scan, opts, false);
        CHECK_EQ(ret, 0);

        //cleanup
        ret = std::remove(test_file);
        CHECK_EQ(ret, 0);
        
    } //end test


    //test 2: read the headers of a file
    SUBCASE(test_cc_serialiser_subtests[2]) {

        std::optional<sc::combined_file_hdr> cmb_hdr;

        ret = serialiser.save_scan(fix_scan, opts);
        CHECK_EQ(ret, 0);

        cmb_hdr = serialiser.read_headers(test_file);
        CHECK_EQ(cmb_hdr.has_value(), true);

        //assert the ScanCry header
        CHECK_EQ(cmb_hdr->scancry_hdr.magic, sc::file_magic);
        CHECK_EQ(cmb_hdr->scancry_hdr.scan_type, sc::scan_type_ptr);
        CHECK_EQ(cmb_hdr->scancry_hdr.version, sc::file_ver_0);

        //assert the scan header
        CHECK_EQ(cmb_hdr->ptr_hdr.chains_num,
                 _scan_helper::chains_num);
        CHECK_EQ(cmb_hdr->ptr_hdr.chains_offset,
                 _scan_helper::chains_offset);
        CHECK_EQ(cmb_hdr->ptr_hdr.pathnames_num,
                 _scan_helper::pathnames_num);
        CHECK_EQ(cmb_hdr->ptr_hdr.chains_offset,
                 _scan_helper::pathnames_offset);

    } //end test

    return;
}
