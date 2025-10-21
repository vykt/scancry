#pragma once

//standard template libraru
#include <atomic>

//C standard library
#include <cstdint>

//system headers
#include <unistd.h>

//test target headers
#include "../lib/scancry.h"



namespace _scan_helper {

//hexdump a buffer
void hexdump(cm_byte * buf, size_t sz);

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
        // -- [attributes]
        //behaviour modifiers
        int mod;
        bool do_checks;
        bool do_delay;
        std::atomic<bool> do_crash_one;
        bool do_crash_all;

        //inter-call state
        cm_byte expected_byte;
        off_t read_off;

        //statistics
        long call_count;

    public:
        // -- [methods]        
        /* internal */ [[nodiscard]] virtual _SC_DBG_INLINE off_t
            _process_addr(
                const struct sc::_scan_arg & arg,
                const sc::opt & opts,
                const sc::_opt_scan & opts_fxt) noexcept override final;

        //ctor & dtor
        _fixture_scan() noexcept;
        _fixture_scan(const
            _scan_helper::_fixture_scan & fix_scan) noexcept = delete;
        _fixture_scan(const
            _scan_helper::_fixture_scan && fix_scan) noexcept = delete;
        ~_fixture_scan() noexcept {};

        //operators
        _scan_helper::_fixture_scan & operator=(
            const _scan_helper::_fixture_scan & scan_fxt) = delete;
        _scan_helper::_fixture_scan & operator=(
            const _scan_helper::_fixture_scan && scan_fxt) = delete;

        //behaviour modifiers
        void set_mod(const int mod) noexcept;
        void set_do_checks(const bool do_checks) noexcept;
        void set_do_delay(const bool do_delay) noexcept;
        void set_do_crash_one(const bool do_crash_one) noexcept;
        void set_do_crash_all(const bool do_crash_all) noexcept;

        //statistics getters
        [[nodiscard]] long get_call_count() const noexcept;


        //interface
        [[nodiscard]] int scan(
                    sc::opt & opts,
                    _fixture_opts & opts_fxt,
                    sc::map_area_set & ma_set,
                    sc::worker_pool & w_pool,
                    cm_byte flags);
        [[nodiscard]] virtual int reset() noexcept;
};

//ptrscan header fixture values
const constexpr uint32_t pathnames_num    = 0xb16b00b5;
const constexpr uint32_t pathnames_offset = 0xdead10cc;
const constexpr uint32_t chains_num       = 0xc0ffee33;
const constexpr uint32_t chains_offset    = 0xfaceb00c;

//file test data
const constexpr char * testdata = "testdata";
const constexpr size_t testdata_sz = 8;

}
