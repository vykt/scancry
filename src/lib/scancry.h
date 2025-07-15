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


/*
 *  FIXME: Make C++ includes only see the C++ part of this file. At the 
 *         same time, compiling the library has to see both.
 */


      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

#ifdef __cplusplus
namespace sc {


/*
 *  --- [CLASSES] ---
 */

/*
 *  NOTE: This is a single address range, used when specifying address
 *        range based constraints.
 */

class addr_range {

    _SC_DBG_PRIVATE:
        //[attributes]
        uintptr_t start_addr;
        uintptr_t end_addr;

    public:
        //[methods]
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
        //[attributes]
        cm_vct /* <const cm_lst_node *> */ omit_areas;
        cm_vct /* <const cm_lst_node *> */ omit_objs;
        cm_vct /* <const cm_lst_node *> */ exclusive_areas;
        cm_vct /* <const cm_lst_node *> */ exclusive_objs;
        cm_vct /* <sc::addr_range> */ omit_addr_ranges;
        cm_vct /* <sc::addr_range> */ exclusive_addr_ranges;
        cm_byte access;

        //[methods]
        void do_copy(sc::opt_map_area & opts_ma) noexcept;

    public:
        //[methods]
        //ctors & dtor
        opt_map_area() noexcept;
        opt_map_area(sc::opt_map_area & opts_ma) noexcept;
        opt_map_area(sc::opt_map_area && opts_ma) = delete;
        ~opt_map_area() noexcept;

        //operators
        sc::opt_map_area & operator=(sc::opt_map_area & opt_ma) noexcept;
        sc::opt_map_area & operator=(sc::opt_map_area && opt_ma) = delete;

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
        //[attributes]
        cm_rbt /* <const cm_lst_node * : nullptr> */ set;

        //[methods]
        void do_copy(sc::map_area_set & ma_set) noexcept;

    public:
        //[methods]
        //ctors & dtor
        map_area_set() noexcept;
        map_area_set(sc::map_area_set & ma_set) noexcept;
        map_area_set(sc::map_area_set && ma_set) = delete;
        ~map_area_set() noexcept;

        //operators
        sc::map_area_set & operator=(sc::map_area_set & ma_set) noexcept;
        sc::map_area_set & operator=(sc::map_area_set && ma_set) = delete;

        //reset
        [[nodiscard]] int reset() noexcept;

        //populate
        [[nodiscard]] int update_set(
            sc::opt_map_area & opts_ma, const mc_vm_map & map) noexcept;

        //getters & setters
        [[nodiscard]] const cm_rbt /* <const cm_lst_node *> */ &
            get_set() const noexcept;
};


//architecture address width enum
enum addr_width /* parity with sc_addr_width */ {
    SC_AW32  = 4,
    SC_AW64  = 8,
    SC_ADDR_WIDTH_UNSET = -1,
};


/*
 *  NOTE: This class defines generic configuration options relevant to
 *        all scan types.
 */

//unset values
namespace val_unset {
    const constexpr enum addr_width addr_width = SC_ADDR_WIDTH_UNSET;
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
        //[attributes]

        /*
         *  NOTE: The number of threads used during scans is determined 
         *        by the number of provided sessions.
         */

        //save & load file paths
        char * file_pathname_out;
        char * file_pathname_in;
        
        //sessions & map
        cm_vct /* <const mc_session *> */ sessions;
        mc_vm_map * map;

        //address width (32bit / 64bit)
        enum addr_width addr_width;

        //set of areas to scan
        sc::map_area_set scan_set;

        //[methods]
        void do_copy(sc::opt & opts) noexcept;

    public:
        //[methods]
        /* internal */ [[nodiscard]] sc::map_area_set &
            _get_scan_set_mut() noexcept;
        
        //ctors & dtor
        opt() noexcept;
        opt(opt & opts) noexcept;
        opt(opt && opts) = delete;
        ~opt() noexcept;

