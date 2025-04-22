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





#define _CHECK_BUF(sz, err_ret) if ((buf_left - sz) < 0) {          \
                                    sc_errno = SC_ERR_INVALID_FILE; \
                                    return err_ret;                 \
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

    //keep adding characters until encountering a null terminator
    while (cur_byte != 0x00 && buf_left > 0) {

        str.push_back(*cur_byte);
        end = false;
        _UPDATE(sizeof(cm_byte));
    }

    return (end == true) ? std::optional<std::string>()
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
    if (buf_left < sizeof(T)) {

        //read an instance of type T
        type = *((T *) cur_byte);
        _UPDATE(sizeof(T));
    }

    return 0;
}


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

        //assert the delimiter denotes either a `next element` or `end of array`
        if ((*cur_byte != fbuf_util::_array_delim)
            && (*cur_byte != fbuf_util::_array_end)) {

            sc_errno = SC_ERR_INVALID_FILE;
            return std::nullopt;
        }
        _UPDATE(sizeof(cm_byte));
    }

    //check the buffer didn't abruptly end halfway through an entry
    if ((buf_left % 5) != 0) {
        sc_errno = SC_ERR_INVALID_FILE;
        return std::nullopt;
    }

    return (end == true) ? std::nullopt : type_arr;
}
