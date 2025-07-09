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
#include "error.hh"



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

/*
 *  --- [PRIVATE] ---
 */

[[nodiscard]] _SC_DBG_INLINE std::optional<cm_byte>
sc::serialiser::get_scan_type(sc::_scan * scan) const {

    if (dynamic_cast<sc::ptrscan *>(scan)) {
        return sc::scan_type_ptr;
    }

    /* return ptrscan during unit testing */
    #ifdef DEBUG
    else {
        return sc::scan_type_ptr;
    }
    #endif

    sc_errno = SC_ERR_RTTI;
    return std::nullopt;
}


[[nodiscard]] _SC_DBG_INLINE bool
sc::serialiser::is_header_valid(sc::scancry_file_hdr & hdr) const {

    //check file magic
    auto diff = std::memcmp(
                    hdr.magic, sc::file_magic, sc::file_magic_sz);
    if (diff != 0) {
        sc_errno = SC_ERR_INVALID_FILE;        
        return false;
    }

    //check file version is compatible
    if (hdr.version != sc::file_ver_0) {
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

    struct sc::scancry_file_hdr sc_hdr;
    std::optional<cm_byte> scan_type;


    //apply lock
    _LOCK(-1)

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
    std::memcpy(sc_hdr.magic, sc::file_magic, sc::file_magic_sz);
    sc_hdr.version = sc::file_ver_0;
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
    _UNLOCK(-1);
    return 0;

    _save_scan_file_fail:
    fs.close();

    _save_scan_fail:
    _UNLOCK(-1)
    return -1;
}


[[nodiscard]] int
sc::serialiser::load_scan(
    sc::_scan & scan, const sc::opt & opts, const bool shallow) {

    int ret;

    std::ifstream fs;
    std::vector<cm_byte> body_buf;

    struct sc::scancry_file_hdr sc_hdr;
    std::optional<cm_byte> scan_type;


    //apply lock
    _LOCK(-1)

    //reset the scan
    ret = scan.reset();
    if (ret != 0) {
        /* `goto` would bypass `in_file` initialisation */
        _UNLOCK(-1);
        return -1;
    }

    //assert an input file was provided
    const std::optional<std::string> & in_file = opts.get_file_path_in();
    if (in_file.has_value() == false) {
        sc_errno = SC_ERR_OPT_MISSING;
        goto _load_scan_fail;
    }

    //open an input file stream
    fs = std::ifstream(opts.get_file_path_in().value(),
                        std::ios::out | std::ios::binary);
    if (fs.fail() == true) {
        sc_errno = SC_ERR_FILE;
        goto _load_scan_fail;
    }

    //read the header
    fs.read(reinterpret_cast<char *>(&sc_hdr), sizeof(sc_hdr));
    if (fs.fail() == true || fs.gcount() < sizeof(sc_hdr)) {
        sc_errno = SC_ERR_FILE;
        goto _load_scan_file_fail;
    }

    //verify the header
    if (this->is_header_valid(sc_hdr) == false) goto _load_scan_file_fail;

    //read the body
    body_buf = std::vector<cm_byte>(std::istreambuf_iterator<char>(fs),
                                    std::istreambuf_iterator<char>());
    fs.close();

    //process the body
    if (shallow) {
        ret = scan._read_body(body_buf, sizeof(sc_hdr));
        if (ret != 0) goto _load_scan_fail;

    } else {
        if (opts.get_map() == nullptr) {
            sc_errno = SC_ERR_OPT_MISSING;
            goto _load_scan_fail;
        }
        ret = scan._process_body(body_buf, sizeof(sc_hdr), *opts.get_map());
        if(ret != 0) goto _load_scan_fail;
    }
    

    fs.close();
    _UNLOCK(-1);
    return 0;

    _load_scan_file_fail:
    fs.close();

    _load_scan_fail:
    _UNLOCK(-1)
    return -1;
}


[[nodiscard]] std::optional<sc::combined_file_hdr>
sc::serialiser::read_headers(const char * file_path) {

    ssize_t scan_hdr_sz;
    
    std::ifstream fs;
    struct sc::combined_file_hdr cmb_hdr;


    //open an input file stream
    fs = std::ifstream(file_path,
                       std::ios::out | std::ios::binary);
    if (fs.fail() == true) {
        sc_errno = SC_ERR_FILE;
        goto _read_headers_fail;
    }

    //read the ScanCry header
    fs.read(reinterpret_cast<char *>(
        &cmb_hdr.scancry_hdr), sizeof(cmb_hdr.scancry_hdr));
    if (fs.fail() == true || fs.gcount() < sizeof(cmb_hdr.scancry_hdr)) {
        sc_errno = SC_ERR_FILE;
        goto _read_headers_fail;
    }

    //verify the header
    if (this->is_header_valid(cmb_hdr.scancry_hdr) == false) goto _read_headers_fail;


    //determine the size of the scan header
    switch (cmb_hdr.scancry_hdr.scan_type) {

        case scan_type_ptr:
            scan_hdr_sz = sizeof(cmb_hdr.ptr_hdr);
            break;
        default:
            sc_errno = SC_ERR_FILE;
            goto _read_headers_fail;
        
    } //end switch

    //read the scan header
    fs.read(reinterpret_cast<char *>(&cmb_hdr.ptr_hdr), scan_hdr_sz);
    if (fs.fail() == true || fs.gcount() < sizeof(cmb_hdr.scancry_hdr)) {
        sc_errno = SC_ERR_FILE;
        goto _read_headers_fail;
    }

    
    //cleanup & return
    fs.close();
    return cmb_hdr;

    _read_headers_fail:
    fs.close();
    return std::nullopt;
}



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

sc_serialiser sc_new_serialiser() {

    try {
        return new sc::serialiser();

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return nullptr;
    }
}


int sc_del_serialiser(sc_serialiser serialiser) {

    sc::serialiser * o = static_cast<sc::serialiser *>(serialiser);

    try {
        delete o;
        return 0;

    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_save_scan(sc_serialiser serialiser, sc_scan scan, const sc_opt opts) {

    int ret;

    sc::serialiser * cc_serialiser
        = static_cast<sc::serialiser *>(serialiser);
    sc::_scan * cc_scan = static_cast<sc::_scan *>(scan);
    sc::opt * cc_opts = static_cast<sc::opt *>(opts);


    try {
        ret = cc_serialiser->save_scan(*cc_scan, *cc_opts);
        return (ret == 0) ? 0 : -1;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}


int sc_load_scan(sc_serialiser serialiser,
                 sc_scan scan, const sc_opt opts, const bool shallow) {

    int ret;

    sc::serialiser * cc_serialiser
        = static_cast<sc::serialiser *>(serialiser);
    sc::_scan * cc_scan = static_cast<sc::_scan *>(scan);
    sc::opt * cc_opts = static_cast<sc::opt *>(opts);


    try {
        ret = cc_serialiser->load_scan(*cc_scan, *cc_opts, shallow);
        return (ret == 0) ? 0 : -1;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }               
}


int sc_read_headers(sc_serialiser serialiser,
                    const char * file_path, sc_combined_file_hdr * cmb_hdr) {

    std::optional<sc::combined_file_hdr> cc_cmb_hdr;
    sc::serialiser * cc_serialiser
        = static_cast<sc::serialiser *>(serialiser);


    try {
        cc_cmb_hdr = cc_serialiser->read_headers(file_path);
        if (cc_cmb_hdr.has_value() == false) return -1;
        
        std::memcpy(cmb_hdr, &cc_cmb_hdr.value(), sizeof(*cmb_hdr));
        return 0;
        
    } catch (const std::exception & excp) {
        exception_sc_errno(excp);
        return -1;
    }
}
