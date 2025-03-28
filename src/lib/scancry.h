#ifndef SCANCRY_H
#define SCANCRY_H

#include <cstddef>
#ifdef __cplusplus
//standard template library
#include <optional>
#include <memory>
#include <vector>
#include <list>
#include <unordered_set>
#include <string>
#include <type_traits>
#endif

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry_impl.h"



/*
 *  NOTE: ScanCry should be accessible through the C ABI for cross
 *        language compatibility. While the internals make use of C++,
 *        this interface either restricts itself to C, or provides C
 *        wrappers to C++.
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
 *  Configuration options for scans.
 */

//architecture address width enum
enum addr_width {
    AW32 = 4,
    AW64 = 8
};


class opt {

    _SC_DBG_PRIVATE:
        //[attributes]
        //save & load file paths
        std::optional<const std::string> file_path_out;
        std::optional<const std::string> file_path_in;

        /*
         *  The number of threads used during scans is determined 
         *  by the number of sessions. Each thread requires its
         *  own session (because each thread needs its own seek
         *  position).
         */
        
        //MemCry sessions
        std::vector<const mc_session *> sessions;

        //MemCry memory map of target
        mc_vm_map const * map;

        /*
         *  The following attributes define constraints to apply
         *  to mc_vm_map. For example, `omit_objs` will not include
         *  any objects in this vector in the scan. Constraints can be
         *  applied at object and area granularity.
         *
         *  omit_*      - Do not include these areas/objects in the scan.
         *  exclusive_* - Only scan these areas/objects (union set).
         */

        //scan constraints
        std::optional<const std::vector<const cm_lst_node *>> omit_areas;
        std::optional<const std::vector<const cm_lst_node *>> omit_objs;
        std::optional<const std::vector<const cm_lst_node *>> exclusive_areas;
        std::optional<const std::vector<const cm_lst_node *>> exclusive_objs;
        std::optional<const std::pair<const uintptr_t, const uintptr_t>> addr_range;
        std::optional<cm_byte> access;

    public:
        //[attributes]
        const enum addr_width addr_width;
    
        //[methods]
        //ctor
        opt(enum addr_width _addr_width)
        : map(nullptr), addr_width(_addr_width) {}


        //getters & setters
        void set_file_path_out(const std::optional<std::string> & file_path_out);
        const std::optional<std::string> & get_file_path_out() const;

        void set_file_path_in(const std::optional<std::string> & file_path_in);
        const std::optional<std::string> & get_file_path_in() const;

        void set_sessions(const std::vector<mc_session const *> & sessions);
        const std::vector<mc_session const *> & get_sessions() const;

        void set_map(const mc_vm_map * map) noexcept;
        mc_vm_map const * get_map() const noexcept;
        
        void set_omit_areas(const std::optional<std::vector<cm_lst_node *>> & omit_areas);
        const std::optional<std::vector<cm_lst_node *>> & get_omit_areas() const;

        void set_omit_objs(const std::optional<std::vector<cm_lst_node *>> & omit_objs);
        const std::optional<std::vector<cm_lst_node *>> & get_omit_objs() const;

        void set_exclusive_areas(const std::optional<std::vector<cm_lst_node *>> & exclusive_areas);
        const std::optional<std::vector<cm_lst_node *>> & get_exclusive_areas() const;

        void set_exclusive_objs(const std::optional<std::vector<cm_lst_node *>> & exclusive_objs);
        const std::optional<std::vector<cm_lst_node *>> & get_exclusive_objs() const;

        void set_addr_range(const std::optional<std::pair<uintptr_t, uintptr_t>> & addr_range);
        const std::optional<std::pair<uintptr_t, uintptr_t>> get_addr_range() const;

        void set_access(const std::optional<cm_byte> & access) noexcept;
        std::optional<cm_byte> get_access() const noexcept;
};



/*
 *  Configuration options only applicable to pointer scans
 */

class opt_ptrscan {

    _SC_DBG_PRIVATE:
        //[attributes]
        /* The following 4 optionals are _not_ optional, they must be set. */
        std::optional<uintptr_t> target_addr;
        std::optional<off_t> alignment;
        std::optional<off_t> max_obj_sz;
        std::optional<int> max_depth;

        std::optional<std::unordered_set<cm_lst_node *>> static_areas;
        std::optional<std::vector<off_t>> preset_offsets;

        /*
         *  NOTE: With `smart_scan` on, every potential pointer will be 
         *        treated as pointing only to one parent node to which it's
         *        offset is smallest.
         *
         *        For example: imagine 4 objects of size 0x40 each allocated
         *        inside an array. With `max_obj_sz` of 0x100, a potential
         *        pointer to the start of the array will technically point to
         *        each object in the array. With `smart_scan` set to true, it
         *        will only point to the first object.
         */
        
        bool smart_scan;

    public:
        //ctor
        opt_ptrscan()
        : smart_scan(false) {}