        //operators
        sc::opt & operator=(sc::opt & opts) noexcept;
        sc::opt & operator=(sc::opt && opts) = delete;

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
            sc::map_area_set & scan_set) noexcept;
        [[nodiscard]] const sc::map_area_set &
            get_scan_set() const noexcept;
};



/*
 *  NOTE: This class defines configuration options only applicable to
 *        pointer scans.
 */

enum smart_scan /* parity with sc_smart_scan */ {
    SC_SMART_SCAN_ENABLED = 0,
    SC_SMART_SCAN_DISABLED = 1,
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
    const constexpr enum smart_scan smart_scan = SC_SMART_SCAN_ENABLED;
}

//bad values
namespace val_bad {
    const constexpr uintptr_t       target_addr = UINTPTR_MAX;
    const constexpr off_t           alignment   = -1;
    const constexpr off_t           max_obj_sz  = -1;
    const constexpr int             max_depth   = -1;
}

class opt_ptr final : public _opt_scan {

    _SC_DBG_PRIVATE:
        //[attributes]
        //address to scan for
        uintptr_t target_addr;

        //pointer alignment in bytes
        off_t alignment;

        //maximum structure size to accept
        off_t max_obj_sz;

        //number of iterations to perform (time grows exponentially)
        int max_depth;

        //areas to treat as terminal nodes (areas holding static globals)
        sc::map_area_set static_set;

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

        //[methods]
        void do_copy(sc::opt_ptr & opts_ptr) noexcept;

    public:
        //[methods]
        /* internal */ [[nodiscard]] sc::map_area_set &
            _get_static_set_mut() noexcept;
    
        //ctors & dtor
        opt_ptr() noexcept;
        opt_ptr(opt_ptr & opts_ptr) noexcept;
        opt_ptr(opt_ptr && opts_ptr) = delete;
        ~opt_ptr() noexcept override final;

        //operators
        sc::opt_ptr & operator=(sc::opt_ptr & opts_ptr) noexcept;
        sc::opt_ptr & operator=(sc::opt_ptr && opts_ptr) = delete;

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
            sc::map_area_set & static_set) noexcept;
        [[nodiscard]] const sc::map_area_set &
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
    const constexpr cm_byte keep_workers  = 0x1 << 0;
    const constexpr cm_byte keep_scan_set = 0x1 << 1;
}

#if 0
class worker_pool : public _lockable {

    _SC_DBG_PRIVATE:
        //[attributes]
        //worker threads
        cm_vct /* <sc const::_worker> */ workers;
        cm_vct /* <sc::pthread_t> */ worker_ids;

        //local copy of the last provided scan set (`map_area_set`), sorted
        //by area size
        cm_vct /* <sc::_sa_sort_entry> */ sorted_entries;
        //a scan set per worker
        cm_vct /* <cm_vct <const cm_lst_node *>> */ worker_scan_sets;

        //options cache
        sc::opt * opts;
        sc::_opt_scan * opts_scan;
        sc::_scan * scan;

        //concurrency
        struct _worker_concurrency concur;

        //[methods]
        [[nodiscard]] int spawn_workers() noexcept;
        [[nodiscard]] int kill_workers() noexcept;
        [[nodiscard]] int sort_by_size(
            const map_area_set & scan_set) noexcept;
        [[nodiscard]] int update_scan_area_set(
            const map_area_set & scan_set) noexcept;

    public:
        //[methods]
        //perform a single pass over the scan set
        /* internal */ [[nodiscard]] int _single_run() noexcept;
        
        //setup ahead of a scan
        /* internal */ [[nodiscard]] int _setup(
                                        sc::opt & opts,
                                        sc::_opt_scan & opts_scan,
                                        sc::_scan & scan,
                                        const sc::map_area_set & ma_set,
                                        const cm_byte flags) noexcept;
        
        //ctor & dtor
        worker_pool();
        worker_pool(const worker_pool & wp) = delete;
        worker_pool(const worker_pool && wp) = delete;
        ~worker_pool();

