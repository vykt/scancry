#pragma once

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry.h"
#include "file.hh"


namespace sc {

namespace file {

    //ptrscan magic
    const constexpr cm_byte ptr_magic[sc::file::scan_magic_sz]
                                               = {'P', 'T', 'R', ' '};

    //ptrscan header
    struct __attribute__((__packed__)) ptr_hdr {
        cm_byte magic[sc::file::scan_magic_sz];
        uint32_t obj_tbl_num;
        uint32_t chain_num;
    };

    //chain entry
    struct __attribute__((__packed__)) chain_entry {
        uint16_t sync_idx;
        uint64_t off;
        uint32_t obj_tbl_idx;
        uint64_t obj_tbl_off;
    };
    
} //end namespace `file`

} //end namespace `sc`
