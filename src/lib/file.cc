//C standard library
#include <cstring>
#include <cstdio>

//external libraries
#include <cmore.h>

//local headers
#include "scancry.h"
#include "scancry_impl.h"
#include "file.hh"
#include "error.hh"


      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

/*
 *  --- [FILE | INTERNAL] ---
 */

//write a scancry header to a file stream
[[nodiscard]] int sc::file::wr_scancry_hdr(
    FILE * fs, const enum sc::file::scan_type scan_type) noexcept {

    ssize_t wr_bytes;
    sc::file::sc_hdr hdr
        = {{'S', 'C', 0x13, 0x37}, sc::file::cur_ver, scan_type};

    //write the write header
    wr_bytes = std::fwrite(&hdr, sizeof(hdr), 1, fs);
    if (wr_bytes != 1) {
        sc_errno = SC_ERR_FILE_IO;
        return -1;
    }

    return 0;
}


//read a scancry header from a file stream
[[nodiscard]] int sc::file::rd_scancry_hdr(
    FILE * fs, sc::file::metadata & mdata) noexcept {

    int ret;
    ssize_t rd_bytes;
    sc::file::sc_hdr hdr;

    //read the scancry header
    rd_bytes = std::fread(&hdr, sizeof(hdr), 1, fs);
    if (rd_bytes != 1) {
        sc_errno = SC_ERR_FILE_IO;
        return -1;
    }

    //assert magic
    ret = std::memcmp(
              hdr.magic, sc::file::file_magic, sc::file::file_magic_sz);
    if (ret != 0) {
        sc_errno = SC_ERR_INVALID_FILE;
        return -1;
    }

    //assert version
    if (hdr.ver != sc::file::cur_ver) {
        sc_errno = SC_ERR_VERSION_FILE;
        return -1;
    }

    return 0;
}


/*
 *  --- [FILE | EXTERNAL] ---
 */

//get metadata about a file
[[nodiscard]] int sc::file::get_metadata(
    const sc::opt & opts, sc::file::metadata & mdata) noexcept {

    int ret;
    FILE * fs;
    const char * pathname;


    //get pathname of the input file
    pathname = opts.get_file_pathname_in();
    if (pathname == nullptr) {
        sc_errno = SC_ERR_OPT_EMPTY;
        return -1;
    }

    //open a file stream on the input file
    fs = fopen(pathname, "r");
    if (fs == nullptr) {
        sc_errno = SC_ERR_FILE_IO;
        return -1;
    }

    //get the metadata
    ret = sc::file::rd_scancry_hdr(fs, mdata);
    if (ret != 0) return -1;

    return 0;
}



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

/*
 *  --- [FILE | EXTERNAL] ---
 */

int sc_file_get_metadata(
    const sc_opt * opts, struct sc_file_metadata * mdata) {

    sc::opt * cc_opts = (sc::opt *) opts;
    sc::file::metadata * cc_mdata = (sc::file::metadata *) mdata;
    
    return sc::file::get_metadata(*cc_opts, *cc_mdata);
}
