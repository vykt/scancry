#pragma once

//standard template library
#include <functional>

//C standard library
#include <cstdlib>
#include <cstring>

//external libraries
#include <cmore.h>
#include <doctest/doctest.h>



namespace _class_helper {

    /*
     *  C++ interface tests
     */

    //setup a stub vector
    void setup_vct_stub(cm_vct & vct);


    namespace cc {

    /*
     *  Setter & getter helpers
     */

    //(default) constructor test
    template <typename obj_T>
    void test_ctor_dtor(
        std::function<void(const obj_T &)> ctor_assert_cb,
        std::function<void(obj_T &)> fixture_cb,
        std::function<void(const obj_T &)> dtor_assert_cb) {

        //use placement new to keep memory after dtor is called
        void * obj_buf = std::malloc(sizeof(obj_T));
        REQUIRE_NE(obj_buf, nullptr);

        //run post-constructor checks
        obj_T * obj = new(obj_buf) obj_T();
        ctor_assert_cb(*obj);

        //run post-destructor checks
        obj->~obj_T();
        dtor_assert_cb(*obj);

        std::free(obj_buf);
        return;
    }


    //copy constructor test
    template <typename obj_T>
    void test_copy_ctor(
        std::function<void(const obj_T &)> src_setup_cb,
        std::function<void(const obj_T &,
                      const obj_T &)> copy_ctor_assert_cb,
        std::function<void(const obj_T &)> copy_dtor_assert_cb,
        std::function<void(const obj_T &)> dtor_assert_cb) {

        //use placement new to keep memory after dtors are called
        void * dst_obj_buf = std::malloc(sizeof(obj_T));
        REQUIRE_NE(dst_obj_buf, nullptr);

        void * src_obj_buf = std::malloc(sizeof(obj_T));
        REQUIRE_NE(src_obj_buf, nullptr);

        //setup the source object
        obj_T * src_obj = new(src_obj_buf) obj_T();
        src_setup_cb(*src_obj);

        //run post-copy-constructor checks
        obj_T dst_obj = new(dst_obj_buf) obj_T();
        copy_ctor_assert_cb(*dst_obj, *src_obj);

        //destroy the source object & run checks on destination object
        src_obj->~obj_T();
        copy_dtor_assert_cb();

        //run post-destructor checks
        dst_obj->~obj_T();
        dtor_assert_cb();

        std::free(dst_obj_buf);
        std::free(src_obj_buf);
        return;
    }


    //a value setter & getter test
    template <typename obj_T, typename value_T>
    void test_value_setter_getter(
        const value_T dflt_val, const value_T new_val,
        int (obj_T::*setter_fn)(const value_T val),
        value_T (obj_T::*getter_fn)() const,
        std::function<void(const obj_T &)> setter_assert_cb,
        std::function<void(const obj_T &)> getter_assert_cb) {

        int ret;
        obj_T obj;


        //run the default value getter checks
        const value_T _dflt_val = obj.getter_fn();
        REQUIRE_EQ(_dflt_val, dflt_val);

        //run the new value setter checks
        ret = obj.setter_fn(new_val);
        REQUIRE_EQ(ret, 0);
        setter_assert_cb(obj);

        //run the new value getter checks
        const value_T _new_val = obj.getter_fn();
        REQUIRE_EQ(_new_val, new_val);

        return;
    }


    //a value reference setter & getter test
    template <typename obj_T, typename value_T>
    void test_value_ref_setter_getter(
        const value_T & dflt_val, const value_T & new_val,
        int (obj_T::*setter_fn)(const value_T &),
        const value_T & (obj_T::*getter_fn)() const,
        std::function<void(const obj_T &)> setter_assert_cb) {

        int ret;
        obj_T obj;


        //run the default value reference getter checks
        const value_T & _dflt_val = obj.getter_fn();
        REQUIRE_EQ(_dflt_val, dflt_val);

        //run the new value reference setter checks
        ret = obj.setter_fn(new_val);
        REQUIRE_EQ(ret, 0);
        setter_assert_cb(obj);

        //run the new value reference getter checks
        const value_T & _new_val = obj.getter_fn();
        REQUIRE_EQ(_new_val, new_val);

        return;
    }


    //a pointer setter & getter test
    template <typename obj_T, typename ptr_T>
    void test_ptr_setter_getter(
        const ptr_T * dflt_ptr, const ptr_T * new_ptr,
        int (obj_T::*setter_fn)(const ptr_T *),
        ptr_T * (obj_T::*getter_fn)() const,
        std::function<void(const obj_T &)> setter_assert_cb) {

        int ret;
        obj_T obj;


        //run the default pointer getter checks
        const ptr_T * _dflt_ptr = obj.getter_fn();
        REQUIRE_EQ(_dflt_ptr, dflt_ptr);

        //run the new pointer setter checks
        ret = obj.setter_fn(new_ptr);
        REQUIRE_EQ(ret, 0);
        setter_assert_cb(obj);

        //run the new pointer getter checks
        const ptr_T & _new_ptr = obj.getter_fn();
        REQUIRE_EQ(_new_ptr, new_ptr);

        return;
    }


