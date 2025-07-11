#pragma once

//C++ standard library
#include <new>

//C standard library
#include <cstdlib>

//external libraries
#include <cmore.h>
#include <cstdarg>

//local headers
#include "scancry.h"



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

// -- miscellaneous helpers

//turn a macro argument into a string
#define STRINGIFY(value) #value


// -- vector operation macros

//delete a vector if its initialised
#define _CTOR_VCT_DELETE_IF_INIT(vct) \
    common::del_vct_if_init(vct);     \

//copy a vector if it is initialised; on fail, cleanup and return early
#define _CTOR_VCT_COPY_IF_INIT(dst_vct, src_vct)            \
    ret = common::cpy_vct_if_init(dst_vct, src_vct);        \
    if (ret != 0) { this->_set_ctor_failed(true); return; } \

#define _CTOR_VCT_COPY_IF_INIT_UNLOCK(dst_vct, src_vct, locked_obj) \
    ret = common::cpy_vct_if_init(dst_vct, src_vct);                \
    if (ret != 0) {                                                 \
        locked_obj._unlock();                                       \
        this->_set_ctor_failed(true);                               \
        return;                                                     \
    }                                                               \

//move a vector if its initialised
#define _CTOR_VCT_MOVE_IF_INIT(dst_vct, src_vct) \
    common::mov_vct_if_init(dst_vct, src_vct);   \


// -- red-black tree operation macros

//delete a red-black tree if its initialised
#define _CTOR_RBT_DELETE_IF_INIT(rbt) \
    common::del_rbt_if_init(rbt);     \

//copy a red-black tree if it is initialised; on fail, cleanup and return early
#define _CTOR_RBT_COPY_IF_INIT(dst_rbt, src_rbt)            \
    ret = common::cpy_rbt_if_init(dst_rbt, src_rbt);        \
    if (ret != 0) { this->_set_ctor_failed(true); return; } \

#define _CTOR_RBT_COPY_IF_INIT_UNLOCK(dst_rbt, src_rbt, locked_obj) \
    ret = common::cpy_rbt_if_init(dst_rbt, src_rbt);                \
    if (ret != 0) {                                                 \
        locked_obj._unlock();                                       \
        this->_set_ctor_failed(true);                               \
        return;                                                     \
    }                                                               \

//move a red-black tree if its initialised
#define _CTOR_RBT_MOVE_IF_INIT(dst_rbt, src_rbt) \
    common::mov_rbt_if_init(dst_rbt, src_rbt);   \


// -- primitive setter & getter helper macros

//define a primitive setter
#define _DEFINE_PRIM_SETTER(namespace, type, primitive) \
[[nodiscard]] int namespace::set_##primitive(           \
    const type primitive) noexcept {                    \
                                                        \
    int ret;                                            \
                                                        \
                                                        \
    /* acquire a write lock */                          \
    _LOCK_WRITE(-1)                                     \
                                                        \
    this->primitive = primitive;                        \
                                                        \
    /* release the lock */                              \
    _UNLOCK                                             \
                                                        \
    return 0;                                           \
}                                                       \


//define a primitive getter
#define _DEFINE_PRIM_GETTER(namespace, type, fail_val, primitive) \
[[nodiscard]] type                                                \
    namespace::get_##primitive() noexcept {                       \
                                                                  \
    int ret;                                                      \
    type primitive;                                               \
                                                                  \
    /* acquire a read lock */                                     \
    _LOCK_READ(fail_val)                                          \
                                                                  \
    primitive = this->primitive;                                  \
                                                                  \
    /* release the lock */                                        \
    _UNLOCK                                                       \
                                                                  \
    return primitive;                                             \
}                                                                 \



// -- vector setter & getter helper macros

