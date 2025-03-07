//standard template library
#include <exception>
#include <stdexcept>
#include <typeinfo>

//C standard library
#include <stdio.h>

//local headers
#include "scancry.h"
#include "error.hh"


__thread int ps_errno;


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


/*
 *  --- [EXTERNAL] ---
 */

void ps_perror(const char * prefix) {

    switch(ps_errno) {
        // 1XX - user errors
        case SC_ERR_OPT_NOMAP:
            fprintf(stderr, "%s: %s", prefix, SC_ERR_OPT_NOMAP_MSG);
            break;
        
        case SC_ERR_OPT_NOSESSION:
            fprintf(stderr, "%s: %s", prefix, SC_ERR_OPT_NOSESSION_MSG);
            break;

        // 2XX - internal errors
        case SC_ERR_CMORE:
            fprintf(stderr, "%s: %s", prefix, SC_ERR_CMORE_MSG);
            break;

        case SC_ERR_MEMCRY:
            fprintf(stderr, "%s: %s", prefix, SC_ERR_MEMCRY_MSG);
            break;

        case SC_ERR_EXCP:
            fprintf(stderr, "%s: %s", prefix, SC_ERR_EXCP_MSG);
            break;

        case SC_ERR_RUN_EXCP:
            fprintf(stderr, "%s: %s", prefix, SC_ERR_RUN_EXCP_MSG);
            break;

        // 3XX - environment errors        
        case SC_ERR_MEM:
            fprintf(stderr, "%s: %s", prefix, SC_ERR_MEM_MSG);
            break;
        
        default:
            fprintf(stderr, "Undefined error code.\n");
            break;
    }

    return;
}


const char * ps_strerror(const int ps_errnum) {

    switch (ps_errnum) {
        // 1XX - user errors
        case SC_ERR_OPT_NOMAP:
            return SC_ERR_OPT_NOMAP_MSG;
        
        case SC_ERR_OPT_NOSESSION:
            return SC_ERR_OPT_NOSESSION_MSG;

        // 2XX - internal errors
        case SC_ERR_CMORE:
            return SC_ERR_CMORE_MSG;
        
        case SC_ERR_MEMCRY:
            return SC_ERR_MEMCRY_MSG;

        case SC_ERR_EXCP:
            return SC_ERR_EXCP_MSG;

        case SC_ERR_RUN_EXCP:
            return SC_ERR_RUN_EXCP_MSG;

        // 3XX - environment errors
        case SC_ERR_MEM:
            return SC_ERR_MEM_MSG;

        default:
            return "Undefined error code.\n";
    }
}
