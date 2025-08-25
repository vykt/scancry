#ifndef SCANCRY_IMPL_H
#define SCANCRY_IMPL_H

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
#ifdef SC_DEBUG 
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


//concisely read lock a lockable class
#define _LOCK_READ(bad_ret)                  \
    { int _lock_rd_ret = this->_lock_read(); \
        if (_lock_rd_ret != 0) {             \
            return bad_ret;                  \
        }                                    \
    }                                        \

//concisely write lock a lockable class
#define _LOCK_WRITE(bad_ret)                  \
    { int _lock_wr_ret = this->_lock_write(); \
        if (_lock_wr_ret != 0) {              \
            return bad_ret;                   \
        }                                     \
    }                                         \

//concisely unlock a lockable class
#define _UNLOCK { this->_unlock(); }


//allow a class to be locked (prevent modification)
class _lockable {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        mutable pthread_rwlock_t lock;

        // -- [methods]
        void do_copy(const sc::_lockable & lockable) noexcept;

    public:
        // -- [methods]
        //ctors
        _lockable() noexcept;
        _lockable(const sc::_lockable & lockable) noexcept;
        _lockable(const sc::_lockable && lockable) = delete;

        //operators
        sc::_lockable & operator=(const sc::_lockable & lockable) noexcept;
        sc::_lockable & operator=(const sc::_lockable && lockable) = delete;
        
        //lock operations
        [[nodiscard]] int _lock_read() const noexcept;
        [[nodiscard]] int _lock_write() const noexcept;
        void _unlock() const noexcept;
};


//allow a class constructor to fail without throwing an exception
class _ctor_failable {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        bool ctor_failed;

        // -- [methods]
        void do_copy(const sc::_ctor_failable & ctor_failable) noexcept;

    public:
        // -- [methods]
        //ctors
        _ctor_failable() noexcept;
        _ctor_failable(const sc::_ctor_failable & ctor_failable) noexcept;
        _ctor_failable(const sc::_ctor_failable && ctor_failable) = delete;

        //operators
        sc::_ctor_failable & operator=(
            const sc::_ctor_failable & ctor_failable) noexcept;
        sc::_ctor_failable & operator=(
            const sc::_ctor_failable && ctor_failable) = delete;

        //getter
        [[nodiscard]] bool _get_ctor_failed() const noexcept;
        void _set_ctor_failed(const bool failed) noexcept;
};


/*
 *  NOTE: This is an interface class for scan options.
 */

class _opt_scan : public _lockable, public _ctor_failable {

    _SC_DBG_PRIVATE:
        // -- [methods]
        void do_copy(sc::_opt_scan & opts_scan) noexcept;

    public:
        // -- [methods]
        //ctor
        _opt_scan() noexcept;
        _opt_scan(_opt_scan & opts_scan) noexcept;
        _opt_scan(_opt_scan && opts_scan) = delete;
        virtual ~_opt_scan() noexcept = 0;

        //operators
        sc::_opt_scan & operator=(sc::_opt_scan & opts_scan) noexcept;
        sc::_opt_scan & operator=(sc::_opt_scan && opts_scan) = delete;

        //reset
        [[nodiscard]] virtual int reset() = 0;
};


//defined in `scancry.h`
class opt;


//defined in `scancry.h`
class map_area_set;
class worker_pool;


//argument passed from a worker to the `process_addr()` function
class _scan_arg {

        // -- [attributes]
        uintptr_t addr;
    
        const cm_lst_node * area_node;
        off_t area_off;

        const cm_byte * cur_byte;
        size_t buf_left;

    public:
        // -- [methods]
        //ctors
        _scan_arg(const uintptr_t addr,
                  const cm_lst_node * const area_node,
                  const off_t area_off,
                  const cm_byte * cur_byte,
                  const size_t buf_left) noexcept
         : addr(addr),
           area_node(area_node),
           area_off(area_off),
           cur_byte(cur_byte),
           buf_left(buf_left) {}
        _scan_arg(const _scan_arg & scan_arg) = delete;
        _scan_arg(const _scan_arg && scan_arg) = delete;

        //reset the buffer state
        void reset_buffer(const size_t new_buf_left,
                          const cm_byte * new_cur_byte) noexcept;

        //advance the buffer
        void advance_buffer(const size_t advance) noexcept;

        //getters
        [[nodiscard]] uintptr_t get_addr() noexcept;
        [[nodiscard]] const cm_lst_node * get_area_node() noexcept;
        [[nodiscard]] off_t get_area_off() noexcept;
        [[nodiscard]] const cm_byte * get_cur_byte() noexcept;
        [[nodiscard]] size_t get_buf_left() noexcept;
};


/*
 *  NOTE: This is an abstract scanner class used for dependency injection.
 */

class _scan : public _lockable {

    public:
        // -- [methods]
        //return: number of bytes to advance the buffer by
        /* internal */ [[nodiscard]]
            virtual off_t _process_addr(
                const struct _scan_arg & arg,
                const opt & opts,
                const _opt_scan & opts_scan) = 0;

        [[nodiscard]] virtual int reset() = 0;
};


//worker control flags
namespace _worker_flag {
    const constexpr cm_byte release_ready = 0x1;
    const constexpr cm_byte exit          = 0x2;
    const constexpr cm_byte cancel        = 0x4;
    const constexpr cm_byte error         = 0x8;
}


//worker misc.
const constexpr useconds_t _release_broadcast_wait = 50000;


//concurrent variables shared by a worker pool and its workers
class _worker_concurrency {

    /*
     * NOTE: Acquisition order:
     *
     *  1) release lock
     *
     *  2) alive lock
     *
     *  3) flags lock
     */

