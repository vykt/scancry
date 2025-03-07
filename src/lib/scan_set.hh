#pragma once

//standard template library
#include <optional>
#include <unordered_set>
#include <string>

//external libraries
#include <cmore.h>
#include <memcry.h>

//local includes
#include "scancry.h"
#include "opt.hh"



namespace map_meta {

    //trying to read/write from some backing objects can hang
    inline constexpr const int pathname_blacklist_len = 3;
    inline constexpr const char * pathname_blacklist[pathname_blacklist_len] = {
        "/dev",
        "/memfd",
        "/run"
        //TODO find others
    };
}
