#pragma once

//C++ standard library
#include <new>

//C standard library
#include <cstdlib>
#include <cstring>

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


// -- string operation macros

//delete a string if its initialised
#define _CTOR_STR_DELETE_IF_INIT(str)  \
    common::del_str_if_init(str);      \

//copy a string if its initialised; on fail, cleanup and return early
#define _CTOR_STR_COPY_IF_INIT(dst_str, src_str)            \
    ret = common::cpy_str_if_init(dst_str, src_str);        \
    if (ret != 0) { this->_set_ctor_failed(true); return; } \

//move a string if its initialised
#define _CTOR_STR_MOVE_IF_INIT(dst_str, src_str) \
    common::mov_str_if_init(dst_str, src_str);   \


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


// -- value setter & getter helper macros

//define a value setter
#define _DEFINE_VALUE_SETTER(namespace, type, value) \
[[nodiscard]] int namespace::set_##value(            \
    const type value) noexcept {                     \
                                                     \
    int ret;                                         \
                                                     \
                                                     \
    /* acquire a write lock */                       \
    _LOCK_WRITE(-1)                                  \
                                                     \
    this->value = value;                             \
                                                     \
    /* release the lock */                           \
    _UNLOCK                                          \
                                                     \
    return 0;                                        \
}                                                    \


//define a primitive getter
#define _DEFINE_VALUE_GETTER(namespace, type, fail_val, value) \
[[nodiscard]] type                                             \
    namespace::get_##value() const noexcept {                  \
                                                               \
    int ret;                                                   \
    type value;                                                \
                                                               \
                                                               \
    /* acquire a read lock */                                  \
    _LOCK_READ(fail_val)                                       \
                                                               \
    value = this->value;                                       \
                                                               \
    /* release the lock */                                     \
    _UNLOCK                                                    \
                                                               \
    return value;                                              \
}                                                              \


//define a value setter by reference
#define _DEFINE_VALUE_REF_SETTER(namespace, type, value) \
[[nodiscard]] int namespace::set_##value(                \
    const type & value) noexcept {                       \
                                                         \
    int ret;                                             \
                                                         \
                                                         \
    /* acquire a write lock */                           \
    _LOCK_WRITE(-1)                                      \
                                                         \
    this->value = (type &) value;                        \
                                                         \
    /* release the lock */                               \
    _UNLOCK                                              \
                                                         \
    return 0;                                            \
}                                                        \


//define a value reference getter
#define _DEFINE_VALUE_REF_GETTER(namespace, type, fail_val, value) \
[[nodiscard]] const type &                                         \
    namespace::get_##value() const noexcept {                      \
                                                                   \
    return this->value;                                            \
}                                                                  \



// -- pointer setter & getter helper macros

//define a pointer setter
#define _DEFINE_PTR_SETTER(namespace, type, ptr_field) \
[[nodiscard]] int namespace::set_##ptr_field(          \
    const type * ptr_field) noexcept {                 \
                                                       \
    int ret;                                           \
                                                       \
                                                       \
    /* acquire a write lock */                         \
    _LOCK_WRITE(-1)                                    \
                                                       \
    this->ptr_field = (type *) ptr_field;              \
                                                       \
    /* release the lock */                             \
    _UNLOCK                                            \
                                                       \
    return 0;                                          \
}                                                      \


//define a pointer getter
#define _DEFINE_PTR_GETTER(namespace, type, ptr_field) \
[[nodiscard]] type *                                   \
    namespace::get_##ptr_field() const noexcept {      \
                                                       \
    type * ptr_field;                                  \
                                                       \
                                                       \
    /* acquire a read lock */                          \
    _LOCK_READ(nullptr)                                \
                                                       \
    ptr_field = this->ptr_field;                       \
                                                       \
    /* release the lock */                             \
    _UNLOCK                                            \
                                                       \
    return ptr_field;                                  \
}                                                      \


// -- enum setter & getter helper macros

//define an enum setter
#define _DEFINE_ENUM_SETTER(namespace, type, enm) \
[[nodiscard]] int namespace::set_##enm(           \
    const enum type enm) noexcept {               \
                                                    \
    int ret;                                        \
                                                    \
                                                    \
    /* acquire a write lock */                      \
    _LOCK_WRITE(-1)                                 \
                                                    \
    this->enm = (enum type) enm;                \
                                                    \
    /* release the lock */                          \
    _UNLOCK                                         \
                                                    \
    return 0;                                       \
}                                                   \


