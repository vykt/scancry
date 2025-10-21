#pragma once

//C standard library
#include <cstdio>

//external libraries
#include "cmore.h"

//local headers
#include "scancry.h"


/*
 *  NOTE: scancry uses a binary file format with the following sections:
 *
 *        Name format: <process comm>.<pid>.scf
 *          e.g.: netnote.1823.sc
 *
 *        File format:
 *          [1. scancry header]
 *          [2. scan header   ]
 *          [3. data          ]
 *
 *        The scancry header is a generic header applicable to all files.
 *        The scan header is specific to a given scan type, such as a
 *        pointer scan. The data section is defined by the scan type.
 */

namespace sc {

//scancry header constants
namespace file {

    //file header magic
    const constexpr int file_magic_sz = 0x4;
    const constexpr cm_byte file_magic[sc::file::file_magic_sz]
                                               = {'S', 'C', 0x13, 0x37};
    //scan header magic
    const constexpr int scan_magic_sz = 0x4; 

    //scancry header
    struct __attribute__((__packed__)) sc_hdr {
        cm_byte magic[sc::file::file_magic_sz];
        enum sc::file::ver ver;
        enum sc::file::scan_type type;
    };

    //offsets
    const constexpr off_t scan_hdr_off = sizeof(sc_hdr);


    //write a header
    [[nodiscard]] int wr_scancry_hdr(
        FILE * fs, const enum sc::file::scan_type scan_type) noexcept;

    //read & interpret the scancry header
    [[nodiscard]] int rd_scancry_hdr(
        FILE * fs, sc::file::metadata & mdata) noexcept;


    //read a pointer scan header
    [[nodiscard]] int rd_ptr_hdr(
        FILE * fs,
        const sc::file::metadata & mdata,
        sc::file::ptr_metadata & ptr_mdata) noexcept;

} //end namespace `file`

} //end namespace `sc`



extern "C" {

// -- file operations

//return: 0 on success, -1 on error
int sc_file_get_metadata(
    const sc_opt * opts, struct sc_file_metadata * mdata);

}
