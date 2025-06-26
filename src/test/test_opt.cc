//standard template library
#include <optional>
#include <vector>
#include <unordered_set>
#include <utility>
#include <functional>

//C standard library
#include <cstring>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <doctest/doctest.h>

//local headers
#include "filters.hh"
#include "common.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/opt.hh"



/*
 *  NOTE: Because the setters & getters are so plentiful, with more on 
 *        the way, generic unit tests have been defined inside templates.
 *
 *        For example, there is a single unit test for optionals passed
 *        and returned by reference. It can be called from any class, as
 *        the class is itself part of the template definition.
 */

/*
 *  FIXME: Copy & move constructor tests are very poor.
 */


      /* ===================== * 
 ===== *  C++ INTERFACE TESTS  * =====
       * ===================== */

/*
 *  --- [HELPERS] ---
 */

//test setting & getting an attribute
template <typename O, typename T>
static void _cc_val_test(O & o, T v,
                         int (O::*set)(const T) noexcept,
                         T (O::*get)() const noexcept) {

    int ret;
    T ret_val;

    //set the value
    ret = (o.*set)(v);
    CHECK_EQ(ret, 0);

    //check the value is set
    ret_val = (o.*get)();
    CHECK_EQ(ret_val, v);

    return;
}


//test setting & getting an optional attribute
template <typename O, typename T>
static void _cc_opt_val_test(O & o, std::optional<T> v,
                             int (O::*set)(const std::optional<T> ),
                             std::optional<T> (O::*get)() const) {

    int ret;                      
    std::optional<T> ret_opt;


    //check optional starts out empty
    ret_opt = (o.*get)();
    CHECK_EQ(ret_opt.has_value(), false);

    //set the optional
    ret = (o.*set)(v);
    CHECK_EQ(ret, 0);

    //check the value is set
    ret_opt = (o.*get)();
    CHECK_EQ(ret_opt.has_value(), true);
    CHECK_EQ(ret_opt.value(), v.value());

    //reset the optional
    ret = (o.*set)(std::nullopt);
    CHECK_EQ(ret, 0);

    //check the value was reset
    ret_opt = (o.*get)();
    CHECK_EQ(ret_opt.has_value(), false);

    return;
}


//test setting & getting an optional attribute
template <typename O, typename T>
static void _cc_opt_ref_test(O & o, std::optional<T> & v,
                             int (O::*set)(const std::optional<T> &),
                             const std::optional<T> & (O::*get)() const) {

    int ret;                      
    std::optional<T> ret_opt;


    //check optional starts out empty
    ret_opt = (o.*get)();
    CHECK_EQ(ret_opt.has_value(), false);

    //set the optional
    ret = (o.*set)(v);
    CHECK_EQ(ret, 0);

    //check the value is set
    ret_opt = (o.*get)();
    CHECK_EQ(ret_opt.has_value(), true);
    CHECK_EQ(ret_opt.value(), v.value());

    //reset the optional
    ret = (o.*set)(std::nullopt);
    CHECK_EQ(ret, 0);

    //check the value was reset
    ret_opt = (o.*get)();
    CHECK_EQ(ret_opt.has_value(), false);

    return;
}


//test setting & getting an optional vector attribute
template <typename O, typename T>
static void _cc_vector_test(O & o, std::vector<T> & v,
                            int (O::*set)(const std::optional<
                                                     std::vector<T>> &),
                            const std::optional<
                                       std::vector<T>> & (O::*get)() const) {

    int ret;
    std::optional<std::vector<T>> ret_opt;


    //check optional vector starts out empty
    ret_opt = (o.*get)();
    CHECK_EQ(ret_opt.has_value(), false);

    //set the optional vector
    ret = (o.*set)(v);
    CHECK_EQ(ret, 0);

    //check the optional vector is set
    ret_opt = (o.*get)();
    CHECK_EQ(ret_opt.has_value(), true);
    CHECK_EQ(ret_opt.value(), v);

    //reset the optional vector
    ret = (o.*set)(std::nullopt);
    CHECK_EQ(ret, 0);

    //check the optional vector was reset
    ret_opt = (o.*get)();
    CHECK_EQ(ret_opt.has_value(), false);
    
    return;
}



/*
 *  --- [TESTS - OPT] ---
 */

