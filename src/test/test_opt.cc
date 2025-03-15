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



      /* ===================== * 
 ===== *  C++ INTERFACE TESTS  * =====
       * ===================== */

/*
 *  --- [HELPERS] ---
 */

//test constraints
static void _cc_constraint_test(sc::opt & o, std::vector<cm_lst_node *> & v,
                                void (sc::opt::*set)(const std::vector<cm_lst_node *> &),
                                const std::optional<std::vector<cm_lst_node *>>
                                & (sc::opt::*get)() const) {

    //check constraint starts out empty
    const std::optional<std::vector<cm_lst_node *>> ret_1 = (o.*get)();
    CHECK_EQ(ret_1.has_value(), false);

    //set constraint
    (o.*set)(v);

    //get newly set constraint
    const std::optional<std::vector<cm_lst_node *>> ret_2 = (o.*get)();
    CHECK_EQ(ret_2.value(), v);

    return;
}



/*
 *  --- [TESTS] ---
 */

TEST_CASE(test_cc_opt_subtests[0]) {

    //test 0: construct a opt class
    sc::opt o(test_arch_byte_width);


    //test 1: set & get `file_path_out`
    SUBCASE(test_cc_opt_subtests[1]) {

        std::optional<std::string> ret;
        
        ret = o.get_file_path_out();
        CHECK_EQ(ret.has_value(), false);

        o.set_file_path_out("/foo/bar");
        ret = o.get_file_path_out();
        CHECK_EQ(ret.value(), "/foo/bar");
        
    } //end test 1


    //test 2: set & get `file_path_in`
    SUBCASE(test_cc_opt_subtests[2]) {

        std::optional<std::string> ret;

        ret = o.get_file_path_in();
        CHECK_EQ(ret.has_value(), false);

        o.set_file_path_in("/foo/bar");
        ret = o.get_file_path_in();
        CHECK_EQ(ret.value(), "/foo/bar");

    } //end test 2


    //test 3: set & get `sessions`
    SUBCASE(test_cc_opt_subtests[3]) {

        std::vector<const mc_session *> s = {
            (const mc_session *) 0x10101010,
            (const mc_session *) 0x20202020,
            (const mc_session *) 0x30303030
        };

        std::vector<mc_session const *> ret;

        ret = o.get_sessions();
        CHECK_EQ(ret.size(), 0);

        o.set_sessions(s);
        ret = o.get_sessions();
        CHECK_EQ(ret, s);
        
    } //end test


    //test 4: set & get `map`
    SUBCASE(test_cc_opt_subtests[4]) {

        mc_vm_map const * ret;

        ret = o.get_map();
        CHECK_EQ(ret, nullptr);

        o.set_map((mc_vm_map *) 0x1337);
        ret = o.get_map();
        CHECK_EQ(ret, (mc_vm_map const *) 0x1337);
        
    } //end test

    
    //test 5: set & get `alignment`
    SUBCASE(test_cc_opt_subtests[5]) {

        std::optional<unsigned int> ret;

        ret = o.get_alignment();
        CHECK_EQ(ret.has_value(), false);

        o.set_alignment(4);
        ret = o.get_alignment();
        CHECK_EQ(ret.value(), 4);
        
    } //end test

    
    //test 6: get `arch_byte_width`
    SUBCASE(test_cc_opt_subtests[6]) {

        unsigned int ret;

        ret = o.get_arch_byte_width();
        CHECK_EQ(ret, test_arch_byte_width);
    
    } //end test


    //test 7: set & get `omit_areas`
    SUBCASE(test_cc_opt_subtests[7]) {

        std::vector<cm_lst_node *> s = {
            (cm_lst_node *) 0x40404040,
            (cm_lst_node *) 0x50505050,
            (cm_lst_node *) 0x60606060
        };

        //call constraint test helper        
        _cc_constraint_test(o, s, &sc::opt::set_omit_areas,
                            &sc::opt::get_omit_areas);
                        
    } //end test


    //test 8: set & get `omit_objs`
    SUBCASE(test_cc_opt_subtests[8]) {

        std::vector<cm_lst_node *> s = {
            (cm_lst_node *) 0x04040404,
            (cm_lst_node *) 0x05050505,
            (cm_lst_node *) 0x06060606
        };

        //call constraint test helper        
        _cc_constraint_test(o, s, &sc::opt::set_omit_objs,
                            &sc::opt::get_omit_objs);

    } //end test


    //test 9: set & get `exclusive_areas`
    SUBCASE(test_cc_opt_subtests[9]) {

        std::vector<cm_lst_node *> s = {
            (cm_lst_node *) 0x70707070,
            (cm_lst_node *) 0x80808080,
            (cm_lst_node *) 0x90909090
        };

        //call constraint test helper        
        _cc_constraint_test(o, s, &sc::opt::set_exclusive_areas,
                            &sc::opt::get_exclusive_areas);
        
    } //end test

    
    //test 10: set & get `exclusive_objs`
    SUBCASE(test_cc_opt_subtests[10]) {
        
        std::vector<cm_lst_node *> s = {
            (cm_lst_node *) 0x01010101,
            (cm_lst_node *) 0x02020202,
            (cm_lst_node *) 0x03030303
        };

        //call constraint test helper        
        _cc_constraint_test(o, s, &sc::opt::set_omit_objs,
                            &sc::opt::get_omit_objs);
    } //end test


    //test 11: get & set `addr_range`
    SUBCASE(test_cc_opt_subtests[11]) {

        std::pair<uintptr_t, uintptr_t> p = {4, 2};

        const std::optional<std::pair<uintptr_t, uintptr_t>> ret_1
            = o.get_addr_range();
        CHECK_EQ(ret_1.has_value(), false);

        o.set_addr_range(p);
        const std::optional<std::pair<uintptr_t, uintptr_t>> ret_2
            = o.get_addr_range();
        CHECK_EQ(ret_2.value(), p);

    } //end test


    //test 12: set & get `access`
    SUBCASE(test_cc_opt_subtests[12]) {

        std::optional<cm_byte> ret;

        ret = o.get_access();
        CHECK_EQ(ret.has_value(), false);

        o.set_access(MC_ACCESS_READ | MC_ACCESS_WRITE);
        ret = o.get_alignment();
        CHECK_EQ(ret.value(), MC_ACCESS_READ | MC_ACCESS_WRITE);
        
    } //end test


    return;

} //end TEST_CASE 



      /* =================== * 
 ===== *  C INTERFACE TESTS  * =====
       * =================== */