//define a vector setter
#define _DEFINE_VCT_SETTER(namespace, vct_field)    \
[[nodiscard]] int namespace::set_##vct_field(       \
    const cm_vct & vct_field) noexcept {            \
                                                    \
    int ret;                                        \
                                                    \
                                                    \
    /* acquire a write lock */                      \
    _LOCK_WRITE(-1)                                 \
                                                    \
    _CTOR_VCT_DELETE_IF_INIT(this->vct_field);      \
    ret = cm_vct_cpy(&this->vct_field, &vct_field); \
    if (ret != 0) {                                 \
        sc_errno = SC_ERR_CMORE;                    \
        _UNLOCK                                     \
        return -1;                                  \
    }                                               \
                                                    \
    /* release the lock */                          \
    _UNLOCK                                         \
                                                    \
    return 0;                                       \
}                                                   \


//define a vector getter
#define _DEFINE_VCT_GETTER(namespace, vct_field)  \
[[nodiscard]] const cm_vct &                      \
    namespace::get_##vct_field() noexcept {       \
                                                  \
    return this->vct_field;                       \
}                                                 \


//define an internal mutable vector getter
#define _DEFINE_VCT_GETTER_MUT(namespace, vct_field) \
[[nodiscard]] cm_vct &                               \
    namespace::_get_##vct_field##_mut() noexcept {   \
                                                     \
    return this->vct_field;                          \
}                                                    \


// -- red-black tree setter & getter helper macros

//define a red-black tree setter
#define _DEFINE_RBT_SETTER(namespace, rbt_field)    \
[[nodiscard]] int namespace::set_##rbt_field(       \
    const cm_rbt & rbt_field) noexcept {            \
                                                    \
    int ret;                                        \
                                                    \
                                                    \
    /* acquire a write lock */                      \
    _LOCK_WRITE(-1)                                 \
                                                    \
    _CTOR_RBT_DELETE_IF_INIT(this->rbt_field);      \
    ret = cm_rbt_cpy(&this->rbt_field, &rbt_field); \
    if (ret != 0) {                                 \
        sc_errno = SC_ERR_CMORE;                    \
        _UNLOCK                                     \
        return -1;                                  \
    }                                               \
                                                    \
    /* release the lock */                          \
    _UNLOCK                                         \
                                                    \
    return 0;                                       \
}                                                   \


//define a red-black tree getter
#define _DEFINE_RBT_GETTER(namespace, rbt_field)  \
[[nodiscard]] const cm_rbt &                      \
    namespace::get_##rbt_field() noexcept {       \
                                                  \
    return this->rbt_field;                       \
}                                                 \


//define an internal mutable red-black tree getter
#define _DEFINE_RBT_GETTER_MUT(namespace, rbt_field) \
[[nodiscard]] cm_rbt &                               \
    namespace::_get_##rbt_field##_mut() noexcept {   \
                                                     \
    return this->rbt_field;                          \
}                                                    \



namespace common {

//vector operations
void del_vct_if_init(cm_vct & vct) noexcept;
[[nodiscard]] int cpy_vct_if_init(
    cm_vct & dst_vct, const cm_vct & src_vct) noexcept;
void mov_vct_if_init(
    cm_vct & dst_vct, cm_vct & src_vct) noexcept;

//red-black tree operations
void del_rbt_if_init(cm_rbt & rbt) noexcept;
[[nodiscard]] int cpy_rbt_if_init(
    cm_rbt & dst_rbt, const cm_rbt & src_rbt) noexcept;
void mov_rbt_if_init(
    cm_rbt & dst_rbt, cm_rbt & src_rbt) noexcept;

//setter & getter helpers


} //end namespace `common`



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */


// -- constructors & destructors