    _SC_DBG_PRIVATE:
        // -- [attributes]
        //release adaptive barrier
        pthread_cond_t release_count_cond;
        mutable pthread_mutex_t release_count_lock;
        volatile int release_count;

        //number of alive threads
        pthread_cond_t alive_count_cond;
        mutable pthread_mutex_t alive_count_lock;
        volatile int alive_count;

        //control flags
        mutable pthread_mutex_t flags_lock;
        volatile cm_byte flags;

        //worker pool wakeup
        pthread_cond_t threads_ready_cond;
        mutable pthread_mutex_t threads_ready_lock;

    public:
        // -- [methods]
        //ctors
        _worker_concurrency(const int wkr_count) noexcept
         : release_count_cond(PTHREAD_COND_INITIALIZER),
           release_count_lock(PTHREAD_MUTEX_INITIALIZER),
           release_count(0),
           alive_count_cond(PTHREAD_COND_INITIALIZER),
           alive_count_lock(PTHREAD_MUTEX_INITIALIZER),
           alive_count(0),
           flags_lock(PTHREAD_MUTEX_INITIALIZER),
           flags(0),
           threads_ready_cond(PTHREAD_COND_INITIALIZER),
           threads_ready_lock(PTHREAD_MUTEX_INITIALIZER) {}
        _worker_concurrency(
            const _worker_concurrency & wkr_concur) = delete;
        _worker_concurrency(
            const _worker_concurrency && wkr_concur) = delete;

        // - worker calls

        //concurrency operators - counts
        void wkr_release_wait() noexcept;
        void wkr_enter() noexcept;
        void wkr_exit(const bool is_error) noexcept;

        // - worker & worker pool calls

        //concurrency operators - flags
        void set_flags(const cm_byte bitmask) noexcept;
        [[nodiscard]] cm_byte get_flags() const noexcept;
};


//references to attributes of the worker pool
class _worker_pool_cache {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        //options
        const sc::opt *  opts;
        const sc::_opt_scan * opts_scan; 

        //scan object reference
        sc::_scan * scan;

    public:
        // -- [methods]
        //ctors
        _worker_pool_cache(
            const sc::opt * const & opts,
            const sc::_opt_scan * const & opts_scan,
            sc::_scan * const & scan) noexcept
             : opts(opts),
               opts_scan(opts_scan),
               scan(scan) {}
        _worker_pool_cache(const _worker_pool_cache & pool_cache) = delete;
        _worker_pool_cache(const _worker_pool_cache && pool_cache) = delete;

        //getters & setters
        void set_opts(const sc::opt * opts) noexcept;
        [[nodiscard]] const sc::opt * get_opts() const noexcept;

        void set_opts_scan(const sc::_opt_scan * opts_scan) noexcept; 
        [[nodiscard]] const sc::_opt_scan * get_opts_scan() const noexcept;

        void set_scan(const sc::_scan * scan) noexcept;
        [[nodiscard]] sc::_scan * get_scan() const noexcept;
};


/*
 *  NOTE: This class represents a single thread used for scanning
 *         some set of a selected `map_area_set`.
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
        
        // -- [attributes]
        const cm_vct /* <const cm_lst_node *> */ & scan_area_subset;
        const mc_session * session;

        //shared state
        const struct sc::_worker_pool_cache & pool_cache;
        const struct sc::_worker_concurrency & concur;

        //read buffer
        cm_byte * buf;

        // -- [methods]
        [[nodiscard]] int read_buffer_smart(
                              struct _scan_arg & arg) noexcept;

    public:
        // -- [methods]
        //ctor
        _worker(const struct sc::_worker_pool_cache & pool_cache,
                struct sc::_worker_concurrency & concur, 
                const cm_vct /* <const cm_lst_node *> */ & scan_area_subset,
                const mc_session *& session) noexcept;
        _worker(const _worker & wkr) = delete;
        _worker(const _worker && wkr) = delete;

        void main() noexcept;
};


//worker unit managed by the worker pool
class _worker_bundle {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        sc::_worker worker;
        pthread_t thread_id;
        cm_vct /* <const cm_lst_node *> */ & scan_area_subset;

    public:
        // -- [methods]
        //ctors
        _worker_bundle(
            const struct sc::_worker_pool_cache & pool_cache,
            struct sc::_worker_concurrency & concur,
            const cm_vct /* <const cm_lst_node *> */ & scan_area_subset,
            const mc_session *& session) noexcept;
        _worker_bundle(const _worker_bundle & wkr_bundle) = delete;
        _worker_bundle(const _worker_bundle && wkr_bundle) = delete;
};


//defined in `ptrscan.hh`
class _ptrscan_tree_node;

//pointer scanner cache
struct _ptrscan_cache {

    cm_vct /* <sc::_ptrscan_tree_node> */ * depth_level_vct;

    _ptrscan_cache()
     : depth_level_vct(nullptr) {}
};


//pointer chain data
struct _ptrscan_chain_data {

    const char * pathname;
    const cm_lst_node * area_node;

    _ptrscan_chain_data(const char * pathname,
                        const cm_lst_node * area_node) 
     : pathname(pathname),
       area_node(area_node) {}
};


//size of pathnames & chains in save file
struct _ptrscan_fbuf_data_sz {

    const size_t pathnames_sz;
    const size_t chains_sz;

    _ptrscan_fbuf_data_sz(const size_t pathnames_sz,
                          const size_t chains_sz)
     : pathnames_sz(pathnames_sz),
       chains_sz(chains_sz) {}
};


} //namespace sc
#endif //ifdef __cplusplus


#endif //define SCANCRY_INTERNAL_H