TEST_CASE(test_cc_opt_subtests[0]) {

    int ret;

    //test 0: construct opt classes

    //call regular constructor
    sc::opt o(test_cc_addr_width);

    //apply lock to check copy & move constructors reset it
    ret = o._lock();
    CHECK_EQ(ret, 0);

    //call copy & move constructors
    sc::opt o_copy(o);
    CHECK_EQ(o_copy._get_lock(), false);
    ret = o_copy.set_map((const mc_vm_map *) 0x1337);
    CHECK_EQ(ret, 0);
    sc::opt o_move(std::move(o_copy));
    CHECK_EQ(o_move._get_lock(), false);

    //reset lock
    ret = o._unlock();
    CHECK_EQ(ret, 0);


    //test 1: set & get `file_path_out`
    SUBCASE(test_cc_opt_subtests[1]) {
        title(CC, "opt", "Set & get `file_path_out`");

        std::optional<std::string> path  = "/foo/bar";
        _cc_opt_ref_test<sc::opt, std::string>(
            o, path, &sc::opt::set_file_path_out, &sc::opt::get_file_path_out);
                    
    } //end test


    //test 2: set & get `file_path_in`
    SUBCASE(test_cc_opt_subtests[2]) {
        title(CC, "opt", "Set & get `file_path_in`");

        std::optional<std::string> path = "/foo/bar";
        _cc_opt_ref_test<sc::opt, std::string>(
            o, path, &sc::opt::set_file_path_in, &sc::opt::get_file_path_in);

    } //end test


    //test 3: set & get `sessions`
    SUBCASE(test_cc_opt_subtests[3]) {
        title(CC, "opt", "Set & get `sessions`");

        /*
         *  Test implemented directly.
         */

        std::vector<const mc_session *> sessions = {
            (const mc_session *) 0x10101010,
            (const mc_session *) 0x20202020,
            (const mc_session *) 0x30303030
        };

        std::vector<mc_session const *> ret_vct;

        ret_vct = o.get_sessions();
        CHECK_EQ(ret_vct.size(), 0);

        ret = o.set_sessions(sessions);
        CHECK_EQ(ret, 0);

        ret_vct = o.get_sessions();
        CHECK_EQ(ret_vct, sessions);
        
    } //end test


    //test 4: set & get `map`
    SUBCASE(test_cc_opt_subtests[4]) {
        title(CC, "opt", "Set & get `map`");

        /*
         *  Test implemented directly.
         */

        mc_vm_map const * ret_map;

        ret_map = o.get_map();
        CHECK_EQ(ret_map, nullptr);

        ret = o.set_map((mc_vm_map const *) 0x1337);
        CHECK_EQ(ret, 0);

        ret_map = o.get_map();
        CHECK_EQ(ret_map, (mc_vm_map const *) 0x1337);
        
    } //end test

    
    //test 5: set & get `omit_areas`
    SUBCASE(test_cc_opt_subtests[5]) {
        title(CC, "opt", "Set & get `omit_areas`");

        std::vector<const cm_lst_node *> s = {
            (const cm_lst_node *) 0x40404040,
            (const cm_lst_node *) 0x50505050,
            (const cm_lst_node *) 0x60606060
        };

        //call constraint test helper        
        _cc_vector_test<sc::opt, const cm_lst_node *>(
            o, s, &sc::opt::set_omit_areas, &sc::opt::get_omit_areas);
                        
    } //end test


    //test 6: set & get `omit_objs`
    SUBCASE(test_cc_opt_subtests[6]) {
        title(CC, "opt", "Set & get `omit_objs`");

        std::vector<const cm_lst_node *> s = {
            (const cm_lst_node *) 0x04040404,
            (const cm_lst_node *) 0x05050505,
            (const cm_lst_node *) 0x06060606
        };

        _cc_vector_test<sc::opt, const cm_lst_node *>(
            o, s, &sc::opt::set_omit_objs, &sc::opt::get_omit_objs);

    } //end test


    //test 7: set & get `exclusive_areas`
    SUBCASE(test_cc_opt_subtests[7]) {
        title(CC, "opt", "Set & get `exclusive_areas`");

        std::vector<const cm_lst_node *> s = {
            (const cm_lst_node *) 0x70707070,
            (const cm_lst_node *) 0x80808080,
            (const cm_lst_node *) 0x90909090
        };

        _cc_vector_test<sc::opt, const cm_lst_node *>(
            o, s, &sc::opt::set_exclusive_areas, &sc::opt::get_exclusive_areas);
        
    } //end test

    
    //test 8: set & get `exclusive_objs`
    SUBCASE(test_cc_opt_subtests[8]) {
        title(CC, "opt", "Set & get `exclusive_objs`");
        
        std::vector<const cm_lst_node *> s = {
            (const cm_lst_node *) 0x01010101,
            (const cm_lst_node *) 0x02020202,
            (const cm_lst_node *) 0x03030303
        };

        _cc_vector_test<sc::opt, const cm_lst_node *>(
            o, s, &sc::opt::set_omit_objs, &sc::opt::get_omit_objs);

    } //end test


    //test 9: get & set `omit_addr_ranges`
    SUBCASE(test_cc_opt_subtests[9]) {
        title(CC, "opt", "Set & get `omit_addr_ranges`");

        std::vector<std::pair<uintptr_t, uintptr_t>> ranges = {
            {0x1000, 0x2000},
            {0x3000, 0x4000},
            {0x5000, 0x6000}
        };

        _cc_vector_test<sc::opt, std::pair<uintptr_t, uintptr_t>>(
            o, ranges,
            &sc::opt::set_omit_addr_ranges,
            &sc::opt::get_omit_addr_ranges);
            
    } //end test


    //test 10: get & set `exclusive_addr_ranges`
    SUBCASE(test_cc_opt_subtests[9]) {
        title(CC, "opt", "Set & get `exclusive_addr_ranges`");

        std::vector<std::pair<uintptr_t, uintptr_t>> ranges = {
            {0x1000, 0x2000},
            {0x3000, 0x4000},
            {0x5000, 0x6000}
        };

        _cc_vector_test<sc::opt, std::pair<uintptr_t, uintptr_t>>(
            o, ranges,
            &sc::opt::set_exclusive_addr_ranges,
            &sc::opt::get_exclusive_addr_ranges);
            
    } //end test


    //test 11: set & get `access`
    SUBCASE(test_cc_opt_subtests[11]) {
        title(CC, "opt", "Set & get `access`");

        std::optional<cm_byte> access = MC_ACCESS_READ | MC_ACCESS_WRITE;
        _cc_opt_val_test<sc::opt, cm_byte>(o, access,
                    &sc::opt::set_access, &sc::opt::get_access);

    } //end test


    //test 12: reset
    SUBCASE(test_cc_opt_subtests[12]) {

        /*
         *  TODO Implement.
         */
    }

    return;

} //end TEST_CASE 