        //control workers
        [[nodiscard]] int free_workers() noexcept;
};



/*
 *  Pointer scanner. 
 */

class _ptrscan_tree_node;
class _ptrscan_tree;


//pointer scanner flattened tree
class ptrscan_chain {

    _SC_DBG_PRIVATE:
        //[attributes]
        uint32_t obj_idx;
        const cm_lst_node * obj_node;
        const char * pathname;
        cm_vct /* <off_t> */ offsets;

    public:
        //[methods]
        /* internal */ uint32_t _get_obj_idx() const noexcept;
    
        //ctors & dtor
        ptrscan_chain(const cm_lst_node * obj_node,
                      const uint32_t _obj_idx,
                      const cm_vct /* <off_t> */ & offsets);
        ptrscan_chain(const char * pathname,
                      const uint32_t _obj_idx,
                      const cm_vct /* <off_t> */ & offsets);
        ~ptrscan_chain();
        
        //getters & setters
        const cm_lst_node * get_obj_node() const noexcept;
        const char * get_pathname() const noexcept;
        const cm_vct /* <off_t> */ & get_offsets() const noexcept;
};
 

class ptrscan : public _scan {

    /* FIXME: Pointer scan state needs to save options used to produce
     *        the results, and store it to dist to be read later. */

    _SC_DBG_PRIVATE:
        //[attributes]
        //pointer scan tree
        sc::_ptrscan_tree * tree_p;
        int cur_depth_level;

        //flattened tree chains
        cm_vct /* <const char * (alloc)> */ ser_pathnames;
        cm_vct /* <struct ptrscan_chain> */ chains;

        //cache
        struct _ptrscan_cache cache;

        //[methods]
        void add_node(_ptrscan_tree_node * parent_node,
                      const cm_lst_node * area_node,
                      const uintptr_t own_addr,
                      const uintptr_t ptr_addr);

        [[nodiscard]] sc::_ptrscan_chain_data get_chain_data(
            const cm_lst_node * const area_node) const noexcept;

        [[nodiscard]] int get_chain_idx(const char * pathname);

        [[nodiscard]] bool
            is_chain_valid(const uintptr_t target_addr,
                           const struct sc::ptrscan_chain & chain,
                           mc_session & session) const;

        [[nodiscard]] sc::_ptrscan_fbuf_data_sz
            get_fbuf_data_sz() const noexcept;

#if 0 //TODO FIXME: Just rewrite the serialiser, shit is awful (Thanks STL!)
        [[nodiscard]] int handle_body_start(
            const std::vector<cm_byte> & buf, off_t hdr_off, off_t & buf_off);
        [[nodiscard]]
            std::optional<std::pair<uint32_t, std::vector<off_t>>>
                handle_body_chain(
                    const std::vector<cm_byte> & buf, off_t & buf_off);
#endif
        [[nodiscard]] int flatten_tree();

        void do_reset();

    public:
        //[methods]
        /* internal */ [[nodiscard]] off_t _process_addr(
                    const struct _scan_arg arg,
                    const opt * const opts,
                    const _opt_scan * const opts_scan)
                    noexcept override final;

        /* internal */ [[nodiscard]] int _generate_body(
                    cm_vct /* <cm_byte> */ & buf,
                    const off_t hdr_off) noexcept override final;
#if 0 //TODO FIXME: Just rewrite the serialiser pt. 2
        /* internal */ [[nodiscard]] int _process_body(
                    const std::vector<cm_byte> & buf, off_t hdr_off,
                    const mc_vm_map & map) override final;
        /* internal */ [[nodiscard]] int _read_body(
                    const std::vector<cm_byte> & buf,
                    off_t hdr_off) override final;
#endif

        //ctors
        ptrscan();
        ptrscan(const ptrscan & ptr_s) = delete;
        ptrscan(const ptrscan && ptr_s) = delete;
        ~ptrscan();
        
