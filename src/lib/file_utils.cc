//standard template library
#include <optional>
#include <vector>
#include <string>

//external libraries
#include <cmore.h>

//local headers
#include "file_utils.hh"



/*
 *  FIXME: Since we're doing file buffer reading here, buffer writing
 *         should be put here too to. Almost certainly other scans
 *         will want to use writes that ptrscan currently uses.
 *
 */


//read an arbitrary length string from a file buffer
std::optional<std::string> fl_util::get_file_string(
    const std::vector<cm_byte> & buf, off_t & buf_off) {

    cm_byte * cur_byte;
    std::string str;
    bool end = true;

    /*
     *  FIXME: Handle abrupt end of buffer.
     */

    //setup iteration
    cur_byte = (cm_byte *) buf.data() + buf_off;
    buf_off += 1;

    //keep adding characters until encountering a null terminator
    while (cur_byte != 0x00) {

        str.push_back(*cur_byte);
        end = false;

        cur_byte += 1;
        buf_off += 1;
        
    }

    return (end == true) ? std::nullopt : std::optional<std::string>(str);
}


//read an arbitrary length array of some type
template <typename T>
std::optional<std::vector<T>> get_file_array(
    const std::vector<cm_byte> & buf, off_t & buf_off) {

    T * cur_T;
    cm_byte * cur_byte;
    std::vector<T> arr;

    /*
     *  FIXME: Handle abrupt end of buffer.
     */

    //setup iteration
    cur_byte = (cm_byte *) buf.data() + buf_off;
    
    buf_off += 1;

    //setup iteration
    while (cur_byte != 0x00) {
        
    }
}