/*
 *  --- [HELPERS] ---
 */

//test constraints
static void _c_constraint_test(sc_opt o, cm_lst_node * a[3],
                               int (* set)(sc_opt o, const cm_vct * v),
                               int (* get)(const sc_opt o, cm_vct * v)) {

        int ret;
        cm_lst_node * s;
        cm_vct v, w;


        //setup vector
        ret = cm_new_vct(&v, sizeof(cm_lst_node *));
        CHECK_EQ(ret, 0);

        for (int i = 0; i < 3; ++i) {
            ret = cm_vct_apd(&v, &a[i]);
            CHECK_EQ(ret, 0);
        }

        //run setters & getters
        ret = get(o, &w);
        CHECK_EQ(ret, -1);
        CHECK_EQ(sc_errno, SC_ERR_OPT_EMPTY);

        ret = set(o, &v);
        CHECK_EQ(ret, 0);

        ret = get(o, &w);
        for (int i = 0; i < 3; ++i) {
        
            ret = cm_vct_get(&v, i, &s);
            CHECK_EQ(ret, 0);
            CHECK_EQ(s, a[i]);
        }

        cm_del_vct(&v);
        cm_del_vct(&w);
 
    return;
}


/*
 *  --- [TESTS] ---
 */

TEST_CASE(test_c_opt_subtests[0]) {

    sc_opt o;

    //test 0: create a sc_opt
    o = sc_new_opt(test_arch_byte_width);
    REQUIRE_NE(o, nullptr);
    

    //test 1: set & get `file_path_out`
    SUBCASE(test_c_opt_subtests[1]) {

        int ret;
        const char * p = "/foo/bar";

        const char * ret_1 = sc_opt_get_file_path_out(o);
        CHECK_EQ(ret_1, nullptr);

        ret = sc_opt_set_file_path_out(o, p);
        CHECK_EQ(ret, 0);
        const char * ret_2 = sc_opt_get_file_path_out(o);
        CHECK(std::string(ret_2) == p);

    } //end test


    //test 2: set & get `file_path_in`
    SUBCASE(test_c_opt_subtests[2]) {

        int ret;
        const char * p = "/foo/bar";

        const char * ret_1 = sc_opt_get_file_path_in(o);
        CHECK_EQ(ret_1, nullptr);

        ret = sc_opt_set_file_path_in(o, p);
        CHECK_EQ(ret, 0);
        const char * ret_2 = sc_opt_get_file_path_in(o);
        CHECK(std::string(ret_2) == p);
        
    } //end test


    //test 3: set & get `sessions`
    SUBCASE(test_c_opt_subtests[3]) {

        cm_lst_node * a[3] = {
            (cm_lst_node *) 0x10101010,
            (cm_lst_node *) 0x20202020,
            (cm_lst_node *) 0x30303030
        };

        int ret;
        cm_lst_node * s;
        cm_vct v, w;


        //setup vector
        ret = cm_new_vct(&v, sizeof(cm_lst_node *));
        CHECK_EQ(ret, 0);

        for (int i = 0; i < 3; ++i) {
            ret = cm_vct_apd(&v, &a[i]);
            CHECK_EQ(ret, 0);
        }

        //run setters & getters
        ret = sc_opt_get_sessions(o, &w);
        CHECK_EQ(ret, -1);
        CHECK_EQ(sc_errno, SC_ERR_OPT_EMPTY);

        ret = sc_opt_set_sessions(o, &v);
        CHECK_EQ(ret, 0);

        ret = sc_opt_get_sessions(o, &w);
        for (int i = 0; i < 3; ++i) {
        
            ret = cm_vct_get(&v, i, &s);
            CHECK_EQ(ret, 0);
            CHECK_EQ(s, a[i]);
        }

        cm_del_vct(&v);
        cm_del_vct(&w);
        
    } //end test


    //test 4: set & get `map`
    SUBCASE(test_c_opt_subtests[4]) {

        mc_vm_map const * ret;

        ret = sc_opt_get_map(o);
        CHECK_EQ(ret, nullptr);

        sc_opt_set_map(o, (mc_vm_map *) 0x10203040);
        ret = sc_opt_get_map(o);
        CHECK_EQ(ret, (mc_vm_map const *) 0x10203040);
        
    } //end test


    //test 5: set & get alignment
    SUBCASE(test_c_opt_subtests[5]) {

        int ret;
        unsigned int a;

        a = sc_opt_get_alignment(o);
        CHECK_EQ(a, -1);
        CHECK_EQ(sc_errno, SC_ERR_OPT_EMPTY);

        ret = sc_opt_set_alignment(o, 4);
        CHECK_EQ(ret, 0);
        a = sc_opt_get_alignment(o);
        CHECK_EQ(a, 4);
        
    } //end test


    //test 6: get arch_byte_width
    SUBCASE(test_c_opt_subtests[6]) {

        unsigned int ret;

        ret = sc_opt_get_arch_byte_width(o);
        CHECK_EQ(ret, test_arch_byte_width);
        
    } //end test


    //test 7: set & get omit_areas
    SUBCASE(test_c_opt_subtests[7]) {

        cm_lst_node * a[3] = {
            (cm_lst_node *) 0x40404040,
            (cm_lst_node *) 0x50505050,
            (cm_lst_node *) 0x60606060
        };

        //call constraint test helper
        _c_constraint_test(o, a, sc_opt_set_omit_areas,
                           sc_opt_get_omit_areas);

    } //end test


    //test 8: set & get omit_objs
    SUBCASE(test_c_opt_subtests[8]) {

        cm_lst_node * a[3] = {
            (cm_lst_node *) 0x04040404,
            (cm_lst_node *) 0x05050505,
            (cm_lst_node *) 0x06060606
        };

        //call constraint test helper
        _c_constraint_test(o, a, sc_opt_set_omit_objs,
                           sc_opt_get_omit_objs);

    } //end test


    //test 9: set & get exclusive_areas
    SUBCASE(test_c_opt_subtests[9]) {

        cm_lst_node * a[3] = {
            (cm_lst_node *) 0x70707070,
            (cm_lst_node *) 0x80808080,
            (cm_lst_node *) 0x90909090
        };

        //call constraint test helper
        _c_constraint_test(o, a, sc_opt_set_exclusive_areas,
                           sc_opt_get_exclusive_areas);

    } //end test


    //test 10: set & get exclusive_objs
    SUBCASE(test_c_opt_subtests[10]) {

        cm_lst_node * a[3] = {
            (cm_lst_node *) 0x07070707,
            (cm_lst_node *) 0x08080808,
            (cm_lst_node *) 0x09090909
        };

        //call constraint test helper
        _c_constraint_test(o, a, sc_opt_set_exclusive_objs,
                           sc_opt_get_exclusive_objs);

    } //end test


    //test 11: set & get
    SUBCASE(test_c_opt_subtests[11]) {

        int ret;
        sc_addr_range rett = {0, 0};
        sc_addr_range ar = {0x1000, 0x2000};

        ret = sc_opt_get_addr_range(o, &rett);
        CHECK_EQ(ret, -1);
        CHECK_EQ(sc_errno, SC_ERR_OPT_EMPTY);

        ret = sc_opt_set_addr_range(o, &ar);
        CHECK_EQ(ret, 0);

        ret = sc_opt_get_addr_range(o, &rett);
        CHECK_EQ(ret, 0);
        CHECK_EQ(rett.min, ar.min);
        CHECK_EQ(rett.max, ar.max);       

    } //end test


    //test 12: set & get alignment
    SUBCASE(test_c_opt_subtests[12]) {

        int ret;
        cm_byte a;

        a = sc_opt_get_access(o);
        CHECK_EQ(a, -1);
        CHECK_EQ(sc_errno, SC_ERR_OPT_EMPTY);

        ret = sc_opt_set_alignment(o, MC_ACCESS_READ | MC_ACCESS_WRITE);
        CHECK_EQ(ret, 0);
        a = sc_opt_get_alignment(o);
        CHECK_EQ(a, MC_ACCESS_READ | MC_ACCESS_WRITE);
        
    } //end test



    //test 0 (cont.): destroy a sc_opt
    int _ret = sc_del_opt(o);
    CHECK_EQ(_ret, 0);

} //end TEST_CASE