        [[nodiscard]] int reset() override final;

        //perform a scan
        [[nodiscard]] int scan(
                    sc::opt & opts,
                    sc::opt_ptr & opts_ptr,
                    sc::map_area_set & ma_set,
                    worker_pool & w_pool,
                    cm_byte flags);

        //verify chains
        [[nodiscard]] int verify(
            sc::opt & opts, const sc::opt_ptr & opts_ptr);

        //getters & setters
        [[nodiscard]]
            const cm_vct /* <sc::ptrscan_chain> */ &
                get_chains() const noexcept;
};


/*
 *  NOTE: ScanCry uses a binary file format with the following sections:
 *
 *        Name format: <process comm>.<pid>.sc
 *               e.g.: netnote.1823.sc
 *
 *        File format:
 *                     [ 1. ScanCry header ]
 *                     [ 2. scan header    ]
 *                     [ 3. data           ]
 *
 *        The ScanCry header is a generic header applicable to all files.
 *
 */

//scancry header constants
const constexpr int file_magic_sz = 4;
const constexpr cm_byte file_magic[file_magic_sz]
                                           = {'S', 'C', 0x13, 0x37};
const constexpr cm_byte scan_type_ptr = 0x00;
const constexpr cm_byte scan_type_ptn = 0x01;
const constexpr cm_byte scan_type_val = 0x02;

//versions
const constexpr cm_byte file_ver_0 = 0;


//ScanCry file header
struct scancry_file_hdr {

    cm_byte magic[file_magic_sz];
    cm_byte version;
    cm_byte scan_type;
};


//pointer scan file header
struct ptr_file_hdr {

    uint32_t pathnames_num;
    uint32_t pathnames_offset;
    uint32_t chains_num;
    uint32_t chains_offset;
};


//combined ScanCry & scan header struct
struct combined_file_hdr {
    struct scancry_file_hdr scancry_hdr;
    union {
        ptr_file_hdr ptr_hdr;
    };
};


class serialiser : public _lockable {

    _SC_DBG_PRIVATE:
        //[methods]
        //miscellaneous
        [[nodiscard]] std::optional<cm_byte>
            get_scan_type(_scan * scan) const;
        [[nodiscard]] bool
            is_header_valid(sc::scancry_file_hdr & hdr) const;

    public:
        //file operations
        [[nodiscard]] int save_scan(
            sc::_scan & scan, const sc::opt & opts);
        [[nodiscard]] int load_scan(
            sc::_scan & scan, const sc::opt & opts, const bool shallow);
        [[nodiscard]] std::optional<combined_file_hdr> read_headers(
            const char * file_path);
};
#endif

}; //namespace `sc`
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
    SC_ADDR_WIDTH_BAD   = -2
};

typedef struct sc_opt sc_opt;


// -- scan options

enum sc_smart_scan /* parity with sc::smart_scan */ {
    SC_SMART_SCAN_ENABLED = 0,
    SC_SMART_SCAN_DISABLED = 1,
    SC_SMART_SCAN_ERROR = -1
};

//unset values
#define SC_ADDR_WIDTH_UNSET SC_ADDR_WIDTH_UNSET

//default values
#define SC_ALIGNMENT_DEFAULT  0x4
#define SC_MAX_OBJ_SZ_DEFAULT 0x100
#define SC_MAX_DEPTH_DEFAULT  3
#define SC_SMART_SCAN_DEFAULT SC_SMART_SCAN_ENABLED

//bad values
#define SC_ADDR_WIDTH_BAD  SC_ADDR_WIDTH_BAD
#define SC_TARGET_ADDR_BAD UINTPTR_MAX
#define SC_ALIGNMENT_BAD   -1
#define SC_MAX_OBJ_SZ_BAD  -1
#define SC_MAX_DEPTH_BAD   -1
#define SC_SMART_SCAN_BAD  SC_SMART_SCAN_ERROR