    //an enum setter & getter test
    template <typename obj_T, typename enm_T>
    void test_enm_setter_getter(
        const enm_T dflt_val, const enm_T new_enm,
        int (obj_T::*setter_fn)(const enm_T enm),
        int (obj_T::*getter_fn)(enm_T & enm) const,
        std::function<void(const obj_T &)> setter_assert_cb,
        std::function<void(const obj_T &)> getter_assert_cb) {

        int ret;
        obj_T obj;


        //run the default value getter checks
        const enm_T _dflt_enm = obj.getter_fn();
        REQUIRE_EQ(_dflt_enm, dflt_val);

        //run the new value setter checks
        ret = obj.setter_fn(new_enm);
        REQUIRE_EQ(ret, 0);
        setter_assert_cb(obj);

        //run the new value getter checks
        const enm_T _new_enm = obj.getter_fn();
        REQUIRE_EQ(_new_enm, new_enm);

        return;
    }


    //a string setter & getter test
    template <typename obj_T>
    void test_str_setter_getter(
        const char * dflt_str, const char * new_str,
        int (obj_T::*setter_fn)(const char *),
        const char * const & (obj_T::*getter_fn)() const,
        std::function<void(const obj_T &)> setter_assert_cb) {

        int ret;
        obj_T obj;


        //run the default pointer getter checks
        const char * const & _dflt_str = obj.getter_fn();
        REQUIRE_EQ(_dflt_str, dflt_str);

        //run the new pointer setter checks
        ret = obj.setter_fn(new_str);
        REQUIRE_EQ(ret, 0);
        setter_assert_cb(obj);

        //run the new pointer getter checks
        const char * const & _new_str = obj.getter_fn();
        REQUIRE_EQ(_new_str, new_str);

        return;
    }


    //a vector setter & getter test
    template <typename obj_T, typename elem_T>
    void test_vct_setter_getter(
        cm_vct & new_vct,
        int (obj_T::*setter_fn)(const cm_vct &),
        const cm_vct & (obj_T::*getter_fn)() const,
        std::function<void(const obj_T &)> setter_assert_cb,
        std::function<
            void(const elem_T *, const elem_T *)> elem_assert_cb) {

        int ret;
        obj_T obj;

        elem_T * elem_0, elem_1;


        //run the default vector getter checks
        const cm_vct & _dflt_vct = obj.getter_fn();
        REQUIRE_EQ(_dflt_vct.is_init, false);

        //run the new vector setter checks
        ret = obj.setter_fn(new_vct);
        REQUIRE_EQ(ret, 0);
        setter_assert_cb(obj);

        //run the new vector getter checks
        const cm_vct & _new_vct = obj.getter_fn();
        REQUIRE_EQ(_new_vct.len, new_vct.len);
        for (int i = 0; i < _new_vct.len; ++i) {
            elem_0 = cm_vct_get_p(&_new_vct, i);
            REQUIRE_NE(elem_0, nullptr);
            elem_1 = cm_vct_get_p(&new_vct, i);
            REQUIRE_NE(elem_1, nullptr);
            REQUIRE_NE(elem_0, elem_1);
            elem_assert_cb(elem_0, elem_1);
        }

        return;
    }


    //an object setter & getter test
    template <typename obj_T, typename obj_val_T>
    void test_obj_setter_getter(
        const obj_val_T & new_obj,
        int (obj_T::*setter_fn)(const obj_val_T &),
        const obj_val_T & (obj_T::*getter_fn)() const,
        std::function<
            void(const obj_T &, const obj_val_T &)> dflt_getter_assert_cb,
        std::function<void(const obj_T &)> setter_assert_cb,
        std::function<
            void(const obj_T &, const obj_val_T &)> new_getter_assert_cb) {

        int ret;
        obj_T obj;


        //run the default pointer getter checks
        const obj_val_T & _dflt_obj = obj.getter_fn();
        dflt_getter_assert_cb(*obj, _dflt_obj);

        //run the new pointer setter checks
        ret = obj.setter_fn(new_obj);
        REQUIRE_EQ(ret, 0);
        setter_assert_cb(obj);

        //run the new pointer getter checks
        const obj_val_T & _new_obj = obj.getter_fn();
        new_getter_assert_cb(*obj, _new_obj);

        return;
    }

    } //end namespace `cc`



    /*
     *  C interface tests
     */

    namespace c {

    /*
     *  Setter & getter helpers
     */

