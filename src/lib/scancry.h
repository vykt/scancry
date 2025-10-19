#ifndef SCANCRY_H
#define SCANCRY_H

//C standard library
#include <stddef.h>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry_impl.h"



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

#ifdef __cplusplus
namespace sc {

/*
 *  NOTE: For all classes inheriting the `_ctor_failable` parent,
 *        call `get_ctor_failed()` after construction to determine
 *        if the constructor succeeded.
 */


/*
 *  --- [CLASSES] ---
 */

/*
 *  NOTE: This is a single address range, used when specifying address
 *        range based constraints.
 */

class addr_range {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        uintptr_t start_addr;
        uintptr_t end_addr;

    public:
        // -- [methods]
        //ctor
        addr_range(uintptr_t start_addr, uintptr_t end_addr)
         : start_addr(start_addr), end_addr(end_addr) {}

        //getters
        [[nodiscard]] uintptr_t get_start_addr() const noexcept;
        [[nodiscard]] uintptr_t get_end_addr() const noexcept;
};


/*
 *  NOTE: Constraints included in this class are used to populate a
 *        `map_area_set`. Instances of `map_area_set` represent some
 *        subset of a MemCry target map.
 */

//unset values
namespace val_unset {
    const constexpr cm_byte access = CM_BYTE_MAX - 1;
}

//bad values
namespace val_bad {
    const constexpr cm_byte access = CM_BYTE_MAX;
}

class opt_map_area : public _lockable, public _ctor_failable {

    _SC_DBG_PRIVATE:    
        // -- [attributes]
        cm_vct /* <const cm_lst_node *> */ omit_areas;
        cm_vct /* <const cm_lst_node *> */ omit_objs;
        cm_vct /* <const cm_lst_node *> */ exclusive_areas;
        cm_vct /* <const cm_lst_node *> */ exclusive_objs;
        cm_vct /* <sc::addr_range> */ omit_addr_ranges;
        cm_vct /* <sc::addr_range> */ exclusive_addr_ranges;
        cm_byte access;

        // -- [methods]
        void do_copy(const sc::opt_map_area & opts_ma) noexcept;

    public:
        // -- [methods]
        //ctors & dtor
        opt_map_area() noexcept;
        opt_map_area(const sc::opt_map_area & opts_ma) noexcept;
        opt_map_area(const sc::opt_map_area && opts_ma) = delete;
        ~opt_map_area() noexcept;

        //operators
        sc::opt_map_area & operator=(
            const sc::opt_map_area & opt_ma) noexcept;
        sc::opt_map_area & operator=(
            const sc::opt_map_area && opt_ma) = delete;

        //reset
        [[nodiscard]] int reset() noexcept;

        //getters & setters
        [[nodiscard]] int set_omit_areas(
            const cm_vct /* <const cm_lst_node *> */ & omit_areas) noexcept;
        [[nodiscard]] const cm_vct /* <const cm_lst_node *> */ &
            get_omit_areas() const noexcept;

        [[nodiscard]] int set_omit_objs(
            const cm_vct /* <const cm_lst_node *> */ & omit_objs) noexcept;
        [[nodiscard]] const cm_vct /* <const cm_lst_node *> */ &
            get_omit_objs() const noexcept;

        [[nodiscard]] int set_exclusive_areas(
            const cm_vct /* <const cm_lst_node *> */ & exclusive_areas) noexcept;
        [[nodiscard]] const cm_vct /* <const cm_lst_node *> */ &
            get_exclusive_areas() const noexcept;

        [[nodiscard]] int set_exclusive_objs(
            const cm_vct /* <const cm_lst_node *> */ & exclusive_objs) noexcept;
        [[nodiscard]] const cm_vct /* <const cm_lst_node *> */ &
            get_exclusive_objs() const noexcept;

        [[nodiscard]] int set_omit_addr_ranges(
            const cm_vct /* <sc::addr_range> */ & addr_ranges) noexcept;
        [[nodiscard]] const cm_vct /* <const cm_lst_node *> */ &
            get_omit_addr_ranges() const noexcept;

        [[nodiscard]] int set_exclusive_addr_ranges(
            const cm_vct /* <sc::addr_range> */ & addr_ranges) noexcept;
        [[nodiscard]] const cm_vct /* <const cm_lst_node *> */ &
            get_exclusive_addr_ranges() const noexcept;

        [[nodiscard]] int set_access(const cm_byte access) noexcept;
        [[nodiscard]] cm_byte get_access() const noexcept;
};


/*
 *  NOTE: This class represents a subset of a MemCry target map. They
 *        are used to determine which memory areas to scan, which
 *        memory areas to treat as `static` during pointer scans, and
 *        more.
 */

class map_area_set : public _lockable, public _ctor_failable {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        cm_rbt /* <const cm_lst_node * : const mc_area *> */ set;

        //[methods]
        void do_copy(const sc::map_area_set & ma_set) noexcept;

    public:
        // -- [methods]
        //ctors & dtor
        map_area_set() noexcept;
        map_area_set(const sc::map_area_set & ma_set) noexcept;
        map_area_set(const sc::map_area_set && ma_set) = delete;
        ~map_area_set() noexcept;

        //operators
        sc::map_area_set & operator=(
            const sc::map_area_set & ma_set) noexcept;
        sc::map_area_set & operator=(
            const sc::map_area_set && ma_set) = delete;

        //reset
        [[nodiscard]] int reset() noexcept;

        //populate
        [[nodiscard]] int update_set(const sc::opt_map_area & opts_ma,
                                     const mc_vm_map & map) noexcept;