typedef /* base */ struct sc_opt_scan sc_opt_scan;
typedef struct sc_opt_ptr sc_opt_ptr;


// -- scan types

typedef /* base */ struct sc_scan sc_scan;
typedef struct sc_ptrscan sc_ptrscan;


// -- worker pool

typedef struct sc_worker_pool sc_worker_pool;


// -- serialiser

typedef struct sc_serialiser sc_serialiser;


//scancry header magic
#define SC_FILE_MAGIC_SZ 4
#define SC_FILE_MAGIC {'S', 'C', 0x13, 0x37}

//scancry header file type
#define SC_SCAN_TYPE_PTR 0x00
#define SC_SCAN_TYPE_PTN 0x01;
#define SC_SCAN_TYPE_VAL 0x02;


//scancry file header
typedef struct {

    cm_byte magic[SC_FILE_MAGIC_SZ];
    cm_byte version;
    cm_byte scan_type;

} sc_scancry_file_hdr;


//pointer scan file header
typedef struct {

    uint32_t pathnames_num;
    uint32_t pathnames_offset;
    uint32_t chains_num;
    uint32_t chains_offset;

} sc_ptr_file_hdr;


//combined scancry & scan header struct
typedef struct combined_file_hdr {
    sc_scancry_file_hdr scancry_hdr;
    union {
        sc_ptr_file_hdr ptr_hdr;
    };
} sc_combined_file_hdr;


/*
 *  NOTE: For all functions, see `sc_errno` on error.
 */

/*
 *  --- [OPT_MAP_AREA] ---
 */

//opaque handle = success, NULL = error 
extern sc_opt_map_area * sc_new_opt_ma();
extern sc_opt_map_area * sc_copy_opt_ma(sc_opt_map_area * opts_ma);
//void return
extern void sc_del_opt_ma(sc_opt_map_area * opts_ma);
//0 = success, -1 = error
extern int sc_opt_ma_reset(sc_opt_map_area * opts_ma);

//setters: 0 = success, -1 = error
//getters: pointer = success, NULL = error

//omit areas
extern int sc_opt_ma_set_omit_areas(
    sc_opt_map_area * opts_ma, const cm_vct * omit_areas);
extern const cm_vct * sc_opt_ma_get_omit_areas(sc_opt_map_area * opts_ma);

//omit objects
extern int sc_opt_ma_set_omit_objs(
    sc_opt_map_area * opts_ma, const cm_vct * omit_objs);
extern const cm_vct * sc_opt_ma_get_omit_objs(sc_opt_map_area * opts_ma);

//exclusive areas
extern int sc_opt_ma_set_exclusive_areas(
    sc_opt_map_area * opts_ma, const cm_vct * exclusive_areas);
extern const cm_vct * sc_opt_ma_get_exclusive_areas(sc_opt_map_area * opts_ma);

//exclusive objects
extern int sc_opt_ma_set_exclusive_objs(
    sc_opt_map_area * opts_ma, const cm_vct * exclusive_objs);
extern const cm_vct * sc_opt_ma_get_exclusive_objs(sc_opt_map_area * opts_ma);

//omit address ranges
extern int sc_opt_ma_set_omit_addr_ranges(
    sc_opt_map_area * opts_ma, const cm_vct * omit_addr_ranges);
//only for this getter: 0 = success, -1 = fail, deallocate vector manually
extern int sc_opt_ma_get_omit_addr_ranges(
    sc_opt_map_area * opts_ma, cm_vct * addr_ranges);

//exclusive address ranges
extern int sc_opt_ma_set_exclusive_addr_ranges(
    sc_opt_map_area * opts_ma, const cm_vct * exclusive_addr_ranges);
extern int sc_opt_ma_get_exclusive_addr_ranges(
    sc_opt_map_area * opts_ma, cm_vct * addr_ranges);

