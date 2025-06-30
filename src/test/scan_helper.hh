#pragma once

//C standard library
#include <cstdint>

//system headers
#include <unistd.h>

//test target headers
#include "../lib/scancry.h"



namespace _scan_helper {

/*
 *  NOTE: Seeing as most core code relies on dependency injection through
 *        some derivative of the `_scan` abstract class, a fixture scan
 *        is provided. Many assertions take place here instead of directly
 *        inside of test cases.
 */

//fixture scan options class
class _fixture_opts : public sc::_opt_scan {

    public:
        //[methods]
        ~_fixture_opts() {}
        [[nodiscard]] virtual int reset() { return 0; }
};


//fixture scan class
class _fixture_scan : public sc::_scan {

    private:
        //[attributes]
        bool do_checks;
        bool do_crash_one;
        bool do_crash_all;

        cm_byte expected_byte;
        off_t read_off;
        int mod;
        long call_count;

    public:
        //[methods]
        /* internal */ [[nodiscard]] virtual _SC_DBG_INLINE off_t
            _process_addr(
                const struct sc::_scan_arg arg, const sc::opt * const opts,
                const sc::_opt_scan * const opts_scan);

        /* internal */ [[nodiscard]] virtual int _generate_body(
                std::vector<cm_byte> & buf, off_t hdr_off);
        /* internal */ [[nodiscard]] virtual int _process_body(
                const std::vector<cm_byte> & buf, off_t hdr_off,
                const mc_vm_map & map);
        /* internal */ [[nodiscard]] virtual int _read_body(
                const std::vector<cm_byte> & buf, off_t hdr_off);

        _fixture_scan() : expected_byte(0), read_off(0), mod(1) {}
        void set_mod(int mod) { this->mod = mod; }

        [[nodiscard]] int scan(
                    sc::opt & opts,
                    _fixture_opts & opts_ptrscan,
                    sc::map_area_set & ma_set,
                    sc::worker_pool & w_pool,
                    cm_byte flags);

        void set_do_checks(bool do_checks) { this->do_checks = do_checks; };
        void set_do_crash_one(bool do_crash_one) {
            this->do_crash_one = do_crash_one;
        };
        void set_do_crash_all(bool do_crash_all) {
            this->do_crash_all = do_crash_all;
        };
        long get_call_count() { return this->call_count; }
        [[nodiscard]] virtual int reset();
};

//ptrscan header fixture values
const constexpr uint32_t pathnames_num    = 0xb16b00b5;
const constexpr uint32_t pathnames_offset = 0xdead10cc;
const constexpr uint32_t chains_num       = 0xc0ffee33;
const constexpr uint32_t chains_offset    = 0xfaceb00c;

//file test data
const constexpr char * testdata = "testdata";

}
