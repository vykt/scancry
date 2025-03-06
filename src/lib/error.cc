//C standard library
#include <stdio.h>

//local headers
#include "ptrscan.h"
#include "error.hh"


__thread int ps_errno;



void ps_perror(const char * prefix) {

    switch(ps_errno) {
        // 1XX - user errors
        case PS_ERR_OPT_NOMAP:
            fprintf(stderr, "%s: %s", prefix, PS_ERR_OPT_NOMAP_MSG);
            break;
        
        case PS_ERR_OPT_NOSESSION:
            fprintf(stderr, "%s: %s", prefix, PS_ERR_OPT_NOSESSION_MSG);
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
        case PS_ERR_OPT_NOMAP:
            return PS_ERR_OPT_NOMAP_MSG;
        
        case PS_ERR_OPT_NOSESSION:
            return PS_ERR_OPT_NOSESSION_MSG;

        default:
            return "Undefined error code.\n";
    }
}