/*
 *  --- [TESTS - OPT_PTRSCAN] ---
 */

TEST_CASE(test_cc_opt_ptrscan_subtests[0]) {

    int ret;


    //test 0: construct opt_ptrscan classes

    //call regular constructor
    sc::opt_ptrscan o;

    //apply lock to check copy & move constructors reset it
    ret = o._lock();
    CHECK_EQ(ret, 0);

    //call copy & move constructors
    sc::opt_ptrscan o_copy(o);
    CHECK_EQ(o_copy._get_lock(), false);
    sc::opt_ptrscan o_move(std::move(o));
    CHECK_EQ(o_move._get_lock(), false);

    //reset lock
    ret = o._unlock();
    CHECK_EQ(ret, 0);


    //test 1: set & get `target_addr`
    SUBCASE(test_cc_opt_ptrscan_subtests[1]) {
        title(CC, "opt_ptrscan", "Set & get `target_addr`");

        _cc_opt_val_test<sc::opt_ptrscan, uintptr_t>(
                            o, 0x1000,
                            &sc::opt_ptrscan::set_target_addr,
                            &sc::opt_ptrscan::get_target_addr);

    } //end test


    //test 2: set & get `alignment`
    SUBCASE(test_cc_opt_ptrscan_subtests[2]) {
        title(CC, "opt_ptrscan", "Set & get `alignment`");

        _cc_opt_val_test<sc::opt_ptrscan, off_t>(
                            o, 0x4,
                            &sc::opt_ptrscan::set_alignment,
                            &sc::opt_ptrscan::get_alignment);

    } //end test


    //test 3: set & get `max_obj_sz`
    SUBCASE(test_cc_opt_ptrscan_subtests[3]) {
        title(CC, "opt_ptrscan", "Set & get `max_obj_sz`");

        _cc_opt_val_test<sc::opt_ptrscan, off_t>(
                            o, 0x4,
                            &sc::opt_ptrscan::set_max_obj_sz,
                            &sc::opt_ptrscan::get_max_obj_sz);

    } //end test


    //test 4: set & get `max_depth`
    SUBCASE(test_cc_opt_ptrscan_subtests[4]) {
        title(CC, "opt_ptrscan", "Set & get `max_depth`");

        _cc_opt_val_test<sc::opt_ptrscan, int>(
                            o, 0x4,
                            &sc::opt_ptrscan::set_max_depth,
                            &sc::opt_ptrscan::get_max_depth);

    } //end test


    //test 5: set & get `static_areas`
    SUBCASE(test_cc_opt_ptrscan_subtests[5]) {
        title(CC, "opt_ptrscan", "Set & get `static_areas`");

        /*
         *  Test implemented directly.
         */

        std::vector<const cm_lst_node *> static_areas = {
            (const cm_lst_node *) 0x10101010,
            (const cm_lst_node *) 0x20202020,
            (const cm_lst_node *) 0x30303030
        };
        
        std::optional<std::unordered_set<const cm_lst_node *>> ret_opt;


        //check optional vector starts out empty
        ret_opt = o.get_static_areas();
        CHECK_EQ(ret_opt.has_value(), false);

        //set the optional vector
        ret = o.set_static_areas(static_areas);
        CHECK_EQ(ret, 0);

        //check the optional vector is set
        ret_opt = o.get_static_areas();
        CHECK_EQ(ret_opt.has_value(), true);
        CHECK_EQ(ret_opt->size(), 3);

        //check all static areas are present in the set
        for (auto iter = static_areas.cbegin();
             iter != static_areas.cend(); ++iter) {
            CHECK_EQ(ret_opt->count(*iter), 1);
        }

        //reset the optional vector
        ret = o.set_static_areas(std::nullopt);
        CHECK_EQ(ret, 0);

        //check the optional vector was reset
        ret_opt = o.get_static_areas();
        CHECK_EQ(ret_opt.has_value(), false);
 
    } //end test


    //test 6: set & get `preset_offsets`
    SUBCASE(test_cc_opt_ptrscan_subtests[6]) {
        title(CC, "opt_ptrscan", "Set & get `preset_offsets`");

        std::vector<off_t> preset_offsets = {
            0x40,
            0x80,
            0x100
        };

        _cc_vector_test<sc::opt_ptrscan, off_t>(
                            o, preset_offsets,
                            &sc::opt_ptrscan::set_preset_offsets,
                            &sc::opt_ptrscan::get_preset_offsets);

    } //end test

    
    //test 7: set & get `smart_scan`
    SUBCASE(test_cc_opt_ptrscan_subtests[7]) {
        title(CC, "opt_ptrscan", "Set & get `smart_scan`");

        _cc_val_test<sc::opt_ptrscan, bool>(
                            o, true,
                            &sc::opt_ptrscan::set_smart_scan,
                            &sc::opt_ptrscan::get_smart_scan);

    } //end test


    //test 8: reset
    SUBCASE(test_cc_opt_ptrscan_subtests[8]) {

        //TODO: Implement.

    }

    return;

} //end TEST_CASE 




      /* =================== * 
 ===== *  C INTERFACE TESTS  * =====
       * =================== */

