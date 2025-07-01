//standard template library
#include <vector>
#include <iostream>
#include <iomanip>

//C standard library
#include <cstdlib>
#include <cstdint>
#include <cstring>

//external libraries
#include <doctest/doctest.h>

//local headers
#include "scan_helper.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/fbuf_util.hh"



//helpers

/*
 *  NOTE: Dump buffer contents 16 bytes per line.
 */
void _scan_helper::hexdump(cm_byte * buf, size_t sz) {

    const constexpr int line_bytes = 16;
    int line_num = ((sz - 1) / 16) + 1;
    off_t buf_off = 0x0;

    //for every line
    std::cout << std::hex;
    for (cm_byte * line_start = buf;
         line_start < (line_start + (line_bytes * line_num));
         line_start += line_bytes) {

        //print buffer offset
        std::cout << "0x" << std::setw(8)
                  << std::setfill('0') << buf_off << ":";

        //for every byte on a line
        for (cm_byte * line_byte = line_start;
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


/*
 *  NOTE: To test if workers read memory correctly, worker thread(s) are
 *        directed to read a mmap'ed `patternN.bin` file, which consists
 *        of an incrementing byte pattern for the first kilobyte, followed
 *        by a decrementing pattern for the next kilobyte.
 *
 *        This only supports offsets of 1 and 4.
 */
 
[[nodiscard]] _SC_DBG_INLINE off_t
    _scan_helper::_fixture_scan::_process_addr(
                                const struct sc::_scan_arg arg,
                                const sc::opt * const opts,
                                const sc::_opt_scan * const opts_scan) {

    /*
     *  FIXME: Race condition possible on `do_crash_one` check.
     */

    //crash one worker if requested
    if (this->do_crash_one) {
        this->do_crash_one = false;
        return -1;
    }

    //crash all workers if requested
    if (this->do_crash_all) {
        this->do_crash_all = true;
        return -1;
    }
    

    //increment call count
    this->call_count += 1;

    //if checks are enabled
    if (do_checks) {

        //check byte pattern is correct
        CHECK_EQ(this->expected_byte, *arg.cur_byte);

        //advance state
        read_off += std::abs(this->mod);

        //apply modifier for each scanned byte
        if (read_off < 0x1000) {
            this->expected_byte += this->mod;
        } else {
            //re-align if mod = 4
            if (std::abs(this->mod) == 4) {
                if (this->mod == 4) this->expected_byte = 0xff;
                if (this->mod == -4) this->expected_byte = 0x0;
            }
            this->mod *= -1;
            read_off = 0x0;
        }
    
    } //end if checks are enabled

    return std::abs(this->mod);
}


/*
 *  NOTE: Using a ptrscan header.
 */

[[nodiscard]] int _scan_helper::_fixture_scan::_generate_body(
    std::vector<cm_byte> & buf, off_t hdr_off) {

    int ret;

    off_t buf_off = 0;
    struct sc::ptr_file_hdr local_hdr;
    const char * testdata = _scan_helper::testdata;


    //lock the scanner
    _LOCK(-1)

    //build local header (ptrscan header)
    local_hdr.pathnames_num    = _scan_helper::pathnames_num;
    local_hdr.pathnames_offset = _scan_helper::pathnames_offset;
    local_hdr.chains_num       = _scan_helper::chains_num;
    local_hdr.chains_offset    = _scan_helper::chains_offset;
    
    //allocate space in the vector for the data
    buf.resize(sizeof(local_hdr)
               + strnlen(testdata, _scan_helper::testdata_sz) + 1);

    //store the scan header
    ret = fbuf_util::pack_type<struct sc::ptr_file_hdr>(
        buf, buf_off, local_hdr);
    CHECK_EQ(ret, 0);

    //store data
    ret = fbuf_util::pack_string(buf, buf_off, testdata);
    CHECK_EQ(ret, 0);

    //store the file end byte
    ret = fbuf_util::pack_type(buf, buf_off, fbuf_util::_file_end);
    CHECK_EQ(ret, 0);

    _UNLOCK(-1)
    return 0;
}


[[nodiscard]] int _scan_helper::_fixture_scan::_process_body(
    const std::vector<cm_byte> & buf, off_t hdr_off,
    const mc_vm_map & map) {

    int ret;

    //call into the shallow handler
    ret = this->_read_body(buf, hdr_off);
    CHECK_EQ(ret, 0);
    
    return 0;
}


[[nodiscard]] int _scan_helper::_fixture_scan::_read_body(
    const std::vector<cm_byte> & buf, off_t hdr_off) {

    int ret;
    struct sc::ptr_file_hdr * ptrscan_hdr;


    //lock scanner
    _LOCK(-1)

    //check the header
    ptrscan_hdr = (struct sc::ptr_file_hdr *) buf.data();
    CHECK_EQ(ptrscan_hdr->chains_num, _scan_helper::chains_num);
    CHECK_EQ(ptrscan_hdr->chains_offset, _scan_helper::chains_offset);
    CHECK_EQ(ptrscan_hdr->pathnames_num, _scan_helper::pathnames_num);
    CHECK_EQ(ptrscan_hdr->pathnames_offset, _scan_helper::pathnames_offset);

    //check the body
    const char * testdata = (const char *) (ptrscan_hdr + 1);
    CHECK_EQ(testdata, _scan_helper::testdata);

    _UNLOCK(-1)
    return 0;
}


[[nodiscard]] int _scan_helper::_fixture_scan::reset() {

    this->do_checks     = false;
    this->do_crash_one  = false;
    this->do_crash_all  = false;
    this->expected_byte = 0;
    this->read_off      = 0;
    this->mod           = 0;
    this->call_count    = 0;

    return 0;
}
