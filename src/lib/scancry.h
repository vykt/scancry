#ifndef SCANCRY_H
#define SCANCRY_H

#include <cstddef>
#ifdef __cplusplus
//standard template library
#include <optional>
#include <vector>
#include <unordered_set>
#include <string>
#endif

//external libraries
#include <cmore.h>
#include <memcry.h>


/*
 *  NOTE: ScanCry should be accessible through the C ABI for cross
 *  language compatibility. While the internals make use of C++, this
 *  interface either restricts itself to C, or provides C wrappers to C++.
 */



// [internal unit testing support]
#ifdef DEBUG 
#define _SC_DBG_STATIC
#define _SC_DBG_INLINE
#define _SC_DBG_PRIVATE public
#else
#define _SC_DBG_STATIC static
#define _SC_DBG_INLINE inline
#define _SC_DBG_PRIVATE private
#endif



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
class opt {

    _SC_DBG_PRIVATE:
        //[attributes]
        //save & load file paths
        std::optional<std::string> file_path_out;
        std::optional<std::string> file_path_in;

        /*
         *  The number of threads used during scans is determined 
         *  by the number of sessions. Each thread requires its
         *  own session (because each thread needs its own seek
         *  position).
         */
        
        //MemCry sessions
        std::vector<mc_session const *> sessions;

        //MemCry memory map of target
        mc_vm_map const * map;

        //scan core settings
        std::optional<unsigned int> alignment;
        const unsigned int arch_byte_width;

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
        std::optional<std::vector<cm_lst_node *>> omit_areas;
        std::optional<std::vector<cm_lst_node *>> omit_objs;
        std::optional<std::vector<cm_lst_node *>> exclusive_areas;
        std::optional<std::vector<cm_lst_node *>> exclusive_objs;
        std::optional<std::pair<uintptr_t, uintptr_t>> addr_range;

    public:
        //[methods]
        //ctor
        opt(unsigned int _arch_byte_width)
        : map(nullptr), arch_byte_width(_arch_byte_width) {}


        //getters & setters
        void set_file_path_out(std::string file_path_out);
        const std::optional<std::string> & get_file_path_out() const;

        void set_file_path_in(std::string file_path_in);
        const std::optional<std::string> & get_file_path_in() const;

        void set_sessions(std::vector<mc_session const *> sessions);
        const std::vector<mc_session const *> & get_sessions() const;

        void set_map(mc_vm_map * map);
        mc_vm_map const * get_map() const;

        void set_alignment(int alignment);
        std::optional<unsigned int> get_alignment() const;

        unsigned int get_arch_byte_width() const;
        
        void set_omit_areas(std::vector<cm_lst_node *> & omit_areas);
        const std::optional<std::vector<cm_lst_node *>> &
                                       get_omit_areas() const;

        void set_omit_objs(std::vector<cm_lst_node *> & omit_objs);
        const std::optional<std::vector<cm_lst_node *>> &
                                       get_omit_objs() const;

        void set_exclusive_areas(std::vector<cm_lst_node *> & exclusive_areas);
        const std::optional<std::vector<cm_lst_node *>> &
                                       get_exclusive_areas() const;

        void set_exclusive_objs(std::vector<cm_lst_node *> & exclusive_objs);
        const std::optional<std::vector<cm_lst_node *>> &
                           get_exclusive_objs() const;

        void set_addr_range(std::pair<uintptr_t, uintptr_t> addr_range);
        const std::optional<std::pair<uintptr_t, uintptr_t>>
                                     get_addr_range() const;
};


/*
 *  Abstraction above mc_vm_map; uses constraints from the opt class
 *  to arrive at a final set of areas to scan.
 */
class scan_set {

    private:
        //[attributes]
        std::unordered_set<cm_lst_node *> area_nodes;

    public:
        //[methods]
        std::optional<int> update_scan_areas(const cm_byte access_mask,
                                             const opt & opt);

