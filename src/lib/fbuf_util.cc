//standard template library
#include <optional>
#include <vector>
#include <string>
#include <algorithm>

//C standard library
#include <cstring>

//external libraries
#include <cmore.h>

//local headers
#include "scancry.h"
#include "fbuf_util.hh"
#include "scancry_impl.h"



#define _CHECK_BUF(sz, err_ret) if ((buf_left - (ssize_t) sz) < 0) { \
                                    sc_errno = SC_ERR_INVALID_FILE;  \
                                    return err_ret;                  \
                                }
#define _UPDATE(sz) buf_off += sz; buf_left -= sz; cur_byte += sz;


/*
 *  NOTE: Using templates here over `void *` simplifies the interface
 *        at the expense of instantiating multiple almost identical
 *        functions.
 */

//write an arbitrary length string to a file buffer
int fbuf_util::pack_string(
    const std::vector<cm_byte> & buf,
    off_t & buf_off, const std::string & str) {

    cm_byte * cur_byte = (cm_byte *) buf.data() + buf_off;
    ssize_t buf_left = buf.size() - buf_off;

    _CHECK_BUF(str.size() + 1, -1);
    std::memcpy((cm_byte *) (buf.data() + buf_off), str.c_str(), str.size());
    _UPDATE(str.size() + 1);

    return 0;
}


//write an arbitrary type to a file buffer
template <typename T>
int fbuf_util::pack_type(
    const std::vector<cm_byte> & buf,
    off_t & buf_off, const T & type) {

    cm_byte * cur_byte = (cm_byte *) buf.data() + buf_off;
    ssize_t buf_left = buf.size() - buf_off;

    _CHECK_BUF(sizeof(type), -1);
    std::memcpy((cm_byte *) (buf.data() + buf_off), &type, sizeof(type));
    _UPDATE(sizeof(type));

    return 0;
}

//explicit instantiation - `sc::ptrscan_file_hdr`
template int fbuf_util::pack_type<sc::ptr_file_hdr>(
    const std::vector<cm_byte> & buf,
    off_t & buf_off, const sc::ptr_file_hdr & type);

//explicit instantiation - `uint32_t`
template int fbuf_util::pack_type<uint32_t>(
    const std::vector<cm_byte> & buf,
    off_t & buf_off, const uint32_t & type);

//explicit instantiation - `cm_byte`
template int fbuf_util::pack_type<cm_byte>(
    const std::vector<cm_byte> & buf,
    off_t & buf_off, const cm_byte & type);


//write an arbitrary length array of some type to a file buffer
template <typename T>
int fbuf_util::pack_type_array(
    const std::vector<cm_byte> & buf,
    off_t & buf_off, const std::vector<T> & type_arr) {

    cm_byte * cur_byte = (cm_byte *) buf.data() + buf_off;
    ssize_t buf_left = buf.size() - buf_off;

    //check file buffer has enough space
    _CHECK_BUF((sizeof(T) + 1) * type_arr.size(), -1);

    //pack every array element
    for (auto iter = type_arr.begin(); iter != type_arr.end(); ++iter) {

        //pack the element
        std::memcpy((cm_byte *) (buf.data() + buf_off),
                    &(*iter), sizeof(*iter));
        _UPDATE(sizeof(*iter));

        //pack either a delimiter byte or an end byte
        if (iter == --type_arr.end()) {
            *((cm_byte *) (buf.data() + buf_off)) = fbuf_util::_array_end;
        } else {
            *((cm_byte *) (buf.data() + buf_off)) = fbuf_util::_array_delim;
        }
        _UPDATE(sizeof(cm_byte));
    }

    return 0;
}

//explicit instantiation - `uint32_t`
template int fbuf_util::pack_type_array<uint32_t>(
    const std::vector<cm_byte> & buf,
    off_t & buf_off, const std::vector<uint32_t> & type_arr);

//explicit instantiation - `cm_byte`
template int fbuf_util::pack_type_array<cm_byte>(
    const std::vector<cm_byte> & buf,
    off_t & buf_off, const std::vector<cm_byte> & type_arr);


