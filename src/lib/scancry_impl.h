#ifndef SCANCRY_IMPL_H
#define SCANCRY_IMPL_H

//standard template library
#ifdef __cplusplus
#include <new>
#include <optional>
#include <memory>
#include <vector>
#include <functional>
#endif

//system headers
#include <unistd.h>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>


/*
 *  NOTE: Keywords `private`, `static` and `inline` are only defined for 
 *        release builds. This allows unit tests to access class &
 *        compilation unit internals in debug builds.
 */

//debugging & unit testing support
#ifdef DEBUG 
#define _SC_DBG_STATIC
#define _SC_DBG_INLINE
#define _SC_DBG_PRIVATE   public
#define _SC_DBG_PROTECTED public
#else
#define _SC_DBG_STATIC static
#define _SC_DBG_INLINE inline
#define _SC_DBG_PRIVATE   private
#define _SC_DBG_PROTECTED protected
#endif


#ifdef __cplusplus
namespace sc {


//concisely lock/unlock a lockable class
#define _LOCK(badret)                                         \
    { int _lock_ret = this->_lock();                          \
        if (_lock_ret != 0) {                                 \
            return badret;                                    \
        }                                                     \
    }                                                         \

#define _UNLOCK(badret)                                                 \
    { int _lock_ret = this->_unlock();                                  \
    if (_lock_ret != 0) return badret; } \


//allows a class to be locked (prevent modification)
class _lockable {

    _SC_DBG_PRIVATE:
        //lock
        pthread_mutex_t in_use_lock;
        bool in_use;

    public:
        _lockable() : in_use_lock(PTHREAD_MUTEX_INITIALIZER), in_use(false) {}
        ~_lockable();
        
        //lock operations
        [[nodiscard]] int _lock() noexcept;
        [[nodiscard]] int _unlock() noexcept;
        [[nodiscard]] bool _get_lock() const noexcept;
};


//defined in `scancry.h`
class opt;


//argument passed from a worker to the `process_addr()` function
struct _scan_arg {

    //[members]
    uintptr_t addr;
    off_t area_off;
    size_t buf_left;
    cm_byte * cur_byte;
    const cm_lst_node * const area_node;

    //[methods]
    _scan_arg(const uintptr_t addr,
              const off_t area_off,
              const size_t buf_left,
              cm_byte * cur_byte,
              const cm_lst_node * const area_node)
     : addr(addr),
       area_off(area_off),
       buf_left(buf_left),
       cur_byte(cur_byte),
       area_node(area_node) {};
};


//defined in `scancry.h`
class map_area_set;
class worker_pool;


/*
 *  This is an empty options class. Options for each scan class inherit
 *  from this class. Workers use a generic `_opt_scan` reference. Through
 *  RTTI the real type is recovered by the appropriate scan class.
 */
class _opt_scan : public _lockable {

    public:
        //ctor
        _opt_scan() : _lockable() {}
        virtual ~_opt_scan() = 0;

        //reset
        [[nodiscard]] virtual int reset() = 0;
};


/*
 *  This is an abstract scanner class used for dependency injection.
 */
class _scan : public _lockable {

    public:
        //[methods]
        /* internal */ [[nodiscard]] virtual _SC_DBG_INLINE int
                _process_addr(
                    const struct _scan_arg arg, const opt * const opts,
                    const _opt_scan * const opts_scan) = 0;

        /*
         *  NOTE: _generate_body() is responsible for including the 
         *        file end byte (`fbuf_util::_file_end`).
         */

        /* internal */ [[nodiscard]] virtual int _generate_body(
                std::vector<cm_byte> & buf, off_t hdr_off) = 0;
        /* internal */ [[nodiscard]] virtual int _process_body(
                const std::vector<cm_byte> & buf, off_t hdr_off,
                const mc_vm_map & map) = 0;
        /* internal */ [[nodiscard]] virtual int _read_body(
                const std::vector<cm_byte> & buf, off_t hdr_off) = 0;

        [[nodiscard]] virtual int reset() = 0;
};


//worker control flags
const constexpr cm_byte _worker_flag_release_ready = 0x1;
const constexpr cm_byte _worker_flag_exit          = 0x2;
const constexpr cm_byte _worker_flag_cancel        = 0x4;
const constexpr cm_byte _worker_flag_error         = 0x8;

//worker misc.
const constexpr useconds_t _release_broadcast_wait = 50000;


/*
 *  NOTE: For the time being, just save an error string that the user
 *        can optionally view. Ideally, an error string should only be
 *        saved here if it is no longer recoverable.
 */

//concurrent variables shared by a worker manager and its workers
struct _worker_concurrency {