//access
//0 = success, CM_BYTE_MAX = error
extern int sc_opt_ma_set_access(
    sc_opt_map_area * opts_ma, const cm_byte access);
//CM_BYTE_MAX = error, SC_ACCESS_UNSET = not set, other = success
extern cm_byte sc_opt_ma_get_access(sc_opt_map_area * opts_ma);


/*
 *  --- [MAP_AREA_SET] ---
 */

//opaque handle = success, NULL = error
extern sc_map_area_set * sc_new_ma_set();
extern sc_map_area_set * sc_copy_ma_set(sc_map_area_set * ma_set);
//void return
extern void sc_del_ma_set(sc_map_area_set * ma_set);
//0 = success, -1 = error
extern int sc_ma_set_reset(sc_map_area_set * ma_set);

//0 = success, -1 = error
extern int sc_ma_set_update_set(sc_map_area_set * ma_set,
                                sc_opt_map_area * opts_ma,
                                const mc_vm_map * map);
//pointer = success, -1 = error
extern const cm_rbt * sc_get_set(sc_map_area_set * ma_set);


/*
 *  --- [OPT] ---
 */

//opaque handle = success, NULL = error
extern sc_opt * sc_new_opt();
extern sc_opt * sc_copy_opt(const sc_opt * opts);
//void return
extern void sc_del_opt(sc_opt * opts);
//0 = success, -1 = error
extern int sc_opt_reset(sc_opt * opts);

//0 = success, -1 = error
extern int sc_opt_set_file_pathname_out(sc_opt * opts, const char * path);
//pointer to a private string (can't fail)
extern const char ** sc_opt_get_filename_pathname_out(const sc_opt * opts);

//0 = success, -1 = error
extern int sc_opt_set_file_pathname_in(sc_opt * opts, const char * path);
//pointer to a private string (can't fail)
extern const char ** sc_opt_get_file_pathname_in(const sc_opt * opts);

/*
 *  NOTE: The following setter requires an initialised vector. The
 *        getter requires an unitialised vector which will be
 *        initialised and populated by the call. On success, the
 *        returned vector be manually destroyed. 
 */

//0 = success, -1 = error
extern int sc_opt_set_sessions(sc_opt * opts,
                               const cm_vct /* <mc_session> */ * sessions);
extern int sc_opt_get_sessions(const sc_opt * opts,
                               cm_vct /* <mc_session> */ * sessions);

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
extern enum sc_addr_width sc_opt_get_addr_width(const sc_opt * opts);

//0 = success, -1 = error
extern int sc_opt_set_scan_set(sc_opt * opts,
                               const sc_opt_map_area * opts_ma);
//map area set attribute pointer = success, NULL = error
extern const sc_map_area_set * sc_opt_get_scan_set(const sc_opt * opts);


/*
 *  --- [OPT_PTR] ---
 */

//opaque handle = success, NULL = error
extern sc_opt_ptr * sc_new_opt_ptr();
extern sc_opt_ptr * sc_copy_opt_ptr(const sc_opt_ptr * opts_ptr);
//void return
extern void sc_del_opt_ptr(sc_opt_ptr * opts_ptr);
//0 = success, -1 = error
extern int sc_opt_ptr_reset(sc_opt_ptr * opts_ptr);

//0 = success, -1 = error
extern int sc_opt_ptr_set_target_addr(sc_opt_ptr * opts_ptr,
                                      const uintptr_t target_addr);
//target address = success, SC_TARGET_ADDR_BAD = error
extern uintptr_t sc_opt_ptr_get_target_addr(const sc_opt_ptr * opts_ptr);

//0 = success, -1 = error
extern int sc_opt_ptr_set_alignment(sc_opt_ptr * opts_ptr,
                                    const off_t alignment);
//alignment = success, SC_ALIGNMENT_BAD = error
extern off_t sc_opt_ptr_get_alignment(const sc_opt_ptr * opts_ptr);