        //convert the set to an address ordered vector of areas
        [[nodiscard]] int to_addr_ord_vct(
            cm_vct /* <const cm_lst_node *> */ & vct) const noexcept;
        //convert the set to a size ordered vector of areas
        [[nodiscard]] int to_size_ord_vct(
            cm_vct /* <const cm_lst_node *> */ & vct) const noexcept;

        //getters
        [[nodiscard]] const cm_rbt /* <const cm_lst_node *> */ &
            get_set() const noexcept;
};


//architecture address width enum
enum addr_width /* parity with sc_addr_width */ {
    AW32  = 4,
    AW64  = 8,
    ADDR_WIDTH_UNSET = -1,
};


/*
 *  NOTE: This class defines generic configuration options relevant to
 *        all scan types.
 */

//unset values
namespace val_unset {
    const constexpr enum addr_width addr_width = sc::ADDR_WIDTH_UNSET;
}

//bad values
namespace val_bad {

    _Pragma("GCC diagnostic push")
    _Pragma("GCC diagnostic ignored \"-Wunused-variable\"")
    static inline mc_vm_map * map = (mc_vm_map *) UINTPTR_MAX;
    _Pragma("GCC diagnostic pop")
}

class opt : public _lockable, public _ctor_failable {

    _SC_DBG_PRIVATE:
        // -- [attributes]

        /*
         *  NOTE: The number of threads used during scans is determined 
         *        by the number of provided sessions.
         */

        //save & load file paths
        char * /* alloc */ file_pathname_out;
        char * /* alloc */ file_pathname_in;

        //sessions & map
        cm_vct /* <const mc_session *> */ sessions;
        mc_vm_map * map;

        //address width (32bit / 64bit)
        enum addr_width addr_width;

        //set of areas to scan
        const sc::map_area_set * scan_set;

        //[methods]
        void do_copy(const sc::opt & opts) noexcept;

    public:
        // -- [methods]
        //ctors & dtor
        opt() noexcept;
        opt(const sc::opt & opts) noexcept;
        opt(const sc::opt && opts) = delete;
        ~opt() noexcept;

        //operators
        sc::opt & operator=(const sc::opt & opts) noexcept;
        sc::opt & operator=(const sc::opt && opts) = delete;

        //reset
        [[nodiscard]] int reset() noexcept;

        //getters & setters
        [[nodiscard]] int set_file_pathname_out(
            const char * file_pathname_out) noexcept;
        [[nodiscard]] const char * const &
            get_file_pathname_out() const noexcept;

        [[nodiscard]] int set_file_pathname_in(
            const char * file_pathname_in) noexcept;
        [[nodiscard]] const char * const &
            get_file_pathname_in() const noexcept;

        [[nodiscard]] int set_sessions(
            const cm_vct /* <const mc_session *> */ & sessions) noexcept;
        [[nodiscard]] const cm_vct /* <const mc_session *> */ &
            get_sessions() const noexcept;

        [[nodiscard]] int set_map(const mc_vm_map * map) noexcept;
        [[nodiscard]] mc_vm_map * get_map() const noexcept;

        [[nodiscard]] int set_addr_width(
            const enum addr_width addr_width) noexcept;
        [[nodiscard]] int get_addr_width(
            enum sc::addr_width & addr_width) const noexcept;

        [[nodiscard]] int set_scan_set(
            const sc::map_area_set * scan_set) noexcept;
        [[nodiscard]] const sc::map_area_set *
            get_scan_set() const noexcept;
};



/*
 *  NOTE: This class defines configuration options only applicable to
 *        pointer scans.
 */

enum smart_scan /* parity with sc_smart_scan */ {
    SMART_SCAN_ENABLED = 0,
    SMART_SCAN_DISABLED = 1,
};

//unset values
namespace val_unset {
    const constexpr uintptr_t       target_addr = 0x0;
}

//default values
namespace val_default {
    const constexpr off_t           alignment  = 0x4;
    const constexpr off_t           max_obj_sz = 0x100;
    const constexpr int             max_depth  = 3;
    const constexpr enum smart_scan smart_scan = sc::SMART_SCAN_ENABLED;
}

//bad values
namespace val_bad {
    const constexpr uintptr_t       target_addr = UINTPTR_MAX;
    const constexpr off_t           alignment   = -1;
    const constexpr off_t           max_obj_sz  = -1;
    const constexpr int             max_depth   = -1;
}

class opt_ptrscan final : public _opt_scan {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        //address to scan for
        uintptr_t target_addr;

        //pointer alignment in bytes
        off_t alignment;

        //maximum structure size to accept
        off_t max_obj_sz;

        //number of iterations to perform (time grows exponentially)
        int max_depth;

        //areas to treat as terminal nodes (areas holding static globals)
        const sc::map_area_set * static_set;

        //first N offsets
        cm_vct /* <off_t> */ preset_offsets;

        /*
         *  NOTE: With `smart_scan` enabled, every potential pointer
         *        will be treated as pointing only to nodes to which it's
         *        offset is smallest.
         *
         *        For example: imagine 4 objects of size 0x40 each are
         *        allocated inside an array. With `max_obj_sz` of 0x100,
         *        a potential pointer to the start of the array will
         *        technically point to each object in this array.
         *
         *        Enabling a smart scan will cause the potential pointer
         *        to point only to the first object with the offset of
         *        0x40. This will greatly reduce the amount of false
         *        positives, but may miss some valid results.
         */

        //perform a smart pointer scan
        enum smart_scan smart_scan;

        // -- [methods]
        void do_copy(const sc::opt_ptrscan & opts_ptr) noexcept;

    public:
        // -- [methods]
        //ctors & dtor
        opt_ptrscan() noexcept;
        opt_ptrscan(const sc::opt_ptrscan & opts_ptr) noexcept;
        opt_ptrscan(const sc::opt_ptrscan && opts_ptr) = delete;
        ~opt_ptrscan() noexcept override final;