//read an arbitrary length string from a file buffer
std::optional<std::string> fbuf_util::unpack_string(
    const std::vector<cm_byte> & buf, off_t & buf_off) {

    cm_byte * cur_byte;
    ssize_t buf_left = buf.size() - buf_off;
    std::string str;
    bool end = true;


    //setup iteration
    _CHECK_BUF(sizeof(cm_byte), std::nullopt);
    cur_byte = (cm_byte *) buf.data() + buf_off;

    //if buffer run out, return an error
    if (buf_left <= 0) return std::nullopt;

    //handle empty strings
    if (*cur_byte == 0x00) {

        end = false;
        _UPDATE(sizeof(cm_byte));

    //handle non-empty strings
    } else {
    
        do {
            str.push_back(*cur_byte);
            end = false;
            _UPDATE(sizeof(cm_byte));
        
        } while ((*cur_byte != 0x00) && (buf_left > 0));
    }

    return (end == true) ? std::nullopt
                         : std::optional<std::string>(str);
}




//read an arbitrary type from a file buffer
template <typename T>
_SC_DBG_INLINE std::optional<T> fbuf_util::unpack_type(
    const std::vector<cm_byte> & buf, off_t & buf_off) {

    cm_byte * cur_byte;
    ssize_t buf_left = buf.size() - buf_off;
    T type;
    bool end = true;


    //setup buffer pointer
    cur_byte = (cm_byte *) buf.data() + buf_off;

    //assert that the buffer has enough space to fit an instance of type T
    if (buf_left >= sizeof(T)) {

        //read an instance of type T
        type = *((T *) cur_byte);
        end = false;
        _UPDATE(sizeof(T));
    }

    return (end == true) ? std::nullopt : std::optional<T>(type);
}

//explicit instantiation - `sc::_ptrscan_file_hdr`
template std::optional<sc::ptr_file_hdr>
    fbuf_util::unpack_type<sc::ptr_file_hdr>(
    const std::vector<cm_byte> & buf, off_t & buf_off);

//explicit instantiation - `sc::_ptrscan_file_hdr`
template std::optional<uint32_t>
    fbuf_util::unpack_type<uint32_t>(
    const std::vector<cm_byte> & buf, off_t & buf_off);

//explicit instantiation - `sc::_ptrscan_file_hdr`
template std::optional<cm_byte>
    fbuf_util::unpack_type<cm_byte>(
    const std::vector<cm_byte> & buf, off_t & buf_off);


//read an arbitrary length array of some type from a file buffer
template <typename T>
_SC_DBG_INLINE std::optional<std::vector<T>> fbuf_util::unpack_type_array(
    const std::vector<cm_byte> & buf, off_t & buf_off) {

    cm_byte * cur_byte;
    ssize_t buf_left = buf.size() - buf_off;
    std::vector<T> type_arr;
    bool end = true;


    //setup iteration
    _CHECK_BUF(sizeof(cm_byte), std::nullopt);
    cur_byte = (cm_byte *) buf.data() + buf_off;

    //iterate over array entries until meeting `arr_end` control byte
    while (*cur_byte != fbuf_util::_array_end && buf_left >= 5) {

        //read one type T instance
        type_arr.push_back(*((T *) cur_byte));
        _UPDATE(sizeof(T));
        end = false;

        //on element delimeter
        if (*cur_byte == fbuf_util::_array_delim) {
            _UPDATE(sizeof(cm_byte));

        //on end of array
        } else if (*cur_byte == fbuf_util::_array_end) {
            _UPDATE(sizeof(cm_byte));
            break;

        //on malformed array
        } else {
            sc_errno = SC_ERR_INVALID_FILE;
            return std::nullopt;   
        }
    }

    return (end == true) ? std::nullopt
                         : std::optional<std::vector<T>>(type_arr);
}

//explicit instantiation - `uint32_t`
template std::optional<std::vector<uint32_t>>
    fbuf_util::unpack_type_array<uint32_t>(
        const std::vector<cm_byte> & buf, off_t & buf_off);