//0 = success. -1 = error
extern int sc_opt_ptr_set_max_obj_sz(sc_opt_ptr * opts_ptr,
                                     const off_t max_obj_sz);
//max object size = success, SC_MAX_OBJ_SZ_BAD = error
extern off_t sc_opt_ptr_get_max_obj_sz(const sc_opt_ptr * opts_ptr);

//0 = success, -1 = error
extern int sc_opt_ptr_set_max_depth(sc_opt_ptr * opts_ptr,
                                    const int max_depth);
//max depth = succeess, SC_MAX_DEPTH_BAD = error
extern int sc_opt_ptr_get_max_depth(const sc_opt_ptr * opts_ptr);

//0 = success, -1 = error
extern int sc_opt_ptr_set_static_set(sc_opt_ptr * opts_ptr,
                                     const sc_map_area_set * static_set);
//pointer to a private map area set (can't fail)
extern const sc_map_area_set *
    sc_opt_ptr_get_static_set(const sc_opt_ptr * opts_ptr);

//0 = success, -1 = error
extern int sc_opt_ptr_set_preset_offsets(
    sc_opt_ptr * opts_ptr, const cm_vct * preset_offsets);
//pointer to a private vector (can't fail)
extern const cm_vct *
    sc_opt_ptr_get_preset_offsets(const sc_opt_ptr * opts_ptr);

//0 = success, -1 = fail
extern int sc_opt_ptr_set_smart_scan(sc_opt_ptr * opts_ptr,
                                     const enum sc_smart_scan smart_scan);
//smart scan enum = success, SC_SMART_SCAN_BAD = errorr
extern enum sc_smart_scan
    sc_opt_ptr_get_smart_scan(const sc_opt_ptr * opts_ptr);

#if 0
/*
 *  --- [WORKER_POOL] ---
 */

//return: opaque handle to `worker_pool` object, or NULL on error
extern sc_worker_pool sc_new_worker_pool();
//return: 0 on success, -1 on error
extern int sc_del_worker_pool(sc_worker_pool w_pool);

//return: 0 on success, -1 on error
extern int sc_wp_free_workers(sc_worker_pool w_pool);


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
#define SC_ERR_OPT_NOMAP      3100
#define SC_ERR_OPT_NOSESSION  3101
#define SC_ERR_SCAN_EMPTY     3102
#define SC_ERR_OPT_EMPTY      3103
#define SC_ERR_OPT_MISSING    3104
#define SC_ERR_OPT_TYPE       3105
#define SC_ERR_TIMESPEC       3106
#define SC_ERR_IN_USE         3107
#define SC_ERR_NO_RESULT      3108
#define SC_ERR_SHALLOW_RESULT 3109
#define SC_ERR_INVALID_FILE   3110
#define SC_ERR_VERSION_FILE   3111

// 2XX - internal errors
#define SC_ERR_CMORE          3200
#define SC_ERR_MEMCRY         3201
#define SC_ERR_PTHREAD        3202
#define SC_ERR_DEADLOCK       3203
#define SC_ERR_PTR_CHAIN      3204
#define SC_ERR_RTTI           3205
#define SC_ERR_TYPECAST       3206

// 3XX - environment errors
#define SC_ERR_MEM            3300
#define SC_ERR_FILE           3301


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
#define SC_ERR_TIMESPEC_MSG \
    "Failed to fetch the current monotonic time.\n"
#define SC_ERR_IN_USE_MSG \
    "Resource you're attempting to modify is currently in use.\n"
#define SC_ERR_NO_RESULT_MSG \
    "No results present in this scan.\n"
#define SC_ERR_SHALLOW_RESULT_MSG \
    "Shallow result format can't be verified.\n"
#define SC_ERR_INVALID_FILE_MSG \
    "The provided file is invalid or corrupt.\n"
#define SC_ERR_VERSION_FILE_MSG \
    "The provided file's version is incompatible.\n"

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

#endif //define SCANCRY_H