        //operators
        sc::opt_ptrscan & operator=(
            const sc::opt_ptrscan & opts_ptr) noexcept;
        sc::opt_ptrscan & operator=(
            const sc::opt_ptrscan && opts_ptr) = delete;

        //reset
        [[nodiscard]] int reset() noexcept override final;

        //getters & setters
        [[nodiscard]] int set_target_addr(
            const uintptr_t target_addr) noexcept;
        [[nodiscard]] uintptr_t get_target_addr() const noexcept;

        [[nodiscard]] int set_alignment(const off_t alignment) noexcept;
        [[nodiscard]] off_t get_alignment() const noexcept;

        [[nodiscard]] int set_max_obj_sz(const off_t max_obj_sz) noexcept;
        [[nodiscard]] off_t get_max_obj_sz() const noexcept;

        [[nodiscard]] int set_max_depth(const int max_depth) noexcept;
        [[nodiscard]] int get_max_depth() const noexcept;

        [[nodiscard]] int set_static_set(
            const sc::map_area_set * static_set) noexcept;
        [[nodiscard]] const sc::map_area_set *
            get_static_set() const noexcept;

        [[nodiscard]] int set_preset_offsets(
            const cm_vct /* <off_t> */ & preset_offsets) noexcept;
        [[nodiscard]] const cm_vct /* <off_t> */ &
            get_preset_offsets() const noexcept;

        [[nodiscard]] int set_smart_scan(
            const enum smart_scan smart_scan) noexcept;
        [[nodiscard]] int
            get_smart_scan(enum smart_scan & smart_scan) const noexcept;
};


/*
 *  NOTE: This is a manager of worker threads, responsible for spawning,
 *        dispatching, synchronising, and cleaning up threads. The
 *        number of spawned threads is determined by the number of 
 *        sessions provided in `sc::opt.sessions`.
 */

//worker_pool::update_workers() behavioural flags
namespace bits_worker {
    const constexpr cm_byte keep_scan_set = 0b1 << 0;
}

class worker_pool
    : public _lockable, public _ctor_failable, public _stateful {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        //workers
        cm_lst /* <_worker_bundle> */ wkr_bundles;
        int wkr_next_uid;

        //local copy of the last provided scan set, sorted by area size
        cm_vct /* <cm_lst_node *> */ sorted_areas_cache;

        //cache & concurrency
        _worker_pool_cache cache;
        _worker_concurrency concur;


        //[methods]
        [[nodiscard]] int do_run() noexcept;
        [[nodiscard]] int do_await(
            const bool do_block, const bool do_timeout) noexcept;

        [[nodiscard]] int do_ctrl_run() noexcept;
        [[nodiscard]] int do_scan_run() noexcept;

        void remove_wkr_bundles(const int count) noexcept;
        void cleanup_err() noexcept;
        [[nodiscard]] int change_wkr_count(const int count) noexcept;
        [[nodiscard]] int cache_areas(
            const sc::map_area_set & ma_set) noexcept;
        [[nodiscard]] int distrib_areas() noexcept;

    public:
        // -- [methods]
        /* internal */ [[nodiscard]] int _setup(
            const sc::opt & opts,
            const sc::_opt_scan & opts_scan,
            sc::_scan & scan,
            const cm_byte flags) noexcept;
        /* internal */ [[nodiscard]] int _teardown() noexcept;
        

        //perform a single pass over the scan set
        /* internal */ [[nodiscard]] int _dispatch_run() noexcept;
        /* internal */ [[nodiscard]] int
            _await_run(const bool do_block) noexcept;
        /* internal */ void _cancel() noexcept;

        //ctor & dtor
        worker_pool() noexcept;
        worker_pool(const sc::worker_pool & w_pool) = delete;
        worker_pool(const sc::worker_pool && w_pool) = delete;
        ~worker_pool() noexcept;

        //operators
        sc::worker_pool & operator=(
            const sc::worker_pool & w_pool) = delete;
        sc::worker_pool & operator=(
            const sc::worker_pool && w_pool) = delete;

        //control workers
        [[nodiscard]] int reset() noexcept;
};


/*
 *  NOTE: The `file` sub-namespace contains types & functions for
 *        processing scancry files (.scf). For scan-specific processing,
 *        use the appropriate scan class.
 */

namespace file {

    //file version
    enum ver : uint16_t /* parity with sc_file_ver */ {
        VER_0_1 = 0x0001
    };

    //scan type
    enum scan_type : cm_byte /* parity with sc_file_scan_type */ {
        PTRSCAN_TYPE = 0x01,
        PTNSCAN_TYPE = 0x02,
        TBLSCAN_TYPE = 0x03
    };

    //current version
    const constexpr enum sc::file::ver cur_ver = VER_0_1;

    //metadata struct
    struct metadata /* parity with sc_file_metadata */ {
        enum sc::file::ver ver;
        enum sc::file::scan_type type;
    };

    //get metadata about a file
    [[nodiscard]] int get_metadata(
        const sc::opt & opts, sc::file::metadata & mdata) noexcept;

} //end namespace `file`



/*
 *  NOTE: This stores a container of object pathnames.
 */

//table of object pathnames
class obj_table : public _ctor_failable {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        cm_vct /* <const char * (alloc)> */ pathname_tbl;

        // -- [methods]
        //destroy the pathname table
        void del_pathname_tbl() noexcept;

        //perform the copy
        void do_copy(const sc::obj_table & obj_tbl) noexcept;

        //insert a pathname into the table
        [[nodiscard]] int do_add(const char * pathname) noexcept;

