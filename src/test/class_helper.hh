#pragma once

//standard template library
#include <functional>

//C standard library
#include <cstdlib>

//external libraries
#include <doctest/doctest.h>


/*
 *  NOTE: 
 *
 */

namespace _class_helper {

    /*
     *  C++ interface tests
     */

    namespace cc {

    //(default) constructor test
    template <typename obj_T>
    void test_ctor_dtor(
        std::function<void(const obj_T & obj)> ctor_assert_cb,
        std::function<void(const obj_T & obj)> dtor_assert_cb) {

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
        std::function<void(const obj_T & src_obj)> src_setup_cb,
        std::function<void(const obj_T & dst_obj,
                      const obj_T & src_obj)> copy_ctor_assert_cb,
        std::function<void(const obj_T & dst_obj)> copy_dtor_assert_cb,
        std::function<void(const obj_T & dst_obj)> dtor_assert_cb) {

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
        int (obj_T::*setter_fn)(value_T val),
        value_T (obj_T::*getter_fn)() const,
        std::function<void(obj_T & obj)> setter_assert_cb) {

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
        int (obj_T::*setter_fn)(value_T & val),
        const value_T & (obj_T::*getter_fn)() const,
        std::function<void(obj_T & obj)> setter_assert_cb) {

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
        int (obj_T::*setter_fn)(const ptr_T * ptr),
        ptr_T * (obj_T::*getter_fn)() const,
        std::function<void(obj_T & obj)> setter_assert_cb) {

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


    //a string settter & getter test
    template <typename obj_T>
    void test_str_setter_getter(
        const char * dflt_str, const char * new_str,
        int (obj_T::*setter_fn)(const char * ptr),
        const char * const & (obj_T::*getter_fn)() const,
        std::function<void(obj_T & obj)> setter_assert_cb) {

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

    } //end namespace `cc`



    /*
     *  C interface tests
     */

    namespace c {

    //(default) constructor test
    template <typename hdl_T, typename obj_T>
    void test_ctor_dtor(
        hdl_T (* ctor_fn)(), void (* dtor_fn)(hdl_T * hdl),
        std::function<void(const obj_T & obj)> ctor_assert_cb
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
        hdl_T (* ctor_fn)(), void (* dtor_fn)(hdl_T * hdl),
        hdl_T (* copy_ctor_fn)(const hdl_T * hdl),
        std::function<void(const obj_T & src_obj)> src_setup_cb,
        std::function<void(const obj_T & dst_obj,
                      const obj_T & src_obj)> copy_ctor_assert_cb,
        std::function<void(const obj_T & dst_obj)> copy_dtor_assert_cb
        /* no destination handle destructor assertions callback */) {

        //setup the source object
        hdl_T * src_hdl = ctor_fn();
        obj_T * src_obj = (obj_T *) src_hdl;
        src_setub_cb(*src_obj);

        //run post-copy-constructor checks
        hdl_T * dst_hdl = copy_ctor_fn(src_hdl);
        obj_T * dst_obj = (obj_T *) dst_hdl;
        copy_ctor_assert_cb(*dst_obj);

        //destroy the source object & run checks on destination object
        dtor_fn(src_hdl);
        copy_dtor_assert_cb(*dst_obj);

        //destroy destination handle
        dtor_fn(dst_hdl);

        return;
    }

    
    
    } //end namespace `c`
}