//define an enum getter
#define _DEFINE_ENUM_GETTER(namespace, type, enm)                    \
[[nodiscard]] int                                                    \
    namespace::get_##enm(enum type & enm) const noexcept {           \
                                                                     \
    /* acquire a read lock */                                        \
    _LOCK_READ(-1)                                                   \
                                                                     \
    enm = this->enm;                                                 \
                                                                     \
    /* release the lock */                                           \
    _UNLOCK                                                          \
                                                                     \
    return 0;                                                        \
}                                                                    \


// -- string setter & getter helper macros

//define a string setter
#define _DEFINE_STR_SETTER(namespace, str_field)  \
[[nodiscard]] int namespace::set_##str_field(     \
    const char * str_field) noexcept {            \
                                                  \
    int ret;                                      \
    size_t len;                                   \
                                                  \
                                                  \
    /* acquire a write lock */                    \
    _LOCK_WRITE(-1)                               \
                                                  \
    len = strlen(str_field);                      \
    this->str_field                               \
        = static_cast<char *>(std::realloc(       \
            (void *) this->str_field, len + 1));  \
    if (this->str_field == nullptr) {             \
        sc_errno = SC_ERR_MEM;                    \
        _UNLOCK                                   \
        return -1;                                \
    }                                             \
    strncpy(this->str_field, str_field, len + 1); \
                                                  \
    /* release the lock */                        \
    _UNLOCK                                       \
                                                  \
    return 0;                                     \
}                                                 \


//define a string getter
#define _DEFINE_STR_GETTER(namespace, str_field)              \
[[nodiscard]] const char * const &                            \
    namespace::get_##str_field() const noexcept {             \
                                                              \
    return const_cast<const char * const &>(this->str_field); \
}                                                             \


//define an internal mutable string getter
#define _DEFINE_STR_GETTER_MUT(namespace, str_field) \
[[nodiscard]] char *&                                \
    namespace:;get_##str_field() noexcept {          \
                                                     \
    return = this->str_field;                        \
}                                                    \


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
    common::del_vct_if_init(this->vct_field);       \
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
    namespace::get_##vct_field() const noexcept { \
                                                  \
    return this->vct_field;                       \
}                                                 \


//define an internal mutable vector getter
#define _DEFINE_VCT_GETTER_MUT(namespace, vct_field)     \
[[nodiscard]] cm_vct &                                   \
    namespace::_get_##vct_field##_mut() noexcept {       \
                                                         \
    return this->vct_field;                              \
}                                                        \


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
    common::del_rbt_if_init(this->rbt_field);       \
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
    namespace::get_##rbt_field() const noexcept { \
                                                  \
    return this->rbt_field;                       \
}                                                 \


//define an internal mutable red-black tree getter
#define _DEFINE_RBT_GETTER_MUT(namespace, rbt_field)     \
[[nodiscard]] cm_rbt &                                   \
    namespace::_get_##rbt_field##_mut() noexcept {       \
                                                         \
    return this->rbt_field;                              \
}                                                        \


// -- object settter & getter helper macros

//define an object setter by reference
#define _DEFINE_OBJ_REF_SETTER(namespace, type, obj)          \
[[nodiscard]] int namespace::set_##obj(type & obj) noexcept { \
                                                              \
    int ret, ret_val = 0;                                     \
                                                              \
                                                              \
    /* acquire a write lock */                                \
    _LOCK_WRITE(-1)                                           \
                                                              \
    /* acquire a read lock on the source object */            \
    ret = obj._lock_read();                                   \
    if (ret != 0) {                                           \
        _UNLOCK                                               \
        return -1;                                            \
    }                                                         \
                                                              \
    this->obj = obj;                                          \
    if (this->_get_ctor_failed() == true) ret_val = -1;       \
                                                              \
    /* release the lock on the source object */               \
    obj._unlock();                                            \
                                                              \
    /* release the lock */                                    \
    _UNLOCK                                                   \
                                                              \
    return ret_val;                                           \
}                                                             \


//define an object reference getter
#define _DEFINE_OBJ_REF_GETTER(namespace, type, obj) \
[[nodiscard]] const type &                           \
    namespace::get_##obj() const noexcept {          \
                                                     \
    return this->obj;                                \
}                                                    \


