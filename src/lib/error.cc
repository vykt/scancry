//standard template library
#include <exception>
#include <stdexcept>
#include <typeinfo>

//C standard library
#include <cstdio>

//local headers
#include "scancry.h"
#include "error.hh"


__thread int sc_errno;


/*
 *  --- [INTERNAL] ---
 */

//convert an exception into a sc_errno
void exception_sc_errno(const std::exception &excp) {

    if (dynamic_cast<const std::bad_alloc *>(& excp)) {
        sc_errno = SC_ERR_MEM;
    } else if (dynamic_cast<const std::runtime_error *>(& excp)) {
        sc_errno = SC_ERR_RUN_EXCP;
    } else {
        sc_errno = SC_ERR_EXCP;
    }

    return;
}

void print_warning(const std::string msg) {

    std::fprintf(stderr, "[WARNING] %s\n", msg.c_str()); 
    return;
}



/*
 *  --- [EXTERNAL] ---
 */

void sc_perror(const char * prefix) {

    switch(sc_errno) {
        // 1XX - user errors
        case SC_ERR_OPT_NOMAP:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_OPT_NOMAP_MSG);
            break;
        
        case SC_ERR_OPT_NOSESSION:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_OPT_NOSESSION_MSG);
            break;

        case SC_ERR_SCAN_EMPTY:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_SCAN_EMPTY_MSG);
            break;

        case SC_ERR_OPT_EMPTY:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_OPT_EMPTY_MSG);
            break;
            
        case SC_ERR_OPT_MISSING:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_OPT_MISSING_MSG);
            break;

        case SC_ERR_OPT_TYPE:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_OPT_TYPE_MSG);
            break;

        case SC_ERR_TIMESPEC:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_TIMESPEC_MSG);
            break;

        case SC_ERR_IN_USE:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_IN_USE_MSG);
            break;

        // 2XX - internal errors
        case SC_ERR_CMORE:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_CMORE_MSG);
            break;

        case SC_ERR_MEMCRY:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_MEMCRY_MSG);
            break;

        case SC_ERR_PTHREAD:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_PTHREAD_MSG);
            break;

        case SC_ERR_EXCP:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_EXCP_MSG);
            break;

        case SC_ERR_RUN_EXCP:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_RUN_EXCP_MSG);
            break;

        case SC_ERR_DEADLOCK:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_DEADLOCK_MSG);
            break;

        // 3XX - environment errors        
        case SC_ERR_MEM:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_MEM_MSG);
            break;
        
        default:
            std::fprintf(stderr, "Undefined error code.\n");
            break;
    }

    return;
}


const char * sc_strerror(const int sc_errnum) {

    switch (sc_errnum) {
        // 1XX - user errors
        case SC_ERR_OPT_NOMAP:
            return SC_ERR_OPT_NOMAP_MSG;
        
        case SC_ERR_OPT_NOSESSION:
            return SC_ERR_OPT_NOSESSION_MSG;

        case SC_ERR_SCAN_EMPTY:
            return SC_ERR_SCAN_EMPTY_MSG;

        case SC_ERR_OPT_EMPTY:
            return SC_ERR_OPT_EMPTY_MSG;

        case SC_ERR_OPT_MISSING:
            return SC_ERR_OPT_MISSING_MSG;

        case SC_ERR_OPT_TYPE:
            return SC_ERR_OPT_TYPE_MSG;

        case SC_ERR_TIMESPEC:
            return SC_ERR_TIMESPEC_MSG;

        case SC_ERR_IN_USE:
            return SC_ERR_IN_USE_MSG;

        // 2XX - internal errors
        case SC_ERR_CMORE:
            return SC_ERR_CMORE_MSG;
        
        case SC_ERR_MEMCRY:
            return SC_ERR_MEMCRY_MSG;

        case SC_ERR_PTHREAD:
            return SC_ERR_PTHREAD_MSG;

        case SC_ERR_EXCP:
            return SC_ERR_EXCP_MSG;

        case SC_ERR_RUN_EXCP:
            return SC_ERR_RUN_EXCP_MSG;

        case SC_ERR_DEADLOCK:
            return SC_ERR_DEADLOCK_MSG;

        // 3XX - environment errors
        case SC_ERR_MEM:
            return SC_ERR_MEM_MSG;

        default:
            return "Undefined error code.\n";
    }
}