/*
 *  --- [HELPERS] ---
 */

//test setting & getting a value
template <typename O, typename T>
static void _c_val_test(void * o, T v,
                        int (* set)(O o, const T v),
                        T (* get)(const O o),
                        std::optional<
                            std::function<bool(T v_1, T v_2)>> compare_fn) {
                            
    int ret;
    T ret_val;


    /* first test: typical use-case */

    //call setter
    ret = (*set)(o, v);
    CHECK_EQ(ret, 0);

    //call getter again to assert the attribute was set
    ret_val = (*get)(o);
    if (compare_fn.has_value() == true) {
        compare_fn.value()(ret_val, v);
    } else {
        CHECK(ret_val == v);
    }

    //cleanup before next test
    sc_errno = 0;

    return;
}


//test setting & getting an optional
template <typename O, typename T>
static void _c_opt_test(void * o, T v, T v_nullopt,
                        int (* set)(O o, const T v),
                        T (* get)(const O o),
                        std::optional<
                            std::function<bool(T v_1, T v_2)>> compare_fn) {
  
    int ret;
    T ret_opt;


    /* first test: typical use-case */

    //call getter before attribute is set
    ret_opt = (*get)(o);
    CHECK_EQ(ret_opt, v_nullopt);

    //call setter
    ret = (*set)(o, v);
    CHECK_EQ(ret, 0);

    //call getter again to assert the attribute was set
    ret_opt = (*get)(o);
    if (compare_fn.has_value() == true) {
        compare_fn.value()(ret_opt, v);
    } else {
        CHECK(ret_opt == v);
    }

    //cleanup before next test
    sc_errno = 0;


    /* second test: reset optional back to nullopt */

    //call setter with this attribute's nullopt C equivalent
    ret = (*set)(o, v_nullopt);
    CHECK_EQ(ret, 0);

    //try to get the attribute to check it's been set to nullopt correctly
    ret_opt = (*get)(o);
    CHECK_EQ(ret_opt, v_nullopt);
    CHECK_EQ(sc_errno, SC_ERR_OPT_EMPTY);

    return;
}