        //getters & setters
        void set_target_addr(const std::optional<uintptr_t> & target_addr);
        std::optional<uintptr_t> get_target_addr() const noexcept;
        
        void set_alignment(const std::optional<off_t> & alignment);
        std::optional<off_t> get_alignment() const noexcept;
        
        void set_max_obj_sz(const std::optional<off_t> & max_obj_sz);
        std::optional<off_t> get_max_obj_sz() const noexcept;
        
        void set_max_depth(const std::optional<int> & max_depth);
        std::optional<int> get_max_depth() const noexcept;
        
        void set_static_areas(const std::optional<std::vector<cm_lst_node *>> & static_areas);
        std::optional<std::unordered_set<cm_lst_node *>> get_static_areas() const;
        
        void set_preset_offsets(const std::optional<std::vector<off_t>> & preset_offsets);
        std::optional<std::vector<off_t>> get_preset_offsets() const;
        
        void set_smart_scan(bool do_smart_scan);
        bool get_smart_scan() const noexcept;    
};


/*
 *  Abstraction above mc_vm_map; uses constraints from the opt class
 *  to arrive at a final set of areas to scan.
 */
class map_area_set {

    _SC_DBG_PRIVATE:
        //[attributes]
        std::unordered_set<cm_lst_node *> area_nodes;

    public:
        //[methods]
        std::optional<int> update_set(const opt & opts);

        //getters & setters
        const std::unordered_set<cm_lst_node *> & get_area_nodes() const noexcept {
            return area_nodes;
        }
};


/*
 *  Pointer scanner. 
 */
class _ptrscan_tree_node;
class _ptrscan_tree;

class ptrscan : public _scan {

    _SC_DBG_PRIVATE:
        //[attributes]
        //pointer scan tree
        std::unique_ptr<_ptrscan_tree> tree_p;
        int cur_depth_level;

        //cache
        std::vector<std::shared_ptr<sc::_ptrscan_tree_node>> * cache_depth_level_vct;

        //[methods]
        void _add_node(std::shared_ptr<_ptrscan_tree_node> parent_node,
                       const cm_lst_node * area_node, const uintptr_t own_addr, const uintptr_t ptr_addr);

    public:
        //[methods]
        //scanner dependency injection function
        void process_addr(_scan_arg arg, const opt & opts, const void * const opts_custom);
};


/*
 *  Manager of worker threads, responsible for spawning, dispatching,
 *  synchronising, and cleaning up threads. The parameters for the
 *  scan are determined by an instance of `opt` class. Dependency
 *  injection is used to determine the type of scan performed.
 */

const constexpr cm_byte WORKER_MNGR_KEEP_WORKERS  = 0x1;
const constexpr cm_byte WORKER_MNGR_KEEP_SCAN_SET = 0x2;

class worker_mngr {

    _SC_DBG_PRIVATE:
        //[attributes]
        //worker threads
        std::vector<_worker> workers;
        std::vector<pthread_t> worker_ids;

        //memory areas each worker scans
        std::vector<sc::_sa_sort_entry> sorted_entries;
        std::vector<std::vector<const cm_lst_node *>> scan_area_sets;

        //scan type
        sc::_scan * scan;

        //concurrency
        struct _worker_concurrency concur;

        //worker signalling
        cm_byte flags;
        bool in_use;

        //[methods]
        std::optional<int> spawn_workers(const opt & opts);
        std::optional<int> kill_workers();
        std::optional<int> sort_by_size(const map_area_set & ma_set);
        std::optional<int> update_scan_area_set(const map_area_set & ma_set);

    public:
        //[methods]
        //ctor & dtor
        worker_mngr()
         : flags(0), in_use(false) {}
        ~worker_mngr();

        //control workers
        std::optional<int> update_workers(const opt & opts,
                                          const map_area_set & ma_set, const cm_byte flags);
        std::optional<int> destroy_workers();

        //control scan area

        std::optional<int> do_scan(_scan * scan);
        std::optional<int> cancel();
};


}; //namespace sc
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

//opaque types
typedef void * sc_opt;
typedef void * sc_map_area_set;


//address range for sc_opt
typedef struct {
    uintptr_t min;
    uintptr_t max;
} sc_addr_range;


//architecture address width enum
enum sc_addr_width {
    AW32 = 4,
    AW64 = 8
};



/*
 *  --- [OPT] ---
 */

/*
 *  To request to unset an option, pass `nullptr` for pointers
 *  or `-1` for numerical values.
 */

//return opaque handle to `opt` object, or NULL on error
extern sc_opt sc_new_opt(const enum sc_addr_width addr_width);
//return 0 on success, -1 on error
extern int sc_del_opt(sc_opt opts);

//return 0 on success, -1 on error
extern int sc_opt_set_file_path_out(sc_opt opts, const char * path);
//return output file path string if set, NULL if not set
extern const char * sc_opt_get_file_path_out(const sc_opt opts);