    //release adaptive barrier
    pthread_cond_t release_count_cond;
    pthread_mutex_t release_count_lock;
    volatile int release_count;

    //number of alive threads
    pthread_cond_t alive_count_cond;
    pthread_mutex_t alive_count_lock;
    volatile int alive_count;

    //control flags
    pthread_mutex_t flags_lock;
    volatile cm_byte flags;

    _worker_concurrency()
     : release_count_cond(PTHREAD_COND_INITIALIZER),
       release_count_lock(PTHREAD_MUTEX_INITIALIZER),
       release_count(0),
       alive_count_cond(PTHREAD_COND_INITIALIZER),
       alive_count_lock(PTHREAD_MUTEX_INITIALIZER),
       alive_count(0),
       flags_lock(PTHREAD_MUTEX_INITIALIZER),
       flags(0) {}
};


/*
 *  This class represents a single thread used for scanning some set of
 *  a selected `map_area_set`.
 */
class _worker {

    _SC_DBG_PRIVATE:

        /*
         *  NOTE: It is necessary to store a reference to the vector
         *        storing all scan area sets, rather than a reference
         *        to just the scan set relevant to this worker. Consider
         *        a case where all scan area sets are destroyed because
         *        the user supplied a new `map_area_set`.
         */
        
        //[attributes]
        const std::vector<std::vector<const cm_lst_node *>> & scan_area_sets;
        const int scan_area_index;
        const mc_session * session;

        /*
         *  Pointers to the worker manager's cache.
         */
        sc::opt ** const opts;
        sc::_opt_scan ** const opts_scan;
        sc::_scan ** scan;

        //concurrency variables
        struct _worker_concurrency & concur;

        //read buffer
        std::vector<cm_byte> buf;

        //[methods]
        void exit_flag_handle();
        [[nodiscard]] int read_buffer_smart(struct _scan_arg & arg) noexcept;
        void do_under_mutex(pthread_mutex_t & mutex, std::function<void()> cb);
        void do_under_mutex_critical(pthread_mutex_t & mutex,
                                     const std::string & msg,
                                     std::function<void()> cb);

        //synchronisation
        [[nodiscard]] int release_wait();
        [[nodiscard]] int layer_wait() noexcept;
        void exit(bool is_error);

    public:
        //[methods]
        //ctor
        _worker(sc::opt ** const opts,
                sc::_opt_scan ** const opts_scan,
                sc::_scan ** scan,
                const std::vector<std::vector<const cm_lst_node *>>
                    & scan_area_sets,
                const int scan_area_index,
                const mc_session * session,
                struct sc::_worker_concurrency & concur);

        void main();
};


//used to sort a `map_area_set` hashmap into a vector by size
class _sa_sort_entry {

    _SC_DBG_PRIVATE:
        //[attributes]
        size_t size;
        /* const */ cm_lst_node * area_node;

    public:
         //[methods]
        _sa_sort_entry(const size_t size, const cm_lst_node * area_node)
         : size(size),
           area_node((cm_lst_node *) area_node) {}
    
        //setters & getters
        size_t get_size() const noexcept;
        const cm_lst_node * get_area_node() const noexcept;
};


//defined in `ptrscan.hh`
class _ptrscan_tree_node;

//pointer scanner cache
struct _ptrscan_cache {

    std::vector<std::shared_ptr<sc::_ptrscan_tree_node>> * depth_level_vct;
    std::vector<cm_byte> serial_buf;

    _ptrscan_cache()
     : depth_level_vct(nullptr),
       serial_buf({}) {}
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
const constexpr int _scancry_magic_sz = 4;
const constexpr cm_byte _scancry_magic[_scancry_magic_sz]
                                           = {'S', 'C', 0x13, 0x37};
const constexpr cm_byte _scan_type_ptrscan = 0x00;
const constexpr cm_byte _scan_type_ptnscan = 0x01;
const constexpr cm_byte _scan_type_valscan = 0x02;

//versions
const constexpr cm_byte _scancry_file_ver_0 = 0;


//ScanCry file header
struct _scancry_file_hdr {

    cm_byte magic[_scancry_magic_sz];
    cm_byte version;
    cm_byte scan_type;
};


//pointer scan file header
struct _ptrscan_file_hdr {

    uint32_t pathnames_num;
    uint32_t pathnames_offset;
    uint32_t chains_num;
    uint32_t chains_offset;
};


} //namespace sc
#endif //ifdef __cplusplus


#endif //define SCANCRY_INTERNAL_H