    public:
        // -- [methods]
        /* internal */ [[nodiscard]] int serialise(
            FILE * fs) const noexcept;
        /* internal */ [[nodiscard]] int deserialise(
            FILE * fs, const uint32_t pathname_num) noexcept;

        //ctors & dtor
        obj_table() noexcept;
        obj_table(const sc::obj_table & obj_tbl) noexcept;
        obj_table(const sc::obj_table && obj_tbl) = delete;
        ~obj_table() noexcept;

        //operators
        sc::obj_table & operator=(
            const sc::obj_table & obj_tbl) noexcept;
        sc::obj_table & operator=(
            const sc::obj_table && obj_tbl) = delete;

        //resetter
        void reset() noexcept;

        //resolvers
        [[nodiscard]] const char * resolv_pathname(
            const int idx) const noexcept;
        [[nodiscard]] const cm_lst_node * resolv_obj_node(
            const int idx, const mc_vm_map * map) const noexcept;

        //add a pathname (skip search)
        [[nodiscard]] int fadd_pathname(const char * pathname) noexcept;

        //add a pathname
        [[nodiscard]] int add_pathname(const char * pathname) noexcept;

        //getters
        [[nodiscard]] int get_sz() const noexcept;
        [[nodiscard]] const cm_vct /* <const char * (alloc)> */ &
            get_pathname_tbl() const noexcept;
};



/*
 *  NOTE: These classes implement a pointer scanner. Internally, the
 *        pointer scanner builds a tree where the root node is the
 *        target address, and leaf nodes are starting points for
 *        the scan. 
 */

//a pointer chain node
class ptr_chain_node {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        //universal data
        off_t off;
        int obj_tbl_idx;
        off_t obj_tbl_off;

        //live data
        uintptr_t addr;
        const cm_lst_node * obj_node;
        const cm_lst_node * area_node;
        off_t obj_off;
        off_t area_off;

        // -- [methods]
        //perform a copy
        void do_copy(const sc::ptr_chain_node & p_chain_node) noexcept;

    public:
        // -- [methods]
        //verify this node
        /* internal */ [[nodiscard]] int _follow(
            const mc_vm_map * map,
            mc_session * sess,
            uintptr_t & cur_addr) noexcept;

        //update live data
        /* internal */ [[nodiscard]] int _update_live_data(
            const mc_vm_map * map,
            mc_session * sess,
            uintptr_t & cur_addr) noexcept;

        //ctors & dtor
        ptr_chain_node(
            const off_t off,
            const int obj_tbl_idx,
            const off_t obj_tbl_off) noexcept;
        ptr_chain_node(const sc::ptr_chain_node & p_chain_node) noexcept;
        ptr_chain_node(const sc::ptr_chain_node && p_chain_node) = delete;
        ~ptr_chain_node() noexcept;

        //operators
        sc::ptr_chain_node & operator=(
            const sc::ptr_chain_node & p_chain_node) noexcept;
        sc::ptr_chain_node & operator=(
            const sc::ptr_chain_node && p_chain_node) = delete;

        //getters - shallow analysis
        [[nodiscard]] off_t get_off() const noexcept;
        [[nodiscard]] int get_obj_tbl_idx() const noexcept;

        //getters - deep analysis
        [[nodiscard]] uintptr_t get_addr() const noexcept;
        [[nodiscard]] const cm_lst_node * get_obj_node() const noexcept;
        [[nodiscard]] const cm_lst_node * get_area_node() const noexcept;
        [[nodiscard]] off_t get_obj_off() const noexcept;
        [[nodiscard]] off_t get_area_off() const noexcept;
};


//a pointer chain
class ptr_chain : public _ctor_failable {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        bool is_static;
        cm_vct /* <sc::ptr_chain_node> */ nodes;

        // -- [methods]
        //perform a copy
        void do_copy(const sc::ptr_chain & p_chain) noexcept;

        //resolve the starting address of this chain
        [[nodiscard]] uintptr_t resolv_start_addr(
            const mc_vm_map * map,
            const sc::obj_table & obj_tbl) noexcept;

    public:
        // -- [methods]
        //ctors & dtor
        ptr_chain(
            const cm_vct /* <off_t> */ & off_vct,
            const cm_vct /* <int> */ & obj_tbl_idx_vct,
            const cm_vct /* <off_t> */ & obj_tbl_off_vct,
            const bool is_static) noexcept;
        ptr_chain(const sc::ptr_chain & p_chain) noexcept;
        ptr_chain(const sc::ptr_chain && p_chain) = delete;
        ~ptr_chain() noexcept;

        //operators
        sc::ptr_chain & operator=(
            const sc::ptr_chain & p_chain) noexcept;
        sc::ptr_chain_node & operator=(
            const sc::ptr_chain && p_chain) = delete;

        //follow & verify
        [[nodiscard]] int verify(
            const mc_vm_map * map,
            mc_session * sess,
            const sc::obj_table & obj_tbl,
            const uintptr_t tgt_addr) noexcept;

        //follow & populate live data
        [[nodiscard]] int update_live_data(
            const mc_vm_map * map,
            mc_session * sess,
            const sc::obj_table & obj_tbl) noexcept;

        //getters
        [[nodiscard]] bool get_static() const noexcept;

        const cm_vct /* <sc::ptr_chain_node> */ &
            get_nodes() const noexcept;
};


//pointer chain scanner
class ptrscan : public _scan {

    /*
     *  TODO: Record in the savefile the parameters used to produce
     *        the results.
     */

    _SC_DBG_PRIVATE:
        // -- [attributes]
        //pointer scan tree
        sc::_ptr_tree tree;

        //flattened tree chains
        sc::obj_table obj_tbl;
        cm_vct /* <sc::ptr_chain> */ chains;

        //depth level
        int depth_lvl;
        cm_vct /* <sc::_ptr_tree_node *> */ * depth_lvl_vct_p;

