//C standard library
#include <cstring>
#include <cstdio>

//external libraries
#include <cmore.h>

//local headers
#include "scancry.h"
#include "scancry_impl.h"
#include "file.hh"
#include "ptrscan.hh"
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
    if (rd_bytes < 0) {
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

    //populate metadata
    mdata.ver = hdr.ver;
    mdata.type = hdr.type;

    return 0;
}


//read a pointer scan header
[[nodiscard]] int sc::file::rd_ptr_hdr(
    FILE * fs,
    const sc::file::metadata & mdata,
    sc::file::ptr_metadata & ptr_mdata) noexcept {
        
    int ret;
    
    size_t rd_ents;
    sc::file::ptr_hdr ptr_hdr;


    //assert the file stores pointer scan data
    if (mdata.type != sc::file::PTRSCAN_TYPE) {
        sc_errno = SC_ERR_INVALID_FILE;
        return -1;
    }

    //read the pointer
    rd_ents = std::fread(&ptr_hdr, sizeof(ptr_hdr), 1, fs);
    if (rd_ents != 1) {
        sc_errno = SC_ERR_FILE_IO;
        return -1;
    }

    //assert magic
    ret = std::memcmp(
              ptr_hdr.magic, sc::file::ptr_magic, sc::file::scan_magic_sz);
    if (ret != 0) {
        sc_errno = SC_ERR_INVALID_FILE;
        return -1;
    }

    //populate pointer scan metadata
    ptr_mdata.obj_tbl_num = (int) ptr_hdr.obj_tbl_num;
    ptr_mdata.chain_num   = (int) ptr_hdr.chain_num;

    return 0;
}



/*
 *  --- [FILE | EXTERNAL] ---
 */

//get metadata about a file
[[nodiscard]] int sc::file::get_metadata(
    const sc::opt & opts, sc::file::metadata & mdata) noexcept {

    int ret;
    int ret_val = -1;

    FILE * fs;
    const char * pathname;


    //read lock options
    ret = opts._lock_read();
    if (ret != 0) { sc_errno = SC_ERR_BUSY; return -1; }

    //get pathname of the input file
    pathname = opts.get_file_pathname_in();
    if (pathname == nullptr) {
        sc_errno = SC_ERR_OPT_EMPTY;
        goto _get_metadata_cleanup_0;
    }

    //open a file stream on the input file
    fs = fopen(pathname, "r");
    if (fs == nullptr) {
        sc_errno = SC_ERR_FILE_IO;
        goto _get_metadata_cleanup_0;
    }

    //get the metadata
    ret = sc::file::rd_scancry_hdr(fs, mdata);
    if (ret != 0) goto _get_metadata_cleanup_1;

    //set return to success
    ret_val = 0;


    _get_metadata_cleanup_1:
    std::fclose(fs);

    _get_metadata_cleanup_0:
    opts._unlock();

    return ret_val;
}


//get pointer scan metadata about a file
[[nodiscard]] int sc::file::get_ptr_metadata(
    const sc::opt & opts, sc::file::ptr_metadata & ptr_mdata) noexcept {

    int ret;
    int ret_val = -1;

    FILE * fs;
    const char * pathname;

    size_t rd_ents;
    sc::file::ptr_hdr ptr_hdr;
    sc::file::metadata mdata;


    //read lock options
    ret = opts._lock_read();
    if (ret != 0) { sc_errno = SC_ERR_BUSY; return -1; }

    //get pathname of the input file
    pathname = opts.get_file_pathname_in();
    if (pathname == nullptr) {
        sc_errno = SC_ERR_OPT_EMPTY;
        goto _get_metadata_cleanup_0;
    }

    //open a file stream on the input file
    fs = fopen(pathname, "r");
    if (fs == nullptr) {
        sc_errno = SC_ERR_FILE_IO;
        goto _get_metadata_cleanup_0;
    }

    //get the metadata
    ret = sc::file::rd_scancry_hdr(fs, mdata);
    if (ret != 0) goto _get_metadata_cleanup_1;

    //get the pointer scan metadata
    ret = sc::file::rd_ptr_hdr(fs, mdata, ptr_mdata);
    if (ret != 0) goto _get_metadata_cleanup_1;

    //set return to success
    ret_val = 0;


    _get_metadata_cleanup_1:
    std::fclose(fs);

    _get_metadata_cleanup_0:
    opts._unlock();

    return ret_val;
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