//return 0 on success, -1 on error
extern int sc_opt_set_file_path_in(sc_opt opts, const char * path);
//return input file path string if set, NULL if not set
extern const char * sc_opt_get_file_path_in(const sc_opt opts);

/*
 * The following setter requires an initialised CMore vector (`cm_vct`)
 * holding `mc_session *`. The getter requires an unitialised CMore
 * vector which will be initialised and populated by the call. Must be
 * manually destroyed later. 
 */

//all return 0 on success, -1 on error
extern int sc_opt_set_sessions(sc_opt opts, const cm_vct * sessions);
extern int sc_opt_get_sessions(const sc_opt opts, cm_vct * sessions);

//void return
extern void sc_opt_set_map(sc_opt opts, const mc_vm_map * map);
//return MemCry map const pointer if set, NULL if not set
extern mc_vm_map const * sc_opt_get_map(const sc_opt opts);

//void return
extern int sc_opt_set_alignment(sc_opt opts, const unsigned int alignment);
//return alignment int if set, -1 if not set
extern unsigned int sc_opt_get_alignment(const sc_opt opts);

//return arch width in bytes if set, -1 if not set
extern enum sc_addr_width sc_opt_get_addr_width(const sc_opt opts);

/*
 * The following setters require an initialised CMore vector (`cm_vct`)
 * holding `cm_lst_node *`. The getters require an unitialised CMore
 * vector which will be initialised and populated by the call. Must be
 * manually destroyed later. 
 */

//all return 0 on success, -1 on error
extern int sc_opt_set_omit_areas(sc_opt opts, const cm_vct * omit_areas);
extern int sc_opt_get_omit_areas(const sc_opt opts, cm_vct * omit_areas);

//all return 0 on success, -1 on error
extern int sc_opt_set_omit_objs(sc_opt opts, const cm_vct * omit_objs);
extern int sc_opt_get_omit_objs(const sc_opt opts, cm_vct * omit_objs);

//all return 0 on success, -1 on error
extern int sc_opt_set_exclusive_areas(sc_opt opts, const cm_vct * exclusive_areas);
extern int sc_opt_get_exclusive_areas(const sc_opt opts, cm_vct * exclusive_areas);

//all return 0 on success, -1 on error
extern int sc_opt_set_exclusive_objs(sc_opt opts, const cm_vct * exclusive_objs);
extern int sc_opt_get_exclusive_objs(const sc_opt opts, cm_vct * exclusive_objs);

//all return 0 on success, -1 on error
extern int sc_opt_set_addr_range(sc_opt opts, const sc_addr_range * range);
//return sc_addr_range, both fields zero if unset
extern int sc_opt_get_addr_range(const sc_opt opts, sc_addr_range * range);

//return 0 on success, -1 on error
extern int sc_opt_set_access(sc_opt opts, const cm_byte access);
//return access mask on success, -1 if not set
extern cm_byte sc_opt_get_access(const sc_opt opts);


/*
 *  --- [SCAN_SET] ---
 */

//return opaque handle to `map_area_set` object, or NULL on error
extern sc_map_area_set sc_new_map_area_set();
//return 0 on success, -1 on error
extern int sc_del_map_area_set(sc_map_area_set s_set);

//return 0 on success, -1 on failure
extern int sc_update_set(sc_map_area_set s_set, const sc_opt opts);

/*
 * The following getter requires an unitialised CMore vector which
 * will be initialised and populated by the call. Must be manually
 * destroyed later. NOTE: Unlike the C++ interface which returns a
 * hashmap, the C interface returns a sorted vector.
 */

//return 0 on success, -1 on failure
extern int sc_get_set(const sc_map_area_set s_set, cm_vct * area_nodes);


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
#define SC_ERR_TIMESPEC       3104

// 2XX - internal errors
#define SC_ERR_CMORE          3200
#define SC_ERR_MEMCRY         3201
#define SC_ERR_PTHREAD        3202
#define SC_ERR_EXCP           3203
#define SC_ERR_RUN_EXCP       3204

// 3XX - environment errors
#define SC_ERR_MEM            3300


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
#define SC_ERR_TIMESPEC_MSG \
    "Failed to fetch the current monotonic time.\n"

// 2XX - internal errors
#define SC_ERR_CMORE_MSG \
    "Internal: CMore error. See cm_perror().\n"
#define SC_ERR_MEMCRY_MSG \
    "Internal: MemCry error. See mc_perror().\n"
#define SC_ERR_PTHREAD_MSG \
    "Internal: Pthread error.\n"
#define SC_ERR_EXCP_MSG \
    "Internal: An unrecoverable exception was thrown.\n"
#define SC_ERR_RUN_EXCP_MSG \
    "Internal: An unrecoverable runtime exception was thrown.\n"

// 3XX - environment errors
#define SC_ERR_MEM_MSG \
    "Failed to acquire the necessary memory.\n"


#endif //define SCANCRY_H