//test setting & getting an optional vector attribute
template <typename O, typename T>
static void _c_vector_test(void * o, T * a, int a_len,
                           int (* set)(O o, const cm_vct * v),
                           int (* get)(const O o, cm_vct * v),
                            std::optional<
                                std::function<bool(T v_1, T v_2)>> compare_fn) {

    int ret;

    T entry;
    cm_vct vct_1, vct_2;


    //construct a CMore vector to pass to the C interface call
    ret = cm_new_vct(&vct_1, sizeof(T));
    CHECK_EQ(ret, 0);

    //populate the newly created CMore vector
    for (int i = 0; i < a_len; ++i) {
        ret = cm_vct_apd(&vct_1, &a[i]);
        CHECK_EQ(ret, 0);
    }


    /* first test: typical use-case */

    //try to get the unset optional
    ret = get(o, &vct_2);
    CHECK_EQ(ret, -1);
    CHECK_EQ(sc_errno, SC_ERR_OPT_EMPTY);

    //fill the optional with the previously constructed CMore vector
    ret = set(o, &vct_1);
    CHECK_EQ(ret, 0);

    //call the getter again
    ret = get(o, &vct_2);
    CHECK_EQ(ret, 0);
    CHECK_EQ(vct_2.len, a_len);

    //check each element from the returned CMore vector is correct
    for (int i = 0; i < a_len; ++i) {
    
        ret = cm_vct_get(&vct_2, i, &entry);
        CHECK_EQ(ret, 0);

        if (compare_fn.has_value() == true) {
            compare_fn.value()(entry, a[i]);
        } else {
            CHECK(std::memcmp(&entry, &a[i], sizeof(entry)) == 0);
        }
    } //end for

    //cleanup before next test
    cm_del_vct(&vct_2);
    sc_errno = 0;


    /* second test: reset optional back to nullopt */

    //call setter with this attribute's nullopt C equivalent
    ret = set(o, nullptr);
    CHECK_EQ(ret, 0);

    //try to get the attribute to check it's been set to nullopt correctly
    ret = get(o, &vct_2);
    CHECK_EQ(ret, -1);
    CHECK_EQ(sc_errno, SC_ERR_OPT_EMPTY);

    //cleanup
    cm_del_vct(&vct_1);

    return;
}


/*
 *  --- [TESTS - OPT] ---
 */