//define a constructor
#define _DEFINE_C_CTOR(type, cc_type, short_type, namespace) \
type * sc_new_##short_type() {                               \
                                                             \
    void * alloc;                                            \
    namespace::cc_type * obj;                                \
                                                             \
                                                             \
    /* allocate space for the new object */                  \
    alloc = std::malloc(sizeof(namespace::cc_type));         \
    if (alloc == NULL) {                                     \
        sc_errno = SC_ERR_MEM;                               \
        return NULL;                                         \
    }                                                        \
                                                             \
    /* construct the object in the malloc allocation */      \
    obj = new(alloc) namespace::cc_type();                   \
                                                             \
    /* abort & cleanup if constructor failed */              \
    if (obj->_get_ctor_failed() == true) {                   \
        obj->~cc_type();                                     \
        free(alloc);                                         \
        return NULL;                                         \
    }                                                        \
                                                             \
    return (type *) obj;                                     \
}                                                            \


//define a copy constructor
#define _DEFINE_C_COPY_CTOR(                                 \
    type, cc_type, short_type, namespace, src_obj)           \
type * sc_new_##short_type(type * src_obj) {                 \
                                                             \
    int ret;                                                 \
                                                             \
    void * alloc;                                            \
    namespace::cc_type * obj;                                \
    namespace::cc_type * cc_src_obj;                         \
                                                             \
                                                             \
    /* cast the source object */                             \
    cc_src_obj = (namespace::cc_type *) src_obj;             \
                                                             \
    /* read lock the source object */                        \
    ret = cc_src_obj->_lock_read();                          \
    if (ret != 0) return NULL;                               \
                                                             \
    /* allocate space for the new object */                  \
    alloc = std::malloc(sizeof(namespace::cc_type));         \
    if (alloc == NULL) {                                     \
        cc_src_obj->_unlock();                               \
        sc_errno = SC_ERR_MEM;                               \
        return NULL;                                         \
    }                                                        \
                                                             \
    /* construct the object in the malloc allocation */      \
    obj = new(alloc) namespace::cc_type(*cc_src_obj);        \
                                                             \
    /* unlock the source object */                           \
    cc_src_obj->_unlock();                                   \
                                                             \
    /* abort & cleanup if constructor failed */              \
    if (obj->_get_ctor_failed() == true) {                   \
        obj->~cc_type();                                     \
        free(alloc);                                         \
        return NULL;                                         \
    }                                                        \
                                                             \
    return (type *) obj;                                     \
}                                                            \


//define a destructor
#define _DEFINE_C_DTOR(                        \
    type, cc_type, short_type, namespace, obj) \
void sc_del_##short_type(type * obj) {         \
                                               \
    namespace::cc_type * cc_obj;               \
                                               \
                                               \
    /* cast the object */                      \
    cc_obj = (namespace::cc_type *) obj;       \
                                               \
    /* destroy the object */                   \
    cc_obj->~cc_type();                        \
                                               \
    /* free allocation */                      \
    std::free(obj);                            \
                                               \
    return;                                    \
}                                              \


//define a resetter
#define _DEFINE_C_RESET(                       \
    type, cc_type, short_type, namespace, obj) \
int sc_##short_type##_reset(type * obj) {      \
                                               \
    int ret;                                   \
    namespace::cc_type * cc_obj;               \
                                               \
                                               \
    /* cast the object */                      \
    cc_obj = (namespace::cc_type *) obj;       \
                                               \
    /* reset the object */                     \
    ret = cc_obj->reset();                     \
    if (ret != 0) return -1;                   \
                                               \
    return 0;                                  \
}                                              \


//C setter prelude
#define _C_SETTER_PRELUDE(cc_type, ret_type, namespace, hdl)    \
    ret_type ret;                                               \
    namespace::cc_type * cc_##hdl = (namespace::cc_type *) hdl; \

//C setter postlude
#define _C_SETTER_POSTLUDE(bad_val, bad_ret, good_ret) \
    return (ret == bad_val) ? bad_ret : good_ret;      \

//C getter prelude
#define _C_GETTER_PRELUDE(cc_type, namespace, hdl)    \
    namespace::cc_type * cc_##hdl = (namespace::cc_type *) hdl; \