//define an object reference getter
#define _DEFINE_OBJ_REF_GETTER_MUT(namespace, type, obj) \
[[nodiscard]] type &                                     \
    namespace::_get_##obj##_mut() noexcept {             \
                                                         \
    return this->obj;                                    \
}                                                        \


namespace common {

//string operations
void del_str_if_init(char *& str) noexcept;
[[nodiscard]] int cpy_str_if_init(
    char *& dst_str, const char * src_str) noexcept;
void mov_str_if_init(
    char *& dst_str, char *& src_str) noexcept;

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

//convert a vector of C objects to/from C++ objects 
[[nodiscard]] int convert_c_data(
    cm_vct /* <N/A> */ & dst_vct,
    const size_t dst_data_sz,
    const cm_vct /* <N/A> */ & src_vct,
    int (* convert)(void * dst, const void * src));


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


//define a C interface setter by value
#define _DEFINE_C_VALUE_SETTER(hdl_type, short_hdl_type,    \
                               type, namespace, hdl, value) \
int sc_##short_hdl_type##_set_##value(                      \
    sc_##hdl_type * hdl, const type value) {                \
                                                            \
    namespace::hdl_type * cc_##hdl                          \
        = (namespace::hdl_type *) hdl;                      \
                                                            \
    return cc_##hdl->set_##value(value);                    \
}                                                           \


//define a C interface getter by value
#define _DEFINE_C_VALUE_GETTER(hdl_type, short_hdl_type,      \
                               type, namespace, hdl, value)   \
type sc_##short_hdl_type##_get_##value(sc_##hdl_type * hdl) { \
                                                              \
    namespace::hdl_type * cc_##hdl                            \
        = (namespace::hdl_type *) hdl;                        \
                                                              \
    return cc_##hdl->get_##value();                           \
}                                                             \


//define a C interface setter by value & convert between C++ & C types
#define _DEFINE_C_VALUE_CONV_SETTER(hdl_type, short_hdl_type, \
                                    type, cc_type, namespace, \
                                    hdl, value, convert_cb)   \
int sc_##short_hdl_type##_set_##value(                        \
    sc_##hdl_type * hdl, const type value) {                  \
                                                              \
    int ret;                                                  \
    cc_type cc_##value;                                       \
    namespace::hdl_type * cc_##hdl                            \
        = (namespace::hdl_type *) hdl;                        \
                                                              \
    ret = convert_cb(&cc_##value, &value);                    \
    if (ret != 0) { sc_errno = SC_ERR_TYPECAST; return -1; }  \
                                                              \
    return cc_##hdl->set_##value(cc_##value);                 \
}                                                             \


//define a C interface getter by value & convert bwetween C++ & C types
#define _DEFINE_C_VALUE_CONV_GETTER(hdl_type, short_hdl_type, \
                                    type, cc_type, namespace, \
                                    hdl, value, convert_cb,   \
                                    bad_val, bad_ret)         \
type sc_##short_hdl_type##_get_##value(sc_##hdl_type * hdl) { \
                                                              \
    int ret;                                                  \
    type c_##value;                                           \
    cc_type cc_##value;                                       \
    namespace::hdl_type * cc_##hdl                            \
        = (namespace::hdl_type *) hdl;                        \
                                                              \
    cc_##value = cc_##hdl->get_##value();                     \
    if (cc_##value != bad_val) return bad_ret;                \
                                                              \
    ret = convert_cb(&c_##value, &cc_##value);                \
    if (ret != 0) {                                           \
        sc_errno = SC_ERR_TYPECAST;                           \
        return bad_ret;                                       \
    }                                                         \
                                                              \
    return c_##value;                                         \
}                                                             \


//define a C interface value setter by pointer
#define _DEFINE_C_PTR_SETTER(hdl_type, short_hdl_type,  \
                             type, namespace, hdl, ptr) \
int sc_##short_hdl_type##_set_##ptr(                    \
    sc_##hdl_type * hdl, const type * ptr) {            \
                                                        \
    namespace::hdl_type * cc_##hdl                      \
        = (namespace::hdl_type *) hdl;                  \
                                                        \
    return cc_##hdl->set_##ptr(*ptr);                   \
}                                                       \


//define a C interface value getter by pointer
#define _DEFINE_C_PTR_GETTER(hdl_type, short_hdl_type,              \
                             type, namespace, hdl, ptr)             \
