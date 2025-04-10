#pragma once

//standard template library
#include <optional>
#include <vector>
#include <string>

//external libraries
#include <cmore.h>


namespace fl_util {

std::optional<std::string> get_file_string(
    const std::vector<cm_byte> & buf, off_t & buf_off);

template <typename T>
std::optional<std::vector<T>> get_file_array(
    const std::vector<cm_byte> & buf, off_t & buf_off);

}
