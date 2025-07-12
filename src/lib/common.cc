//C standard library
#include <cstdlib>
#include <cstring>

//external libraries
#include <cmore.h>

//local headers
#include "scancry.h"
#include "common.hh"
#include "error.hh"



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

//string operations
void common::del_str_if_init(char *& str) noexcept {

    if (str != nullptr) { std::free((void *) str); str = nullptr; };
    return;
}


[[nodiscard]] int common::cpy_str_if_init(
    char *& dst_str, const char * src_str) noexcept {

    size_t len;


    //return if source string is not initialised
    if (src_str == nullptr) return 0;

    //get length of source string
    len = strlen(src_str);

    //reallocate a destination string
    dst_str = static_cast<char *>(std::realloc((void *) dst_str, len + 1));
    if (dst_str == nullptr) return -1;

    //copy a source string into a destination string
    strncpy((char *) dst_str, src_str, len + 1);

    return 0;
}


void common::mov_str_if_init(char *& dst_str, char *& src_str) noexcept {

    //move a string
    if (src_str != nullptr) {
        dst_str = src_str;
        src_str = nullptr;
    }

    return;
}


//vector operations
void common::del_vct_if_init(cm_vct & vct) noexcept {

    if (vct.is_init == true) cm_del_vct(&vct);
    return;
}


[[nodiscard]] int common::cpy_vct_if_init(
    cm_vct & dst_vct, const cm_vct & src_vct) noexcept {

    int ret;


    //copy a vector
    if (src_vct.is_init == true) {
        ret = cm_vct_cpy(&dst_vct, &src_vct);
        if (ret != 0) {
            sc_errno = SC_ERR_CMORE;
            return -1;
        }
    }

    return 0;
}


void common::mov_vct_if_init(
    cm_vct & dst_vct, cm_vct & src_vct) noexcept {

    //move a vector
    if (src_vct.is_init == true)
        cm_vct_mov(&dst_vct, &src_vct);

    return;
}


//red-black tree operations
void common::del_rbt_if_init(cm_rbt & rbt) noexcept {

    if (rbt.is_init == true) cm_del_rbt(&rbt);
    return;
}


[[nodiscard]] int common::cpy_rbt_if_init(
    cm_rbt & dst_rbt, const cm_rbt & src_rbt) noexcept {

    int ret;


    //copy a red-black tree
    if (src_rbt.is_init == true) {
        ret = cm_rbt_cpy(&dst_rbt, &src_rbt);
        if (ret != 0) {
            sc_errno = SC_ERR_CMORE;
            return -1;
        }
    }

    return 0;
}


void common::mov_rbt_if_init(
    cm_rbt & dst_rbt, cm_rbt & src_rbt) noexcept {

    //move a vector
    if (src_rbt.is_init == true)
        cm_rbt_mov(&dst_rbt, &src_rbt);

    return;
}


      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

//convert between vectors of C and C++ data types
[[nodiscard]] int convert_c_data(
    cm_vct /* <N/A> */ & dst_vct,
    const size_t dst_data_sz,
    const cm_vct /* <N/A> */ & src_vct,
    int (* convert)(void *, const void *)) {

    int ret;

    void * src_data;
    void * dst_data_buf;
    

    //allocate a data buffer
    dst_data_buf = std::malloc(dst_data_sz);
    if (dst_data_buf == nullptr) { sc_errno = SC_ERR_MEM; return -1; }

    //construct the destination vector
    ret = cm_new_vct(&dst_vct, dst_data_sz);
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        free(dst_data_buf);
        return -1;
    }

    //for every element in the source vector
    for (int i = 0; i < src_vct.len; ++i) {

        //get the next source element
        src_data = cm_vct_get_p(&src_vct, i);
        if (src_data == nullptr) {
            sc_errno = SC_ERR_CMORE;
            goto _convert_c_data_fail;
        }

        //convert the source element to a destination element
        ret = convert(dst_data_buf, src_data);
        if (ret != 0) {
            sc_errno = SC_ERR_TYPECAST;
            goto _convert_c_data_fail;
        }

        //add the destination element to the destination vector
        ret = cm_vct_apd(&dst_vct, dst_data_buf);
        if (ret != 0) {
            sc_errno = SC_ERR_CMORE;
            goto _convert_c_data_fail;
        }
    }

    //cleanup
    cm_del_vct(&dst_vct);
    free(dst_data_buf);
    return 0;

    _convert_c_data_fail:
    cm_del_vct(&dst_vct);
    free(dst_data_buf);
    return -1;
}