        //high-level state
        cm_byte state_flags;

        // -- [methods]
        [[nodiscard]] int setup_fn(
            const sc::opt * opts,
            const sc::opt_ptrscan * opts_ptr) noexcept;


        [[nodiscard]] int do_reset() noexcept;
        
        [[nodiscard]] int do_await_scan(
            sc::worker_pool & w_pool,
            const bool do_block) noexcept;

        [[nodiscard]] int chain_recurse(
            const mc_vm_map * map,
            const cm_rbt & static_set_tree,
            cm_vct /* <off_t> */ & off_stack,
            cm_vct /* <int> */ & obj_tbl_idx_stack,
            cm_vct /* <off_t> */ & obj_tbl_off_stack,
            const sc::_ptr_tree_node & p_tree_node,
            const uintptr_t tgt_addr) noexcept;

        [[nodiscard]] int wr_chains(FILE * fs) const noexcept; 
        [[nodiscard]] int rd_chains(FILE * fs) const noexcept; 

    public:
        // -- [methods]
        /* internal */ [[nodiscard]] virtual off_t
            _process_addr(
                const sc::_scan_arg & arg,
                const opt & opts,
                const _opt_scan & opts_scan) noexcept override final;

        //ctors & dtor
        ptrscan() noexcept;
        ptrscan(const sc::ptrscan & pscan) = delete;
        ptrscan(const sc::ptrscan && pscan) = delete;
        ~ptrscan() noexcept;

        //operators
        sc::ptrscan & operator=(const sc::ptrscan & pscan) = delete;
        sc::ptrscan & operator=(const sc::ptrscan && pscan) = delete;

        //reset
        [[nodiscard]] int reset() noexcept override final;

        // - perform scans

        //dispatch a scan (+1 depth)
        [[nodiscard]] int dispatch_scan(
            const sc::opt & opts,
            const sc::opt_ptrscan & opts_ptr,
            sc::worker_pool & w_pool,
            const cm_byte w_pool_flags) noexcept;

        //await a scan
        //0 = success, -1 = error, -2 = workers terminated prematurely
        [[nodiscard]] int await_scan(
            sc::worker_pool & w_pool) noexcept;
        [[nodiscard]] int try_await_scan(
            sc::worker_pool & w_pool) noexcept;

        // - tree operations

        //flatten pointer tree into chains
        [[nodiscard]] int flatten_tree(
            const sc::opt & opts,
            const sc::opt_ptrscan & opts_ptr) noexcept;

        // - chain operations

        //verify existing chains
        [[nodiscard]] int verify_chains(
            const sc::opt & opts,
            const sc::opt_ptrscan & opts_ptr,
            const uintptr_t tgt_addr) noexcept;

        //update live data in existing chains
        [[nodiscard]] int update_live_chains(
            const sc::opt & opts,
            const sc::opt_ptrscan & opts_ptr,
            const uintptr_t tgt_addr) noexcept;

        // - exporting data

        //get references to chains & object table 
        const cm_vct /* <sc::ptr_chain> */ & get_chains() const noexcept;
        const sc::obj_table & get_obj_tbl() const noexcept;

        //export a copy of the chains
        [[nodiscard]] int export_chains(
            cm_vct /* <sc::ptr_chain> */ & chains) const noexcept;

        //export a copy of the object table
        [[nodiscard]] int export_obj_tbl(
            sc::obj_table & obj_tbl) const noexcept;

        // - serialisation

        //save & load pointer chains
        [[nodiscard]] int serialise(const sc::opt & opts) const noexcept;
        [[nodiscard]] int deserialise(const sc::opt & opts) const noexcept;

};


}; //end namespace `sc`
#endif //#ifdef __cplusplus



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

/*
 *  The C interface will convert all exceptions into sc_errno.
 */

