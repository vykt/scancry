//standard template library
#include <optional>
#include <vector>
#include <utility>

//C standard library

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <doctest/doctest.h>

//local headers
#include "filters.hh"
#include "common.hh"

//test target headers
#include "../lib/scancry.h"
#include "../lib/worker.hh"



//debug scan class
class _test_scan : public sc::_opt_scan {

    public:
        //[attributes]
        

        //[methods]
        /* internal */ [[nodiscard]] virtual _SC_DBG_INLINE int
            _process_addr(
                const struct _scan_arg arg, const sc::opt * const opts,
                const sc::_opt_scan * const opts_scan);

        /* internal */ [[nodiscard]] virtual int _generate_body(
                std::vector<cm_byte> & buf, off_t hdr_off);
        /* internal */ [[nodiscard]] virtual int _process_body(
                const std::vector<cm_byte> & buf, off_t hdr_off,
                const mc_vm_map & map);
        /* internal */ [[nodiscard]] virtual int _read_body(
                const std::vector<cm_byte> & buf, off_t hdr_off);

        [[nodiscard]] int scan(
                    sc::opt & opts,
                    sc::opt_ptrscan & opts_ptrscan,
                    sc::map_area_set & ma_set,
                    sc::worker_pool & w_pool,
                    cm_byte flags);

        [[nodiscard]] virtual int reset();
};



/*
 *  --- [TESTS - OPT] ---
 */

TEST_CASE(test_cc_opt_subtests[0]) {

    int ret;

    //test 0: construct opt classes

    //call regular constructor
    sc::opt o(test_cc_addr_width);

    /*
     *  TODO: As painful as this is, you're going to need to really 
     *        scrutinise all the threading bullshit that can occur here.
     */
}