TEST_CASE(test_c_opt_subtests[0]) {

    int ret;
    sc_opt o, o_copy;


    //test 0: create sc_opts

    //call constructor
    o = sc_new_opt(test_c_addr_width);
    REQUIRE_NE(o, nullptr);

    //call copy constructor
    o_copy = sc_copy_opt(o);
    REQUIRE_NE(o_copy, nullptr);


    //test 1: set & get `file_path_out`
    SUBCASE(test_c_opt_subtests[1]) {
        title(C, "sc_opt", "Set & get `file_path_out`");

        const char * path = "/foo/bar";

        _c_opt_test<sc_opt, const char *>(
                            o, path, nullptr,
                            sc_opt_set_file_path_out,
                            sc_opt_get_file_path_out,
                            [](const char * s_1, const char * s_2) -> bool {
                                
                                std::string stl_s_1(s_1 == nullptr ? "" : s_1);
                                std::string stl_s_2(s_2 == nullptr ? "" : s_2);
                                return stl_s_1 == stl_s_2;
                            });
    } //end test


    //test 2: set & get `file_path_in`
    SUBCASE(test_c_opt_subtests[2]) {
        title(C, "sc_opt", "Set & get `file_path_in`");

        const char * path = "/foo/bar";

        _c_opt_test<sc_opt, const char *>(
                            o, path, nullptr,
                            sc_opt_set_file_path_in,
                            sc_opt_get_file_path_in,
                            [](const char * s_1, const char * s_2) -> bool {
                                std::string stl_s_1(s_1 == nullptr ? "" : s_1);
                                std::string stl_s_2(s_2 == nullptr ? "" : s_2);
                                return stl_s_1 == stl_s_2;
                            });

    } //end test


    //test 3: set & get `sessions`
    SUBCASE(test_c_opt_subtests[3]) {
        title(C, "sc_opt", "Set & get `sessions`");

        /*
         *  Test implemented directly.
         */

        cm_lst_node * a[3] = {
            (cm_lst_node *) 0x10101010,
            (cm_lst_node *) 0x20202020,
            (cm_lst_node *) 0x30303030
        };

        cm_lst_node * s;
        cm_vct vct_1, vct_2;


        //construct a CMore vector to pass to the C interface call
        ret = cm_new_vct(&vct_1, sizeof(cm_lst_node *));
        CHECK_EQ(ret, 0);

        //populate the newly created CMore vector
        for (int i = 0; i < 3; ++i) {
            ret = cm_vct_apd(&vct_1, &a[i]);
            CHECK_EQ(ret, 0);
        }


        /* first test: typical use-case */

        //try to get sessions before they are set
        ret = sc_opt_get_sessions(o, &vct_2);
        CHECK_EQ(ret, 0);
        cm_del_vct(&vct_2);

        //fill sessions with the previously constructed CMore optional
        ret = sc_opt_set_sessions(o, &vct_1);
        CHECK_EQ(ret, 0);

        //call the getter again & assert
        ret = sc_opt_get_sessions(o, &vct_2);
        CHECK_EQ(ret, 0);
        CHECK_EQ(vct_2.len, 3);
    
        //check each element from the returned CMore vector is correct
        for (int i = 0; i < 3; ++i) {
        
            ret = cm_vct_get(&vct_2, i, &s);
            CHECK_EQ(ret, 0);
            CHECK_EQ(s, a[i]);
        }

        //cleanup before next test
        cm_del_vct(&vct_1);
        cm_del_vct(&vct_2);
        sc_errno = 0;


        /* second test: replace sessions with an empty vector */

        //create an empty CMore vector
        cm_new_vct(&vct_1, sizeof(cm_lst_node *));

        //call the setter, passing it the empty CMore vector
        ret = sc_opt_set_sessions(o, &vct_1);
        CHECK_EQ(ret, 0);

        //try to get the sessions, checking the length is now 0
        ret = sc_opt_get_sessions(o, &vct_2);
        CHECK_EQ(ret, 0);
        CHECK_EQ(vct_2.len, 0);

        //cleanup
        cm_del_vct(&vct_1);
        cm_del_vct(&vct_2);
        
    } //end test


    //test 4: set & get `map`
    SUBCASE(test_c_opt_subtests[4]) {
        title(C, "sc_opt", "Set & get `map`");

        /*
         *  NOTE: The setter signature is cast to fix constness of the
         *        `mc_vm_map` parameter.
         */

        int (* set_cast)(sc_opt opts, mc_vm_map * const)
            = (int(*)(sc_opt opts, mc_vm_map * const)) sc_opt_set_map;

        _c_val_test<sc_opt, mc_vm_map *>(o, (mc_vm_map *) 0x1337,
            set_cast, sc_opt_get_map, std::nullopt);
    
    } //end test


    //test 5: get `addr_width`
    SUBCASE(test_c_opt_subtests[5]) {
        title(C, "sc_opt", "Get `addr_width`");

        /*
         *  Test implemented directly.
         */

        sc_addr_width ret_aw;

        //call the getter for this const attribute
        ret_aw = sc_opt_get_addr_width(o);
        CHECK_EQ(ret_aw, test_c_addr_width);

    } //end test


    //test 6: set & get `omit_areas`
    SUBCASE(test_c_opt_subtests[6]) {
        title(C, "sc_opt", "Set & get `omit_areas`");

        const cm_lst_node * a[3] = {
            (const cm_lst_node *) 0x40404040,
            (const cm_lst_node *) 0x50505050,
            (const cm_lst_node *) 0x60606060
        };

        _c_vector_test<sc_opt, const cm_lst_node *>(
            o, a, 3, sc_opt_set_omit_areas,
            sc_opt_get_omit_areas, std::nullopt);

    } //end test


    //test 7: set & get `omit_objs`
    SUBCASE(test_c_opt_subtests[7]) {
        title(C, "sc_opt", "Set & get `omit_objs`");

        const cm_lst_node * a[3] = {
            (const cm_lst_node *) 0x04040404,
            (const cm_lst_node *) 0x05050505,
            (const cm_lst_node *) 0x06060606
        };

        _c_vector_test<sc_opt, const cm_lst_node *>(
            o, a, 3, sc_opt_set_omit_objs,
            sc_opt_get_omit_objs, std::nullopt);

    } //end test


    //test 8: set & get `exclusive_areas`
    SUBCASE(test_c_opt_subtests[8]) {
        title(C, "sc_opt", "Set & get `exclusive_areas`");

        const cm_lst_node * a[3] = {
            (const cm_lst_node *) 0x70707070,
            (const cm_lst_node *) 0x80808080,
            (const cm_lst_node *) 0x90909090
        };

        _c_vector_test<sc_opt, const cm_lst_node *>(
            o, a, 3, sc_opt_set_exclusive_areas,
            sc_opt_get_exclusive_areas, std::nullopt);

    } //end test


    //test 9: set & get `exclusive_objs`
    SUBCASE(test_c_opt_subtests[9]) {
        title(C, "sc_opt", "Set & get `exclusive_objs`");

        const cm_lst_node * a[3] = {
            (const cm_lst_node *) 0x07070707,
            (const cm_lst_node *) 0x08080808,
            (const cm_lst_node *) 0x09090909
        };

        _c_vector_test<sc_opt, const cm_lst_node *>(
            o, a, 3, sc_opt_set_exclusive_objs,
            sc_opt_get_exclusive_objs, std::nullopt);

    } //end test


    //test 10: set & get an `omit_addr_ranges`
    SUBCASE(test_c_opt_subtests[10]) {
        title(C, "sc_opt", "Set & get `omit_addr_ranges`");

        sc_addr_range a[3] = {
            {0x1000, 0x2000},
            {0x3000, 0x4000},
            {0x5000, 0x6000}
        };

        _c_vector_test<sc_opt, sc_addr_range>(
            o, a, 3,
            sc_opt_set_omit_addr_ranges,
            sc_opt_get_omit_addr_ranges,
            [](sc_addr_range ar_1, sc_addr_range ar_2) -> bool {
                return (ar_1.min == ar_2.min) && (ar_1.max == ar_2.max);
            });

    } //end test


    //test 11: set & get an `exclusive_addr_ranges`
    SUBCASE(test_c_opt_subtests[11]) {
        title(C, "sc_opt", "Set & get `exclusive_addr_ranges`");

        sc_addr_range a[3] = {
            {0x1000, 0x2000},
            {0x3000, 0x4000},
            {0x5000, 0x6000}
        };

        _c_vector_test<sc_opt, sc_addr_range>(
            o, a, 3,
            sc_opt_set_exclusive_addr_ranges,
            sc_opt_get_exclusive_addr_ranges,
            [](sc_addr_range ar_1, sc_addr_range ar_2) -> bool {
                return (ar_1.min == ar_2.min) && (ar_1.max == ar_2.max);
            });

    } //end test


    //test 12: set & get `access`
    SUBCASE(test_c_opt_subtests[12]) {
        title(C, "sc_opt", "Set & get `access`");

        _c_opt_test<sc_opt, cm_byte>(
                            o, MC_ACCESS_READ | MC_ACCESS_WRITE, -1,
                            sc_opt_set_access, sc_opt_get_access, std::nullopt);
      
    } //end test


    //test 0 (cont.): destroy the options objects
    int _ret = sc_del_opt(o);
    CHECK_EQ(_ret, 0);

    _ret = sc_del_opt(o_copy);
    CHECK_EQ(_ret, 0);


} //end TEST_CASE



