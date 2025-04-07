#ifndef SCANCRY_IMPL_H
#define SCANCRY_IMPL_H

#ifdef __cplusplus
#include <optional>
#include <memory>
#include <vector>
#endif

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
#define _LOCK                                                \
    std::optional<int> _lock_ret = this->_lock();            \
    if (_lock_ret.has_value() == false) return std::nullopt; \

#define _UNLOCK                                              \
    _lock_ret = this->_unlock();                             \
    if (_lock_ret.has_value() == false) return std::nullopt; \


//allows a class to be 
class _lockable {

    _SC_DBG_PRIVATE:
        //lock
        pthread_mutex_t in_use_lock;
        bool in_use;

    public:
        _lockable() : in_use_lock(PTHREAD_MUTEX_INITIALIZER), in_use(false) {}
        ~_lockable();
        
        //lock operations
        std::optional<int> _lock() noexcept;
        std::optional<int> _unlock() noexcept;
        bool _get_lock() const noexcept;
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


/*
 *  This is an empty options class. Options for each scan class inherit
 *  from this class. Workers use a generic `_opt_scan` reference. Through
 *  RTTI the real type is recovered by the appropriate scan class.
 */
class _opt_scan : public _lockable {

    public:
        _opt_scan() : _lockable() {}
        virtual ~_opt_scan();
};


//defined in `scancry.h`
class worker_mngr;

/*
 *  This is an abstract scanner class used for dependency injection.
 */
class _scan : public _lockable {

    public:
        //[methods]
        //dependency injection function
        virtual std::optional<int> _process_addr(
                                        const struct _scan_arg arg) = 0;
        virtual std::optional<int> _manage_scan(worker_mngr & w_mngr) = 0;
};


//worker control flags
const constexpr cm_byte _worker_flag_release_ready = 0x1;
const constexpr cm_byte _worker_flag_exit          = 0x2;
const constexpr cm_byte _worker_flag_cancel        = 0x4;

//concurrent variables shared by a worker manager and its workers
struct _worker_concurrency {

    //release adaptive barrier
    pthread_cond_t release_count_cond;
    pthread_mutex_t release_count_lock;
    int release_count;

    //number of alive threads
    pthread_cond_t alive_count_cond;
    pthread_mutex_t alive_count_lock;
    int alive_count;

    //control flags
    pthread_mutex_t flags_lock;
    cm_byte flags;

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
        //[attributes]

        /*
         *  NOTE: It is necessary to store a reference to the vector
         *        storing all scan area sets, rather than a reference
         *        to just the scan set relevant to this worker. Consider
         *        a case where all scan area sets are destroyed because
         *        the user supplied a new `map_area_set`.
         */
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
        cm_byte * buf;

        //[methods]
        std::optional<int> read_buffer_smart(struct _scan_arg & arg) noexcept;

        //synchronisation
        std::optional<int> release_wait() noexcept;
        std::optional<int> layer_wait() noexcept;
        std::optional<int> exit() noexcept;

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
        ~_worker();

        void main();
};


//used to sort a `map_area_set` hashmap into a vector by size
struct _sa_sort_entry {

    const size_t size;
    const cm_lst_node * area_node;

    _sa_sort_entry(const size_t size, const cm_lst_node * area_node)
     : size(size),
       area_node(area_node) {}
};


//defined in `ptrscan.hh`
class _ptrscan_tree_node;

//pointer scanner cache
struct _ptrscan_cache {

    std::vector<std::shared_ptr<sc::_ptrscan_tree_node>> * depth_level_vct;
};


} //namespace sc
#endif //ifdef __cplusplus


#endif //define SCANCRY_INTERNAL_H