#ifdef __cplusplus
extern "C" {
#endif


/*
 *  --- [DATA TYPES] ---
 */

/*
 *  NOTE: C code should treat these as opaque handles. Using incomplete
 *        types is preferable to using void pointers, as reassignment
 *        across types is still treated as a warning/error
 */

// -- map area options & set

//address range
typedef struct {

    uintptr_t start_addr;
    uintptr_t end_addr;

} sc_addr_range;

//unset values
#define SC_ACCESS_UNSET CM_BYTE_MAX - 1

//bad values
#define SC_ACCESS_BAD   CM_BYTE_MAX

typedef struct sc_opt_map_area sc_opt_map_area;
typedef struct sc_map_area_set sc_map_area_set;


// -- generic options

//architecture address width enum
enum sc_addr_width /* parity with sc::addr_width */ {
    SC_AW32 = 4,
    SC_AW64 = 8,
    SC_ADDR_WIDTH_UNSET = -1,
};

typedef struct sc_opt sc_opt;


// -- scan options

enum sc_smart_scan /* parity with sc::smart_scan */ {
    SC_SMART_SCAN_ENABLED = 0,
    SC_SMART_SCAN_DISABLED = 1,
};

//unset values
#define SC_ADDR_WIDTH_UNSET SC_ADDR_WIDTH_UNSET

//default values
#define SC_ALIGNMENT_DEFAULT  0x4
#define SC_MAX_OBJ_SZ_DEFAULT 0x100
#define SC_MAX_DEPTH_DEFAULT  3
#define SC_SMART_SCAN_DEFAULT SC_SMART_SCAN_ENABLED

//bad values
#define SC_TARGET_ADDR_BAD UINTPTR_MAX
#define SC_ALIGNMENT_BAD   -1
#define SC_MAX_OBJ_SZ_BAD  -1
#define SC_MAX_DEPTH_BAD   -1

typedef /* base */ struct sc_opt_scan sc_opt_scan;
typedef struct sc_opt_ptrscan sc_opt_ptrscan;


// -- scan types

typedef /* base */ struct sc_scan sc_scan;
typedef struct sc_ptrscan sc_ptrscan;


// -- worker pool

typedef struct sc_worker_pool sc_worker_pool;


// -- file

//file version
enum sc_file_ver : uint16_t /* parity with sc::file::ver */ {
    SC_FILE_VER_0_1 = 0x0001
};

//scan type
enum sc_file_scan_type : cm_byte /* parity with sc::file::scan_type */ {
    SC_FILE_PTRSCAN_TYPE = 0x01,
    SC_FILE_PTNSCAN_TYPE = 0x02,
    SC_FILE_TBLSCAN_TYPE = 0x03
};

//current version
#define SC_FILE_CUR_VER SC_FILE_VER_0_1

//metadata struct
struct sc_file_metadata /* parity with sc::file::metadata */ {
    enum sc_file_ver ver;
    enum sc_file_scan_type type;
};


/*
 *  NOTE: For all functions, see `sc_errno` on error.
 */

/*
 *  --- [OPT_MAP_AREA] ---
 */

//opaque handle = success, NULL = error 
extern sc_opt_map_area * sc_new_opt_ma();
extern sc_opt_map_area * sc_copy_opt_ma(const sc_opt_map_area * opts_ma);
//0 = success, -1 = error
extern int sc_copy_assign_opt_ma(const sc_opt_map_area * dst_opts_ma,
                                 const sc_opt_map_area * src_opts_ma);
//void return
extern void sc_del_opt_ma(sc_opt_map_area * opts_ma);
//0 = success, -1 = error
extern int sc_opt_ma_reset(sc_opt_map_area * opts_ma);

//setters: 0 = success, -1 = error
//getters: pointer = success, NULL = error

//omit areas
extern int sc_opt_ma_set_omit_areas(
    sc_opt_map_area * opts_ma, const cm_vct * omit_areas);
extern const cm_vct * sc_opt_ma_get_omit_areas(const sc_opt_map_area * opts_ma);

//omit objects
extern int sc_opt_ma_set_omit_objs(
    sc_opt_map_area * opts_ma, const cm_vct * omit_objs);
extern const cm_vct * sc_opt_ma_get_omit_objs(
    const sc_opt_map_area * opts_ma);

//exclusive areas
extern int sc_opt_ma_set_exclusive_areas(
    sc_opt_map_area * opts_ma, const cm_vct * exclusive_areas);
extern const cm_vct * sc_opt_ma_get_exclusive_areas(
    const sc_opt_map_area * opts_ma);

//exclusive objects
extern int sc_opt_ma_set_exclusive_objs(
    sc_opt_map_area * opts_ma, const cm_vct * exclusive_objs);
extern const cm_vct * sc_opt_ma_get_exclusive_objs(
    const sc_opt_map_area * opts_ma);

//omit address ranges
extern int sc_opt_ma_set_omit_addr_ranges(
    sc_opt_map_area * opts_ma, const cm_vct * omit_addr_ranges);
//only for this getter: 0 = success, -1 = fail, deallocate vector manually
extern int sc_opt_ma_get_omit_addr_ranges(
    const sc_opt_map_area * opts_ma, cm_vct * omit_addr_ranges);

//exclusive address ranges
extern int sc_opt_ma_set_exclusive_addr_ranges(
    sc_opt_map_area * opts_ma, const cm_vct * exclusive_addr_ranges);
extern int sc_opt_ma_get_exclusive_addr_ranges(
    const sc_opt_map_area * opts_ma, cm_vct * omit_addr_ranges);

//access
//0 = success, CM_BYTE_MAX = error
extern int sc_opt_ma_set_access(
    sc_opt_map_area * opts_ma, const cm_byte access);
//CM_BYTE_MAX = error, SC_ACCESS_UNSET = not set, other = success
extern cm_byte sc_opt_ma_get_access(const sc_opt_map_area * opts_ma);


/*
 *  --- [MAP_AREA_SET] ---
 */

//opaque handle = success, NULL = error
extern sc_map_area_set * sc_new_ma_set();
extern sc_map_area_set * sc_copy_ma_set(const sc_map_area_set * ma_set);
//0 = success, -1 = error
extern int sc_copy_assign_ma_set(const sc_map_area_set * dst_ma_set,
                                 const sc_map_area_set * src_ma_set);
//void return
extern void sc_del_ma_set(sc_map_area_set * ma_set);
//0 = success, -1 = error
extern int sc_ma_set_reset(sc_map_area_set * ma_set);

//0 = success, -1 = error
extern int sc_ma_set_update_set(sc_map_area_set * ma_set,
                                const sc_opt_map_area * opts_ma,
                                const mc_vm_map * map);
extern int sc_ma_set_build_sorted_vct(const sc_map_area_set * ma_set,
                                      cm_vct * sorted_vct);
//pointer = success, -1 = error
extern const cm_rbt * sc_get_set(const sc_map_area_set * ma_set);


/*
 *  --- [OPT] ---
 */

//opaque handle = success, NULL = error
extern sc_opt * sc_new_opt();
extern sc_opt * sc_copy_opt(const sc_opt * opts);
//0 = success, -1 = error
extern int sc_copy_assign_opt(const sc_opt * dst_opts,
                              const sc_opt * src_opts);
//void return
extern void sc_del_opt(sc_opt * opts);
//0 = success, -1 = error
extern int sc_opt_reset(sc_opt * opts);

//0 = success, -1 = error
extern int sc_opt_set_file_pathname_out(sc_opt * opts, const char * path);
//pointer to a private string (can't fail)
extern const char * const * sc_opt_get_file_pathname_out(
    const sc_opt * opts);

//0 = success, -1 = error
extern int sc_opt_set_file_pathname_in(sc_opt * opts, const char * path);
//pointer to a private string (can't fail)
extern const char * const * sc_opt_get_file_pathname_in(
    const sc_opt * opts);

/*
 *  NOTE: The following setter requires an initialised vector. The
 *        getter requires an unitialised vector which will be
 *        initialised and populated by the call. On success, the
 *        returned vector be manually destroyed. 
 */

//0 = success, -1 = error
extern int sc_opt_set_sessions(sc_opt * opts, const cm_vct * sessions);
extern const cm_vct * sc_opt_get_sessions(const sc_opt * opts);

//0 = success, -1 = error
extern int sc_opt_set_map(sc_opt * opts, const mc_vm_map * map);
//pointer = success, NULL = error
extern mc_vm_map * sc_opt_get_map(const sc_opt * opts);

//0 = success, -1 = error
extern int sc_opt_set_alignment(sc_opt * opts, const off_t alignment);
//alignment = success, SC_ALIGNMENT_BAD = error
extern off_t sc_opt_get_alignment(const sc_opt opts);

//0 = success. -1 = error
extern int sc_opt_set_addr_width(sc_opt * opts,
                                 const sc_addr_width addr_width);
//address width = success, SC_ADDR_WIDTH_BAD = error
extern int sc_opt_get_addr_width(const sc_opt * opts,
                                 sc_addr_width * addr_width);

//0 = success, -1 = error
extern int sc_opt_set_scan_set(sc_opt * opts,
                               const sc_map_area_set * ma_set);
//map area set attribute pointer = success, NULL = error
extern const sc_map_area_set * sc_opt_get_scan_set(const sc_opt * opts);


/*
 *  --- [OPT_PTRSCAN] ---
 */

//opaque handle = success, NULL = error
extern sc_opt_ptrscan * sc_new_opt_ptr();
extern sc_opt_ptrscan * sc_copy_opt_ptr(
                            const sc_opt_ptrscan * opts_ptr);
//0 = success, -1 = error
extern int sc_copy_assign_opt_ptr(
                            const sc_opt_ptrscan * dst_opts_ptr,
                            const sc_opt_ptrscan * src_opts_ptr);
//void return
extern void sc_del_opt_ptr(sc_opt_ptrscan * opts_ptr);
//0 = success, -1 = error
extern int sc_opt_ptr_reset(sc_opt_ptrscan * opts_ptr);

//0 = success, -1 = error
extern int sc_opt_ptr_set_target_addr(sc_opt_ptrscan * opts_ptr,
                                      const uintptr_t target_addr);
//target address = success, SC_TARGET_ADDR_BAD = error
extern uintptr_t sc_opt_ptr_get_target_addr(
                     const sc_opt_ptrscan * opts_ptr);

//0 = success, -1 = error
extern int sc_opt_ptr_set_alignment(sc_opt_ptrscan * opts_ptr,
                                    const off_t alignment);
//alignment = success, SC_ALIGNMENT_BAD = error
extern off_t sc_opt_ptr_get_alignment(const sc_opt_ptrscan * opts_ptr);

//0 = success. -1 = error
extern int sc_opt_ptr_set_max_obj_sz(sc_opt_ptrscan * opts_ptr,
                                     const off_t max_obj_sz);
//max object size = success, SC_MAX_OBJ_SZ_BAD = error
extern off_t sc_opt_ptr_get_max_obj_sz(const sc_opt_ptrscan * opts_ptr);

//0 = success, -1 = error
extern int sc_opt_ptr_set_max_depth(sc_opt_ptrscan * opts_ptr,
                                    const int max_depth);
//max depth = succeess, SC_MAX_DEPTH_BAD = error
extern int sc_opt_ptr_get_max_depth(const sc_opt_ptrscan * opts_ptr);

//0 = success, -1 = error
extern int sc_opt_ptr_set_static_set(sc_opt_ptrscan * opts_ptr,
                                     const sc_map_area_set * static_set);
//pointer to a private map area set (can't fail)
extern const sc_map_area_set *
    sc_opt_ptr_get_static_set(const sc_opt_ptrscan * opts_ptr);

//0 = success, -1 = error
extern int sc_opt_ptr_set_preset_offsets(
    sc_opt_ptrscan * opts_ptr, const cm_vct * preset_offsets);
//pointer to a private vector (can't fail)
extern const cm_vct *
    sc_opt_ptr_get_preset_offsets(const sc_opt_ptrscan * opts_ptr);

//0 = success, -1 = fail
extern int sc_opt_ptr_set_smart_scan(sc_opt_ptrscan * opts_ptr,
                                     const enum sc_smart_scan smart_scan);
//smart scan enum = success, SC_SMART_SCAN_BAD = errorr
extern int sc_opt_ptr_get_smart_scan(const sc_opt_ptrscan * opts_ptr,
                                     sc_smart_scan * smart_scan);


/*
 *  --- [WORKER_POOL] ---
 */

//return: opaque handle to `worker_pool` object, or NULL on error
extern sc_worker_pool * sc_new_w_pool();
//return: 0 on success, -1 on error
extern void sc_del_w_pool(sc_worker_pool * w_pool);

//return: 0 on success, -1 on error
extern int sc_wp_reset(sc_worker_pool * w_pool);


/*
 *  --- [FILE] --- 
 */

//return: 0 on success, -1 on error
extern int sc_file_get_metadata(
    const sc_opt * opts, struct sc_file_metadata * mdata);


#if 0
/*
 *  --- [SERIALISER] --- 
 */

//return: 0 on success, -1 on error
extern sc_serialiser sc_new_serialiser();
extern int sc_del_serialiser(sc_serialiser serialiser);
extern int sc_save_scan(sc_serialiser serialiser,
                        sc_scan scan, const sc_opt opts);
extern int sc_load_scan(sc_serialiser serialiser,
                        sc_scan scan, const sc_opt opts, const bool shallow);
extern int sc_read_headers(sc_serialiser serialiser, const char * file_path,
                           sc_combined_file_hdr * cmb_hdr);

#endif
#ifdef __cplusplus
} //extern "C"
#endif



      /* ===================== * 
 ===== *  UNIVERSAL INTERFACE  * =====
       * ===================== */

