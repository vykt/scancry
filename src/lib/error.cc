//C standard library
#include <cstdio>

//local headers
#include "scancry.h"
#include "error.hh"


__thread int sc_errno;


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

        case SC_ERR_NO_RESULT:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_NO_RESULT_MSG);
            break;

        case SC_ERR_SHALLOW_RESULT:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_SHALLOW_RESULT_MSG);
            break;

        case SC_ERR_INVALID_FILE:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_INVALID_FILE_MSG);
            break;

        case SC_ERR_VERSION_FILE:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_VERSION_FILE_MSG);
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

        case SC_ERR_DEADLOCK:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_DEADLOCK_MSG);
            break;
            
        case SC_ERR_PTR_CHAIN:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_PTR_CHAIN_MSG);
            break;

        case SC_ERR_RTTI:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_RTTI_MSG);
            break;

        case SC_ERR_TYPECAST:
            std::fprintf(stderr, "%s, %s", prefix, SC_ERR_TYPECAST_MSG);
            break;

        // 3XX - environment errors        
        case SC_ERR_MEM:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_MEM_MSG);
            break;

        case SC_ERR_FILE:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_FILE_MSG);
            break;

        case SC_ERR_PAGESIZE:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_PAGESIZE_MSG);
            break;

        case SC_ERR_WORKER_TIMEOUT:
            std::fprintf(stderr, "%s: %s", prefix, SC_ERR_WORKER_TIMEOUT_MSG);
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

        case SC_ERR_NO_RESULT:
            return SC_ERR_NO_RESULT_MSG;

        case SC_ERR_SHALLOW_RESULT:
            return SC_ERR_SHALLOW_RESULT_MSG;

        case SC_ERR_INVALID_FILE:
            return SC_ERR_INVALID_FILE_MSG;

        case SC_ERR_VERSION_FILE:
            return SC_ERR_VERSION_FILE_MSG;

        // 2XX - internal errors
        case SC_ERR_CMORE:
            return SC_ERR_CMORE_MSG;
        
        case SC_ERR_MEMCRY:
            return SC_ERR_MEMCRY_MSG;

        case SC_ERR_PTHREAD:
            return SC_ERR_PTHREAD_MSG;

        case SC_ERR_DEADLOCK:
            return SC_ERR_DEADLOCK_MSG;

        case SC_ERR_PTR_CHAIN:
            return SC_ERR_PTR_CHAIN_MSG;

        case SC_ERR_RTTI:
            return SC_ERR_RTTI_MSG;

        case SC_ERR_TYPECAST:
            return SC_ERR_TYPECAST_MSG;

        // 3XX - environment errors
        case SC_ERR_MEM:
            return SC_ERR_MEM_MSG;

        case SC_ERR_FILE:
            return SC_ERR_FILE_MSG;

        case SC_ERR_PAGESIZE:
            return SC_ERR_PAGESIZE_MSG;

        case SC_ERR_WORKER_TIMEOUT:
            return SC_ERR_WORKER_TIMEOUT_MSG;

        default:
            return "Undefined error code.\n";
    }
}