const type * sc_##short_hdl_type##_get_##ptr(sc_##hdl_type * hdl) { \
                                                                    \
    namespace::hdl_type * cc_##hdl                                  \
        = (namespace::hdl_type *) hdl;                              \
                                                                    \
    return (const type *) &cc_##hdl->get_##ptr();                   \
}                                                                   \


//define a C interface object setter
#define _DEFINE_C_OBJ_SETTER(hdl_type, short_hdl_type,  \
                             type, namespace, hdl, obj) \
int sc_##short_hdl_type##_set_##obj(                    \
    sc_##hdl_type * hdl, const sc_##type * obj) {            \
                                                        \
    namespace::hdl_type * cc_##hdl                      \
        = (namespace::hdl_type *) hdl;                  \
                                                        \
    /* cast setter object */                            \
    namespace::type * cc_##obj                          \
        = (namespace::type *) obj;                      \
                                                        \
    return cc_##hdl->set_##obj(*cc_##obj);               \
}                                                       \


//define a C interface object getter
#define _DEFINE_C_OBJ_GETTER(hdl_type, short_hdl_type,                   \
                             type, namespace, hdl, obj)                  \
const sc_##type * sc_##short_hdl_type##_get_##obj(sc_##hdl_type * hdl) { \
                                                                         \
    namespace::hdl_type * cc_##hdl                                       \
        = (namespace::hdl_type *) hdl;                                   \
                                                                         \
    return (const sc_##type *) &cc_##hdl->get_##obj();                   \
}                                                                        \


//define a C interface value setter by pointer
#define _DEFINE_C_ENUM_SETTER(hdl_type, short_hdl_type,  \
                             type, namespace, hdl, enm)  \
int sc_##short_hdl_type##_set_##enm(                     \
    sc_##hdl_type * hdl, const sc_##type * enm) {        \
                                                         \
    namespace::hdl_type * cc_##hdl                       \
        = (namespace::hdl_type *) hdl;                   \
                                                         \
    return cc_##hdl->set_##enm((namespace::type &) enm); \
}                                                        \


//define a C interface value getter by pointer
#define _DEFINE_C_ENUM_GETTER(hdl_type, short_hdl_type,  \
                              type, namespace, hdl, enm) \
int sc_##short_hdl_type##_get_##enm(                     \
    sc_##hdl_type * hdl, enum sc_##type enm) {           \
                                                         \
    namespace::hdl_type * cc_##hdl                       \
        = (namespace::hdl_type *) hdl;                   \
                                                         \
    return cc_##hdl->get_##enm((namespace::type &) enm); \
}                                                        \


//define a C interface vector setter which converts C structures to C++
#define _DEFINE_C_VCT_CONV_SETTER(hdl_type, short_hdl_type,    \
                                  vct_cc_data_type, namespace, \
                                  hdl, vct, convert_cb)        \
int sc_##short_hdl_type##_set_##vct(                           \
    sc_##hdl_type * hdl, const cm_vct * vct) {                 \
                                                               \
    int ret;                                                   \
    cm_vct cc_##vct;                                           \
    namespace::hdl_type * cc_##hdl                             \
        = (namespace::hdl_type *) hdl;                         \
                                                               \
    /* convert C address ranges to C++ */                      \
    ret = convert_c_data(cc_##vct,                             \
                         sizeof(vct_cc_data_type),             \
                         *vct,                                 \
                         convert_cb);                          \
    if (ret != 0) return -1;                                   \
                                                               \
    ret = cc_##hdl->set_##vct(cc_##vct);                       \
    cm_del_vct(&cc_##vct);                                     \
    return ret;                                                \
}                                                              \


//define a C interface vector getter (by value) which converts C
//structures to C++
#define _DEFINE_C_VCT_CONV_GETTER(hdl_type, short_hdl_type, \
                                  vct_data_type, namespace, \
                                  hdl, vct, convert_cb)     \
int sc_##short_hdl_type##_get_##vct(                        \
    sc_##hdl_type * hdl, cm_vct * vct) {                    \
                                                            \
    namespace::hdl_type * cc_##hdl                          \
        = (namespace::hdl_type *) hdl;                      \
                                                            \
    const cm_vct & cc_##vct                                 \
        = cc_##hdl->get_##vct();                            \
                                                            \
    /* convert C++ address ranges to C */                   \
    return convert_c_data(*vct,                             \
                          sizeof(vct_data_type),            \
                          cc_##vct,                         \
                          convert_cb);                      \
}                                                           \