/*
 *  --- [TESTS - OPT_PTRSCAN] ---
 */

TEST_CASE(test_c_opt_ptrscan_subtests[0]) {

    int ret;
    sc_opt_ptrscan o, o_copy;


    //test 0: create sc_opt_ptrscans

    //call constructor
    o = sc_new_opt_ptrscan();
    REQUIRE_NE(o, nullptr);

    //call copy constructor
    o_copy = sc_copy_opt_ptrscan(o);
    REQUIRE_NE(o_copy, nullptr);


    //test 1: set & get `target_addr`
    SUBCASE(test_c_opt_ptrscan_subtests[1]) {
        title(C, "sc_opt_ptrscan", "Set & get `target_addr`");

        _c_opt_test<sc_opt_ptrscan, uintptr_t>(
                            o, 0x1000, 0,
                            sc_opt_ptr_set_target_addr,
                            sc_opt_ptr_get_target_addr,
                            std::nullopt);

    } //end test


    //test 2: set & get `alignment`
    SUBCASE(test_c_opt_ptrscan_subtests[2]) {

        _c_opt_test<sc_opt_ptrscan, off_t>(
                            o, 0x4, 0,
                            sc_opt_ptr_set_alignment,
                            sc_opt_ptr_get_alignment,
                            std::nullopt);

    } //end test


    //test 3: set & get `max_obj_sz`
    SUBCASE(test_c_opt_ptrscan_subtests[3]) {
        title(C, "sc_opt_ptrscan", "Set & get `max_obj_sz`");

        _c_opt_test<sc_opt_ptrscan, off_t>(
                            o, 0x4, 0,
                            sc_opt_ptr_set_max_obj_sz,
                            sc_opt_ptr_get_max_obj_sz,
                            std::nullopt);

    } //end test


    //test 4: set & get `max_depth`
    SUBCASE(test_c_opt_ptrscan_subtests[4]) {
        title(C, "sc_opt_ptrscan", "Set & get `max_depth`");

        _c_opt_test<sc_opt_ptrscan, int>(
                            o, 0x4, 0,
                            sc_opt_ptr_set_max_depth,
                            sc_opt_ptr_get_max_depth,
                            std::nullopt);

    } //end test

    //test 5: set & get `static_areas`
    SUBCASE(test_c_opt_ptrscan_subtests[5]) {
        title(C, "sc_opt_ptrscan", "Set & get `static_areas`");

        /*
         *  NOTE: The C interface converts the STL hashmap into an
         *        ordered CMore vector. This means the templated vector
         *        test perfectly applicable here.
         *
         *        For the unordered set to be correctly sorted, it must
         *        be able to sort nodes based on start & end addresses
         *        of their underlying areas. For this to take place, the
         *        below stubs are defined.
         */

        cm_lst_node * area_node_ptrs_arr[3] = {};
        cm_lst_node area_nodes_arr[3] = {};
        mc_vm_area areas_arr[3] = {};

        //populate MemCry area stubs
        areas_arr[0].start_addr = 0x1000; areas_arr[0].end_addr = 0x2000;
        areas_arr[1].start_addr = 0x3000; areas_arr[1].end_addr = 0x4000;
        areas_arr[2].start_addr = 0x5000; areas_arr[2].end_addr = 0x6000;

        //populate node stubs
        area_nodes_arr[0].data = &areas_arr[0];
        area_nodes_arr[1].data = &areas_arr[1];
        area_nodes_arr[2].data = &areas_arr[2];

        //populate node pointers
        area_node_ptrs_arr[0] = &area_nodes_arr[0];
        area_node_ptrs_arr[1] = &area_nodes_arr[1];
        area_node_ptrs_arr[2] = &area_nodes_arr[2];

        _c_vector_test<sc_opt_ptrscan, const cm_lst_node *>(
            o, (const cm_lst_node **) area_node_ptrs_arr,
            3, sc_opt_ptr_set_static_areas,
            sc_opt_ptr_get_static_areas, std::nullopt);

    } //end test


    //test 6: set & get `preset_offset`
    SUBCASE(test_c_opt_subtests[6]) {
        title(C, "sc_opt_ptrscan", "Set & get `preset_offsets`");

        off_t offset_arr[3] = {
            0x40,
            0x80,
            0x100
        };

        _c_vector_test<sc_opt_ptrscan, off_t>(
            o, offset_arr, 3, sc_opt_ptr_set_preset_offsets,
            sc_opt_ptr_get_preset_offsets, std::nullopt);

    } //end test


    //test 7: set & get `smart_scan`
    SUBCASE(test_c_opt_ptrscan_subtests[7]) {
        title(C, "sc_opt_ptrscan", "Set & get `smart_scan`");

        _c_val_test<sc_opt_ptrscan, bool>(
            o, true, sc_opt_ptr_set_smart_scan,
            sc_opt_ptr_get_smart_scan, std::nullopt);

    } //end test


    //test 0 (cont.): destroy the pointer scan options objects
    int _ret = sc_del_opt_ptrscan(o);
    CHECK_EQ(_ret, 0);

    _ret = sc_del_opt_ptrscan(o_copy);
    CHECK_EQ(_ret, 0);


} //end TEST_CASE