        //getters & setters
        const std::unordered_set<cm_lst_node *> & get_area_nodes() const {
            return area_nodes;
        }
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
typedef void * sc_scan_set;

//address range for sc_opt
typedef struct {
    uintptr_t min;
    uintptr_t max;
} sc_addr_range;



/*
 *  --- [OPT] ---
 */

// [opt]
//return opaque handle to `opt` object, or NULL on error
extern sc_opt sc_new_opt(const int arch_byte_width);
//return 0 on success, -1 on error
extern int sc_del_opt(sc_opt opts);

//return 0 on success, -1 on error
extern int sc_opt_set_file_path_out(sc_opt opts, const char * path);
//return output file path string if set, NULL if not set
extern const char * sc_opt_get_file_path_out(sc_opt opts);

//return 0 on success, -1 on error
extern int sc_opt_set_file_path_in(sc_opt opts, const char * path);
//return input file path string if set, NULL if not set
extern const char * sc_opt_get_file_path_in(sc_opt opts);

/*
 * The following setter requires an initialised CMore vector (`cm_vct`)
 * holding `mc_session *`. The getter requires an unitialised CMore
 * vector which will be initialised and populated by the call. Must be
 * manually destroyed later. 
 */

//all return 0 on success, -1 on error
extern int sc_opt_set_sessions(sc_opt opts, cm_vct * sessions);
extern int sc_opt_get_sessions(sc_opt opts, cm_vct * sessions);

//void return
extern void sc_opt_set_map(sc_opt opts, mc_vm_map * map);
//return MemCry map const pointer if set, NULL if not set
extern mc_vm_map const * sc_opt_get_map(sc_opt opts);

//void return
extern int sc_opt_set_alignment(sc_opt opts, int alignment);
//return alignment int if set, -1 if not set
extern unsigned int sc_opt_get_alignment(sc_opt opts);

//return arch width in bytes if set, -1 if not set
extern unsigned int sc_opt_get_arch_byte_width(sc_opt opts);

/*
 * The following setters require an initialised CMore vector (`cm_vct`)
 * holding `cm_lst_node *`. The getters require an unitialised CMore
 * vector which will be initialised and populated by the call. Must be
 * manually destroyed later. 
 */

//all return 0 on success, -1 on error
extern int sc_opt_set_omit_areas(sc_opt opts, cm_vct * omit_areas);
extern int sc_opt_get_omit_areas(sc_opt opts, cm_vct * omit_areas);

//all return 0 on success, -1 on error
extern int sc_opt_set_omit_objs(sc_opt opts, cm_vct * omit_objs);
extern int sc_opt_get_omit_objs(sc_opt opts, cm_vct * omit_objs);

//all return 0 on success, -1 on error
extern int sc_opt_set_exclusive_areas(sc_opt opts, cm_vct * exclusive_areas);
extern int sc_opt_get_exclusive_areas(sc_opt opts, cm_vct * exclusive_areas);

//all return 0 on success, -1 on error
extern int sc_opt_set_exclusive_objs(sc_opt opts, cm_vct * exclusive_objs);
extern int sc_opt_get_exclusive_objs(sc_opt opts, cm_vct * exclusive_objs);

//void return
extern int sc_opt_set_addr_range(sc_opt opts, sc_addr_range * range);
//return sc_addr_range, both fields zero if unset
extern int sc_opt_get_addr_range(sc_opt opts, sc_addr_range * range);



/*
 *  --- [SCAN_SET] ---
 */

//return opaque handle to `scan_set` object, or NULL on error
extern sc_scan_set sc_new_scan_set();
//return 0 on success, -1 on error
extern int sc_del_scan_set(sc_scan_set s_set);

//return 0 on success, -1 on failure
extern int sc_update_scan_areas(sc_scan_set s_set,
                                const sc_opt opts, const cm_byte access_mask);

/*
 * The following getter requires an unitialised CMore vector which
 * will be initialised and populated by the call. Must be manually
 * destroyed later. NOTE: Unlike the C++ interface which returns a
 * hashmap, the C interface returns a sorted vector.
 */

//return 0 on success, -1 on failure
extern int sc_scan_set_get_area_nodes(const sc_scan_set s_set,
                                      cm_vct * area_nodes);


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

// 2XX - internal errors
#define SC_ERR_CMORE          3200
#define SC_ERR_MEMCRY         3201
#define SC_ERR_EXCP           3202
#define SC_ERR_RUN_EXCP       3203

// 3XX - environment errors
#define SC_ERR_MEM            3300


// [error code messages]

// 1XX - user errors
#define SC_ERR_OPT_NOMAP_MSG \
    "Provided opt did not contain a `mc_vm_map`.\n"
#define SC_ERR_OPT_NOSESSION_MSG \
    "Provided opt did not contain a `mc_session`.\n"
#define SC_ERR_SCAN_EMPTY_MSG \
    "Scan set is empty following an update.\n"
#define SC_ERR_OPT_EMPTY_MSG \
    "`sc_opt` does not contain a value for this entry.\n"

// 2XX - internal errors
#define SC_ERR_CMORE_MSG \
    "Internal: CMore error. See cm_perror().\n"
#define SC_ERR_MEMCRY_MSG \
    "Internal: MemCry error. See mc_perror().\n"
#define SC_ERR_EXCP_MSG \
    "Internal: An exception was thrown.\n"
#define SC_ERR_RUN_EXCP_MSG \
    "Internal: A runtime exception was thrown.\n"

// 3XX - environment errors
#define SC_ERR_MEM_MSG \
    "Failed to acquire the necessary memory.\n"


#endif //define SCANCRY_H