    //(default) constructor test
    template <typename hdl_T, typename obj_T>
    void test_ctor_dtor(
        hdl_T (* ctor_fn)(), void (* dtor_fn)(hdl_T *),
        std::function<void(const obj_T &)> ctor_assert_cb
        /* no destructor assertions callback, rely on sanitisers */) {

        //run post-constructor checks
        hdl_T * hdl = ctor_fn();
        obj_T * obj = (obj_T *) hdl;
        ctor_assert_cb(obj);

        //run destructor (rely on sanitisers to catch leaks)
        dtor_fn(hdl);
        
        return;
    }


    //copy constructor test
    template <typename hdl_T, typename obj_T>
    void test_copy_ctor(
        hdl_T (* ctor_fn)(), void (* dtor_fn)(hdl_T *),
        hdl_T (* copy_ctor_fn)(const hdl_T *),
        std::function<void(const obj_T &)> src_setup_cb,
        std::function<
            void(const obj_T &, const obj_T &)> copy_ctor_assert_cb,
        std::function<void(const obj_T &)> copy_dtor_assert_cb
        /* no destination handle destructor assertions callback */) {

        //setup the source handle
        hdl_T * src_hdl = ctor_fn();
        obj_T * src_obj = (obj_T *) src_hdl;
        src_setub_cb(*src_obj);

        //run post-copy-constructor checks
        hdl_T * dst_hdl = copy_ctor_fn(src_hdl);
        obj_T * dst_obj = (obj_T *) dst_hdl;
        copy_ctor_assert_cb(*dst_obj);

        //destroy the source handle & run checks on destination handle
        dtor_fn(src_hdl);
        copy_dtor_assert_cb(*dst_obj);

        //destroy destination handle
        dtor_fn(dst_hdl);

        return;
    }


    //a setter & getter test for values
    template <typename hdl_T, typename obj_T, typename value_T>
    void test_value_setter_getter(
        const value_T & new_val,
        hdl_T (* ctor_fn)(), void (* dtor_fn)(hdl_T *),
        int (* setter_fn)(hdl_T * hdl, const value_T),
        value_T (* getter_fn)(const hdl_T *),
        std::function<void(const obj_T &, const value_T &)> dflt_getter_cb,
        std::function<void(const obj_T &)> new_setter_cb,
        std::function<void(const obj_T &, const value_T &)> new_getter_cb) {

        int ret;


        //setup the handle
        hdl_T * hdl = ctor_fn();
        obj_T * obj = (obj_T *) hdl;

        //run the default value getter checks
        const value_T _dflt_val = getter_fn(hdl);
        dflt_getter_cb(*obj, _dflt_val);

        //run the new value setter checks
        ret = setter_fn(hdl, new_val);
        REQUIRE_EQ(ret, 0);

        //run the new value getter checks
        const value_T _new_val = getter_vn(hdl);
        new_getter_cb(*obj, _new_val);

        //destroy handle
        dtor_fn(hdl);
    }

    
    //a setter & getter test for pointers
    template <typename hdl_T, typename obj_T, typename ptr_T>
    void test_ptr_setter_getter(
        const ptr_T * new_ptr,
        hdl_T (* ctor_fn)(), void (* dtor_fn)(hdl_T * hdl),
        int (* setter_fn)(hdl_T * hdl, const ptr_T *),
        const ptr_T * (* getter_fn)(const hdl_T *),
        std::function<void(obj_T)> setter_cb) {

        int ret;


        //setup the handle        
        hdl_T * hdl = ctor_fn();
        obj_T * obj = (obj_T *) hdl;

        //run the default pointer getter checks
        const ptr_T * _dflt_ptr = getter_fn(hdl);
        REQUIRE_EQ(_dflt_ptr, NULL);

        //run the new pointer setter checks
        ret = setter_fn(hdl, new_ptr);
        REQUIRE_EQ(ret, 0);
        setter_cb(*obj);

        //run the new pointer getter checks
        const ptr_T * _new_ptr = getter_vn(hdl);
        REQUIRE_EQ(_new_ptr, new_ptr);

        //destroy handle
        dtor_fn(hdl);
    }


    //a setter & getter test for enums
    template <typename hdl_T, typename obj_T, typename enm_T>
    void test_enm_setter_getter(
        const enm_T dflt_enm, const enm_T new_enm,
        hdl_T (* ctor_fn)(), void (* dtor_fn)(hdl_T * hdl),
        int (* setter_fn)(hdl_T * hdl, const enm_T),
        int (* getter_fn)(const hdl_T * hdl, enm_T *),
        std::function<void(const obj_T &)> setter_cb) {

        int ret;
        enm_T _dflt_enm, _new_enm;


        //setup the object        
        hdl_T * hdl = ctor_fn();
        obj_T * obj = (obj_T *) hdl;

        //run the default enum getter checks
        ret = getter_fn(hdl, &_dflt_enm);
        REQUIRE_EQ(ret, 0);
        REQUIRE_EQ(_dflt_enm, dflt_enm);

        //run the new enum setter checks
        ret = setter_fn(hdl, new_enm);
        REQUIRE_EQ(ret, 0);
        setter_cb(obj);

        //run the new enum getter checks
        ret = getter_fn(hdl, &_new_enm);
        REQUIRE_EQ(ret, 0);
        REQUIRE_EQ(_new_enm, new_enm);

        //destroy handle
        dtor_fn(hdl);
    }


