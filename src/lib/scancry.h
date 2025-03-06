#ifndef SCANCRY_H
#define SCANCRY_H

#ifdef __cplusplus
//standard template library
#include <optional>
#include <vector>
#include <string>
#endif

//external libraries
#include <cmore.h>
#include <memcry.h>


/*
 *  NOTE: ScanCry should be accessible through the C ABI for cross
 *  language compatibility. While the internals make use of C++, this
 *  interface either restricts itself to C, or provides C wrappers to C++.
 */



// --- [error handling]
//void return
extern void sc_perror(const char * prefix);
extern const char * sc_strerror(const int sc_errnum);



/*
 *  --- [ERROR HANDLING] ---
 */

extern __thread int sc_errno;


// [error codes] TODO define error code values 3***

// 1XX - user errors
#define SC_ERR_OPT_NOMAP      3100
#define SC_ERR_OPT_NOSESSION  3101
#define SC_ERR_SCAN_SET_EMPTY 3102


// [error code messages]

// 1XX - user errors
#define SC_ERR_OPT_NOMAP_MSG \
    "Provided options did not contain a `mc_vm_map`.\n"
#define SC_ERR_OPT_NOSESSION_MSG \
    "Provided options did not contain a `mc_session`.\n"
#define SC_ERR_SCAN_SET_EMPTY_MSG \
    "Scan set is empty following an update.\n"


#endif