// --- [map area set helpers]
#define SC_GET_SET_KEY(node)  (*((cm_lst_node **) (node->key)))
#define SC_GET_SET_DATA(node)  (*((mc_vm_area **) (node->data)))


// --- [error handling]
//void return
extern "C" {
extern void sc_perror(const char * prefix);
extern const char * sc_strerror(const int sc_errnum);
} //extern "C"



/*
 *  Both the C++ and C interfaces will set sc_errno on error.
 */

extern __thread int sc_errno;


// [error codes] TODO define error code values 3***

// 1XX - user errors
#define SC_ERR_OPT_NOMAP        3100
#define SC_ERR_OPT_NOSESSION    3101
#define SC_ERR_SCAN_EMPTY       3102
#define SC_ERR_OPT_EMPTY        3103
#define SC_ERR_OPT_MISSING      3104
#define SC_ERR_OPT_TYPE         3105
#define SC_ERR_OPT_CHANGED      3106
#define SC_ERR_OPT_BAD          3107
#define SC_ERR_TIMESPEC         3108
#define SC_ERR_IN_USE           3109
#define SC_ERR_NO_RESULT        3110
#define SC_ERR_SHALLOW_RESULT   3111
#define SC_ERR_INVALID_FILE     3112
#define SC_ERR_VERSION_FILE     3113
#define SC_ERR_STATE            3114
#define SC_ERR_BUSY             3115

