//standard template library
#include <optional>
#include <vector>
#include <string>
#include <fstream>

//C standard library
#include <cstring>

//external libraries
#include <cmore.h>

//local headers
#include "scancry.h"
#include "scancry_impl.h"
#include "error.h"



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

/*
 *  --- [PRIVATE] ---
 */

[[nodiscard]] _SC_DBG_INLINE std::optional<cm_byte>
sc::serialiser::get_scan_type(sc::_scan * scan) {

    if (dynamic_cast<sc::ptrscan *>(scan)) {
        return sc::_scan_type_ptrscan;
    }

    sc_errno = SC_ERR_RTTI;
    return std::nullopt;
}


[[nodiscard]] _SC_DBG_INLINE bool
sc::serialiser::is_header_valid(sc::_scancry_file_hdr & hdr) {

    //check file magic
    auto diff = std::memcmp(
                    hdr.magic, sc::_scancry_magic, sc::_scancry_magic_sz);
    if (diff != 0) {
        sc_errno = SC_ERR_INVALID_FILE;        
        return false;
    }

    //check file version is compatible
    if (hdr.version != sc::_scancry_file_ver_0) {
        sc_errno = SC_ERR_VERSION_FILE;
        return false;
    }
    
    return true;
}



/*
 *  --- [PUBLIC]
 */

[[nodiscard]] int
sc::serialiser::save_scan(
    sc::_scan & scan, const sc::opt & opts) {

    int ret;

    std::ofstream fs;
    std::vector<cm_byte> body_buf;

    struct sc::_scancry_file_hdr sc_hdr;
    std::optional<cm_byte> scan_type;


    //apply lock
    _LOCK

    //check that a file is provided
    if (opts.get_file_path_out().has_value() == false) {
        sc_errno = SC_ERR_OPT_MISSING;
        goto _save_scan_fail;
    }

    //get the scan type
    scan_type = this->get_scan_type(&scan);
    if (scan_type.has_value() == false) goto _save_scan_fail;

    //open an output file stream
    fs = std::ofstream(opts.get_file_path_out().value(),
                        std::ios::out | std::ios::binary);
    if (fs.fail() == true) {
        sc_errno = SC_ERR_FILE;
        goto _save_scan_fail;
    }

    //build the header
    std::memcpy(sc_hdr.magic, sc::_scancry_magic, sc::_scancry_magic_sz);
    sc_hdr.version = sc::_scancry_file_ver_0;
    sc_hdr.scan_type = scan_type.value();

    //write the header
    fs.write(reinterpret_cast<const char *>(&sc_hdr), sizeof(sc_hdr));
    if (fs.fail() == true) {
        sc_errno = SC_ERR_FILE;
        goto _save_scan_file_fail;
    }

    //build the body
    ret = scan._generate_body(body_buf, sizeof(sc_hdr));
    if (ret != 0) goto _save_scan_file_fail;

    //write the body
    fs.write(reinterpret_cast<const char *>(body_buf.data()), body_buf.size());
    if (fs.fail() == true) {
        sc_errno = SC_ERR_FILE;
        goto _save_scan_file_fail;
    }


    fs.close();
    return 0;

    _save_scan_file_fail:
    fs.close();

    _save_scan_fail:
    _UNLOCK
    return -1;
}


[[nodiscard]] int
sc::serialiser::load_scan(
    sc::_scan & scan, const sc::opt & opts, bool shallow) {

    int ret;

    std::ifstream fs;
    std::vector<cm_byte> body_buf;

    struct sc::_scancry_file_hdr sc_hdr;
    std::optional<cm_byte> scan_type;


    //apply lock
    _LOCK

    //open an input file stream
    fs = std::ifstream(opts.get_file_path_in().value(),
                        std::ios::out | std::ios::binary);
    if (fs.fail() == true) {
        sc_errno = SC_ERR_FILE;
        goto _save_scan_fail;
    }

    //read the header
    fs.read(reinterpret_cast<char *>(&sc_hdr), sizeof(sc_hdr));
    if (fs.fail() == true || fs.gcount() < sizeof(sc_hdr)) {
        sc_errno = SC_ERR_FILE;
        goto _save_scan_file_fail;
    }

    //verify the header
    if (this->is_header_valid(sc_hdr) == false) goto _save_scan_file_fail;

    //read & process the body
    if (shallow) {
        ret = scan._read_body(body_buf, sizeof(sc_hdr));
        if (ret != 0) goto _save_scan_file_fail;

    } else {
        if (opts.get_map() == nullptr) {
            sc_errno = SC_ERR_OPT_MISSING;
            goto _save_scan_file_fail;
        }
        ret = scan._process_body(body_buf, sizeof(sc_hdr), *opts.get_map());
        if(ret != 0) goto _save_scan_file_fail;
    }
    

    fs.close();
    return 0;

    _save_scan_file_fail:
    fs.close();

    _save_scan_fail:
    _UNLOCK
    return -1;
}


