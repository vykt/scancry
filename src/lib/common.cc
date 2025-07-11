//external libraries
#include <cmore.h>

//local headers
#include "scancry.h"
#include "common.hh"
#include "error.hh"


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