// 2XX - internal errors
#define SC_ERR_CMORE            3200
#define SC_ERR_MEMCRY           3201
#define SC_ERR_PTHREAD          3202
#define SC_ERR_DEADLOCK         3203
#define SC_ERR_PTR_CHAIN        3204
#define SC_ERR_RTTI             3205
#define SC_ERR_TYPECAST         3206

// 3XX - environment errors
#define SC_ERR_MEM              3300
#define SC_ERR_FILE             3301
#define SC_ERR_PAGESIZE         3302
#define SC_ERR_WORKER_TIMEOUT   3303
#define SC_ERR_FILE_IO          3304


// [error code messages]

// 1XX - user errors
#define SC_ERR_OPT_NOMAP_MSG \
    "Provided opt did not contain a `mc_vm_map`, or the map is empty.\n"
#define SC_ERR_OPT_NOSESSION_MSG \
    "Provided opt did not contain a `mc_session`.\n"
#define SC_ERR_SCAN_EMPTY_MSG \
    "Scan set is empty following an update.\n"
#define SC_ERR_OPT_EMPTY_MSG \
    "`sc_opt` does not contain a value for this entry.\n"
#define SC_ERR_OPT_MISSING_MSG \
    "Required options are not set.\n"
#define SC_ERR_OPT_TYPE_MSG \
    "Mismatching options class provided for a scan.\n"
#define SC_ERR_OPT_CHANGED_MSG \
    "Untimely change in provided options.\n"
#define SC_ERR_OPT_BAD_MSG \
    "Provided option has a bad value.\n"
#define SC_ERR_TIMESPEC_MSG \
    "Failed to fetch the current monotonic time.\n"
#define SC_ERR_IN_USE_MSG \
    "Resource you're attempting to modify is locked.\n"
#define SC_ERR_NO_RESULT_MSG \
    "No results present in this scan.\n"
#define SC_ERR_SHALLOW_RESULT_MSG \
    "Shallow result format can't be verified.\n"
#define SC_ERR_INVALID_FILE_MSG \
    "The provided file is invalid or corrupt.\n"
#define SC_ERR_VERSION_FILE_MSG \
    "The provided file's version is incompatible.\n"
#define SC_ERR_STATE_MSG \
    "Object is in an invalid state for this operation.\n"
#define SC_ERR_BUSY_MSG \
    "Object is busy.\n"

// 2XX - internal errors
#define SC_ERR_CMORE_MSG \
    "Internal: CMore error. See cm_perror().\n"
#define SC_ERR_MEMCRY_MSG \
    "Internal: MemCry error. See mc_perror().\n"
#define SC_ERR_PTHREAD_MSG \
    "Internal: Pthread error.\n"
#define SC_ERR_DEADLOCK_MSG \
    "Internal: Pthreads encountered a deadlock.\n"
#define SC_ERR_PTR_CHAIN_MSG \
    "Internal: Failed to create a pointer chain.\n"
#define SC_ERR_RTTI_MSG \
    "Internal: RTTI cast error.\n"
#define SC_ERR_TYPECAST_MSG \
    "Internal: Typecast between C & C++ interface failed.\n"

// 3XX - environment errors
#define SC_ERR_MEM_MSG \
    "Failed to acquire the necessary memory.\n"
#define SC_ERR_FILE_MSG \
    "Failed to open, read, or write to a file.\n"
#define SC_ERR_PAGESIZE_MSG \
    "Unable to fetch pagesize through sysconf().\n"
#define SC_ERR_WORKER_TIMEOUT_MSG \
    "Operation on worker threads timed out.\n"
#define SC_ERR_FILE_IO_MSG \
    "File I/O failed.\n"

#endif //define SCANCRY_H
