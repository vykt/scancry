//C standard library
#include <cstdlib>
#include <cstdint>
#include <cstring>

//external libraries
#include <doctest/doctest.h>

//local headers
#include "scan_helper.hh"
#include "common.hh"

//test target headers
#include "../lib/scancry.h"


/*
 *  --- [FIXTURE SCAN CLASS | INTERNAL] ---
 */

/*
 *  NOTE: To test if workers read memory correctly, worker thread(s) are
 *        directed to read a mmap'ed `patternN.bin` file, which consists
 *        of an incrementing byte pattern for the first kilobyte, followed
 *        by a decrementing pattern for the next kilobyte.
 *
 *        TODO: Currently this only supports offsets of 1 and 4.
 */


const constexpr useconds_t _process_addr_delay = 200;
[[nodiscard]] _SC_DBG_INLINE off_t
    _scan_helper::_fixture_scan::_process_addr(
                                const struct sc::_scan_arg & arg,
                                const sc::opt & opts,
                                const sc::_opt_scan & opts_scan) {

    //crash all workers if requested
    if (this->do_crash_all) return -1;

    //crash one worker if requested
    bool crash_one_expected = true;
    if (this->do_crash_one.compare_exchange_strong(
            crash_one_expected, false) == true) return -1;

    //introduce delay if requested
    if (this->do_delay == true) usleep(_process_addr_delay);

    //increment call count
    this->call_count += 1;

    //if checks are enabled
    if (do_checks == true) {

        //check byte pattern is correct
        CHECK_EQ(this->expected_byte, *arg.get_cur_byte());

        //advance state
        this->read_off += std::abs(this->mod);

        //apply modifier for each scanned byte
        if (this->read_off < 0x1000) {
            this->expected_byte += this->mod;
        } else {
            //re-align if mod = 4
            if (std::abs(this->mod) == 4) {
                if (this->mod == 4) this->expected_byte = 0xff;
                if (this->mod == -4) this->expected_byte = 0x0;
            }
            this->mod *= -1;
            read_off = 0x0;
        }
    
    } //end if checks are enabled

    //return buffer advance
    return std::abs(this->mod);
}



/*
 *  --- [FIXTURE SCAN CLASS | PUBLIC] ---
 */

//ctor
_scan_helper::_fixture_scan::_fixture_scan() noexcept
 : _scan(),
   mod(1),
   do_checks(true),
   do_crash_one(false),
   do_crash_all(false),
   expected_byte(0),
   read_off(0),
   call_count(0) {}


//behaviour modifiers
void _scan_helper::_fixture_scan::set_mod(const int mod) noexcept {

    this->mod = mod;
    return;
}


void _scan_helper::_fixture_scan::set_do_checks(
    const bool do_checks) noexcept {

    this->do_checks = do_checks;
    return;
};


void _scan_helper::_fixture_scan::set_do_delay(
    const bool do_delay) noexcept {

    this->do_delay = do_delay;
    return;
}


void _scan_helper::_fixture_scan::set_do_crash_one(
    const bool do_crash_one) noexcept {
    
    this->do_crash_one.store(do_crash_one, std::memory_order_relaxed);
    return;
};


void _scan_helper::_fixture_scan::set_do_crash_all(
    const bool do_crash_all) noexcept {

    this->do_crash_all = do_crash_all;
    return;
};


//statistics getters
[[nodiscard]] long
    _scan_helper::_fixture_scan::get_call_count() const noexcept {

    return this->call_count;
}


//interface
/* _fixture_scan::scan() - <no_impl> */


[[nodiscard]] int _scan_helper::_fixture_scan::reset() noexcept {

    this->do_checks     = false;
    this->do_delay      = false;
    this->do_crash_one  = false;
    this->do_crash_all  = false;
    this->expected_byte = 0;
    this->read_off      = 0;
    this->mod           = 1;
    this->call_count    = 0;

    return 0;
}
