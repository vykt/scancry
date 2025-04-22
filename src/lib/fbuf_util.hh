#pragma once

//standard template library
#include <optional>
#include <vector>
#include <string>

//external libraries
#include <cmore.h>


namespace fbuf_util {

//file constants
const constexpr cm_byte _array_delim      = 0x00;
const constexpr cm_byte _array_end        = 0xFF;
const constexpr cm_byte _file_end         = 0xBB;


//file buffer packing
int pack_string(
    const std::vector<cm_byte> & buf,
    off_t & buf_off, const std::string & str);

template <typename T>
int pack_type(
    const std::vector<cm_byte> & buf,
    off_t & buf_off, const T & type);

template <typename T>
int pack_type_array(
    const std::vector<cm_byte> & buf,
    off_t & buf_off, const std::vector<T> & type_arr);


//file buffer unpacking
std::optional<std::string> unpack_string(
    const std::vector<cm_byte> & buf, off_t & buf_off);

template <typename T>
std::optional<T> unpack_type(
    const std::vector<cm_byte> & buf, off_t & buf_off);

template <typename T>
std::optional<std::vector<T>> unpack_type_array(
    const std::vector<cm_byte> & buf, off_t & buf_off);

}