    //a setter & getter test for strings
    template <typename hdl_T, typename obj_T>
    void test_str_setter_getter(
        const char * dflt_str, const char * new_str,
        hdl_T (* ctor_fn)(), void (* dtor_fn)(hdl_T * hdl),
        int (* setter_fn)(hdl_T * hdl, const char *),
        const char * const * (* getter_fn)(const hdl_T * hdl),
        std::function<void(const obj_T &)> setter_cb) {

        int ret;


        //setup the object       
        hdl_T * hdl = ctor_fn();
        obj_T * obj = (obj_T *) hdl;

        //run the default string getter checks
        const char * const * _dflt_str = getter_fn(hdl);
        REQUIRE_NE(*_dflt_str, dflt_str);
        REQUIRE_EQ(strcmp(*_dflt_str, dflt_str), 0);

        //run the new string setter checks
        ret = setter_fn(hdl, new_str);
        REQUIRE_EQ(ret, 0);
        setter_cb(obj);

        //run the new string getter checks
        const char * const * _new_str = getter_vn(hdl);
        REQUIRE_NE(*_dflt_str, dflt_str);
        REQUIRE_EQ(strcmp(*_new_str, new_str), 0);

        //destroy handle
        dtor_fn(hdl);
    }


    //a setter & getter test for vectors
    template <typename hdl_T, typename obj_T, typename elem_T>
    void test_vct_setter_getter(
        cm_vct * new_vct,
        hdl_T (* ctor_fn)(), void (* dtor_fn)(hdl_T * hdl),
        int (* setter_fn)(hdl_T * hdl, const cm_vct *),
        int (* getter_fn)(const hdl_T * hdl, cm_vct *),
        std::function<void(const obj_T &)> new_setter_cb,
        const std::function<
            void(const elem_T *, const elem_T *)> elem_assert_cb) {

        int ret;
        elem_T * elem_0, * elem_1;


        //setup the object        
        hdl_T * hdl = ctor_fn();
        obj_T * obj = (obj_T *) hdl;

        //run the default vector getter checks
        const cm_vct * _dflt_vct = getter_fn(hdl);
        REQUIRE_EQ(_dflt_vct->is_init, false);

        //run the new vector setter checks
        ret = setter_fn(hdl, new_vct);
        REQUIRE_EQ(ret, 0);
        new_setter_cb(*obj);

        //run the new vector getter checks
        const cm_vct * _new_vct = getter_vn(hdl);
        REQUIRE_EQ(_new_vct->len, new_vct->len);
        for (int i = 0; i < _new_vct->len; ++i) {
            elem_0 = cm_vct_get_p(_new_vct, i);
            REQUIRE_NE(elem_0, nullptr);
            elem_1 = cm_vct_get_p(new_vct, i);
            REQUIRE_NE(elem_1, nullptr);
            REQUIRE_NE(elem_0, elem_1);
            elem_assert_cb(elem_0, elem_1);
        }

        //destroy handle
        dtor_fn(hdl);
    }


    //a setter & getter test for objects
    template <typename hdl_T, typename obj_T, typename obj_val_T>
    void test_obj_setter_getter(
        const obj_val_T & new_obj,
        hdl_T (* ctor_fn)(), void (* dtor_fn)(hdl_T * hdl),
        int (* setter_fn)(hdl_T * hdl, const obj_val_T *),
        const obj_val_T * (* getter_fn)(const hdl_T *),
        const std::function<
            void(const obj_T &, const obj_val_T *)> dflt_getter_cb, 
        std::function<void(const obj_T &)> new_setter_cb,
        const std::function<
            void(const obj_T &, const obj_val_T *)> new_getter_cb) {

        int ret;


        //setup the object        
        hdl_T * hdl = ctor_fn();
        obj_T * obj = (obj_T *) hdl;

        //run the default value getter checks
        const obj_val_T * obj_0 = getter_fn(hdl);
        dflt_getter_cb(obj, obj_0);

        //run the new value setter checks
        ret = setter_fn(hdl, new_obj);
        CHECK_EQ(ret, 0);
        new_setter_cb(obj);

        //run the new value getter checks
        const obj_val_T * obj_1 = getter_vn(hdl);
        new_getter_cb(obj, obj_1);

        //destroy handle
        dtor_fn(hdl);
    }


    } //end namespace `c`
}
