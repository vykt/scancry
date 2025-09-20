#pragma once

#ifdef SC_DEBUG
namespace dbg {

    void print_trace(const char * fmt, ...) noexcept;

}
#endif
