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
            sc_errno = SC_ERR_IN_USE;        \
            return bad_ret;                  \
        }                                    \
    }                                        \

//concisely write lock a lockable class
#define _LOCK_WRITE(bad_ret)                  \
    { int _lock_wr_ret = this->_lock_write(); \
        if (_lock_wr_ret != 0) {              \
            sc_errno = SC_ERR_IN_USE;         \
            return bad_ret;                   \
        }                                     \
    }                                         \

//concisely wait for a read lock on a lockable class
#define _AWAIT_READ(bad_ret)                  \
    { int _lock_rd_ret = this->_await_read(); \
        if (_lock_rd_ret != 0) {              \
            sc_errno = SC_ERR_IN_USE;         \
            return bad_ret;                   \
        }                                     \
    }                                         \

//concisely wait for a write lock on a lockable class
#define _AWAIT_WRITE(bad_ret)                  \
    { int _lock_wr_ret = this->_await_write(); \
        if (_lock_wr_ret != 0) {               \
            sc_errno = SC_ERR_IN_USE;          \
            return bad_ret;                    \
        }                                      \
    }                                          \

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
        [[nodiscard]] int _await_read() const noexcept;
        [[nodiscard]] int _await_write() const noexcept;
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
        [[nodiscard]] bool get_ctor_failed() const noexcept;
        void _set_ctor_failed(const bool failed) noexcept;
};


/*
 *  NOTE: Many classes want a bitset interface for state management.
 *        This is particularly important for scan types which must be
 *        re-entrant only in some control paths. While rwlocks are
 *        helpful in preventing simultaneous state modification,
 *        management of permitted state changes is delegated to the
 *        `_stateful` subclass.
 *
 *  NOTE: Each class defines its own flags namespace with a `_sf`
 *        (state flag) suffix.
 */

//provide state flags (state machine) for a class
class _stateful {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        cm_byte state_bitset;

    public:
        // -- [methods]
        //ctors
        _stateful() noexcept;
        _stateful(const sc::_stateful & state) = delete;
        _stateful(const sc::_stateful && state) = delete;

        //operators
        sc::_stateful & operator=(
            const sc::_stateful & state) = delete;
        sc::_stateful & operator=(
            const sc::_stateful && state) = delete;

        //setters & getters
        void _set_bits(const cm_byte bitset) noexcept;
        void _unset_bits(const cm_byte bitset) noexcept;
        cm_byte _get_bits(const cm_byte bitset) noexcept;      
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

    _SC_DBG_PRIVATE:
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
        ~_scan_arg() noexcept {}

        //reset the buffer state
        void reset_buf(const size_t new_buf_left,
                          const cm_byte * new_cur_byte) noexcept;

        //advance the buffer
        void advance_buf(const size_t advance) noexcept;

        //getters
        [[nodiscard]] uintptr_t get_addr() const noexcept;
        [[nodiscard]] const cm_lst_node * get_area_node() const noexcept;
        [[nodiscard]] off_t get_area_off() const noexcept;
        [[nodiscard]] const cm_byte * get_cur_byte() const noexcept;
        [[nodiscard]] size_t get_buf_left() const noexcept;
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

//state flags shared by all scan types
namespace _scan_sf {
    const constexpr cm_byte running   = 0b1 << 0; //running on worker pool
    const constexpr cm_byte scan_data = 0b1 << 1; //scan results present
}




//worker control flags
namespace _worker_flag {
    const constexpr cm_byte release_ready = 0b1 << 0;
    const constexpr cm_byte ctrl_run      = 0b1 << 1;
    const constexpr cm_byte exit          = 0b1 << 2;
    const constexpr cm_byte cancel        = 0b1 << 3;
    const constexpr cm_byte error         = 0b1 << 4;
}

//worker misc.
const constexpr useconds_t _release_broadcast_wait = 50000;

//concurrent variables shared by a worker pool and its workers
class _worker_concurrency : public sc::_ctor_failable {

    /*
     * NOTE: Acquisition order:
     *
     *  1) release lock
     *
     *  2) total lock
     *
     *  3) alive lock
     *
     *  4) errno lock
     *
     *  5) flags lock
     */

    _SC_DBG_PRIVATE:
        // -- [attributes]
        //release adaptive barrier
        pthread_cond_t release_count_cond;
        mutable pthread_mutex_t release_count_lock;
        int release_count;

        pthread_cond_t total_count_cond;
        mutable pthread_mutex_t total_count_lock;
        int total_count;

        //number of alive threads
        pthread_cond_t alive_count_cond;
        mutable pthread_mutex_t alive_count_lock;
        int alive_count;

        //control flags
        mutable pthread_mutex_t flags_lock;
        cm_byte flags;

        //worker pool wakeup
        pthread_cond_t threads_ready_cond;
        mutable pthread_mutex_t threads_ready_lock;

        //number of threads to exit
        mutable pthread_mutex_t exit_uids_lock;
        cm_vct exit_uids;

        //errno
        mutable pthread_mutex_t errno_lock;
        int wkr_errno;

    public:
        // -- [methods]
        //ctor & dtor
        _worker_concurrency() noexcept;
        _worker_concurrency(
            const _worker_concurrency & wkr_concur) = delete;
        _worker_concurrency(
            const _worker_concurrency && wkr_concur) = delete;
        ~_worker_concurrency() noexcept;

        // - worker calls

        //concurrency operators - control
        void wkr_release_wait() noexcept;
        void wkr_enter() noexcept;
        void wkr_exit(const bool is_error) noexcept;
        [[nodiscard]] int wkr_check_kill(
            const int uid, bool & do_exit) noexcept;

        //concurrency operators - error propagation
        void wkr_set_errno(const int wkr_sc_errno) noexcept;

        // - worker pool calls

        //concurrency operators - control
        [[nodiscard]] int wp_await_wkrs(
            const bool do_block, const bool do_timeout) noexcept;
        void wp_release_wkrs() noexcept;

        void wp_set_total_wkrs(const int total) noexcept;
        void wp_fix_release() noexcept;
        
        [[nodiscard]] int wp_wkr_kill(const int uid) noexcept;
        void wp_wkr_kill_reset() noexcept;

        //concurrency operators - error propagation
        void wp_reset_error() noexcept;

        // - worker & worker pool calls

        //check errno
        [[nodiscard]] int get_errno() noexcept;

        //concurrency operators - flags
        void set_flags(const cm_byte bitmask) noexcept;
        void unset_flags(const cm_byte bitmask) noexcept;
        [[nodiscard]] cm_byte get_flags() const noexcept;
};

//references to attributes of the worker pool
class _worker_pool_cache {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        //state
        bool is_locked;

        //options
        const sc::opt * opts;
        const sc::_opt_scan * opts_scan; 

        //scan object reference
        sc::_scan * scan;

    public:
        // -- [methods]
        //ctor
        _worker_pool_cache(
            const sc::opt * opts,
            const sc::_opt_scan * opts_scan,
            sc::_scan * scan) noexcept
             : is_locked(false),
               opts(opts),
               opts_scan(opts_scan),
               scan(scan) {}
        _worker_pool_cache(const _worker_pool_cache & pool_cache) = delete;
        _worker_pool_cache(const _worker_pool_cache && pool_cache) = delete;
        ~_worker_pool_cache() noexcept;

        //lock & unlock cache
        [[nodiscard]] int lock() noexcept;
        void unlock() noexcept;

        //getters
        [[nodiscard]] const sc::opt * get_opts() const noexcept;
        [[nodiscard]] const sc::_opt_scan * get_opts_scan() const noexcept;
        [[nodiscard]] sc::_scan * get_scan() const noexcept;
};

/*
 *  NOTE: This class represents a single thread used for scanning
 *         some set of a selected `map_area_set`.
 */

class _worker : public sc::_ctor_failable {

    _SC_DBG_PRIVATE:

        /*
         *  NOTE: It is necessary to store a reference to the vector
         *        storing all scan area sets, rather than a reference
         *        to just the scan set relevant to this worker. Consider
         *        a case where all scan area sets are destroyed because
         *        the user supplied a new `map_area_set`.
         */
        
        // -- [attributes]
        const int & uid;
        
        const cm_vct /* <const cm_lst_node *> */ & scan_area_subset;

        //memcry session
        const int session_idx;
        mc_session * cached_session;

        //shared state
        const sc::_worker_pool_cache & pool_cache;
        sc::_worker_concurrency & concur;

        //read buffer
        cm_byte * buf;

        // -- [methods]
        [[nodiscard]] int handle_exit() noexcept;
        [[nodiscard]] int read_buf_smart(
                              struct _scan_arg & arg) noexcept;

    public:
        // -- [methods]
        //ctor
        _worker(const int & uid,
                const struct sc::_worker_pool_cache & pool_cache,
                struct sc::_worker_concurrency & concur, 
                const cm_vct /* <const cm_lst_node *> */ & scan_area_subset,
                const int session_idx) noexcept;
        _worker(const _worker & wkr) = delete;
        _worker(const _worker && wkr) = delete;
        ~_worker() noexcept;

        //thread main
        void main() noexcept;

        //getters
        [[nodiscard]] int get_uid() noexcept;
};


//worker unit managed by the worker pool
class _worker_bundle : public _ctor_failable {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        int uid;
        sc::_worker wkr;
        pthread_t thread_id;
        cm_vct /* <const cm_lst_node *> */ scan_area_subset;

    public:
        // -- [methods]
        //ctor & dtor
        _worker_bundle(
            const int uid,
            const struct sc::_worker_pool_cache & pool_cache,
            sc::_worker_concurrency & concur,
            const int session_idx) noexcept;
        _worker_bundle(const _worker_bundle & wkr_bundle) = delete;
        _worker_bundle(const _worker_bundle && wkr_bundle) = delete;
        ~_worker_bundle() noexcept;

        //getters
        [[nodiscard]] int get_wkr_uid() noexcept;
        [[nodiscard]] cm_vct & get_scan_area_subset() noexcept;
};

#define _SC_GET_NODE_WKR_BUNDLE(node) ((sc::_worker_bundle *) (node->data))

//worker pool state flags
namespace _worker_pool_sf {
    const constexpr cm_byte bound   = 0b1 << 0; //bound to a scan
    const constexpr cm_byte running = 0b1 << 1; //running a scan
}


class _ptr_tree_node : public _lockable {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        //children of this code
        cm_lst /* <sc::_ptrscan_tree_node> */ child_nodes;

    public:
        // -- [attributes]
        const int id;
        const cm_lst_node * const area_node;

        const uintptr_t own_addr;
        const uintptr_t ptr_addr;

        const sc::_ptr_tree_node * parent;
    
        // -- [methods]
        //ctors & dtor
        _ptr_tree_node(
            const int id,
            const cm_lst_node * area_node,
            const uintptr_t own_addr,
            const uintptr_t ptr_addr,
            const _ptr_tree_node * parent) noexcept;
        _ptr_tree_node(
            const sc::_ptr_tree_node & p_tree_node) = delete;
        _ptr_tree_node(
            const sc::_ptr_tree_node && p_tree_node) = delete;
        ~_ptr_tree_node() noexcept;

        //operators
        sc::_ptr_tree_node & operator=(
            const sc::_ptr_tree_node & p_tree) = delete;
        sc::_ptr_tree_node & operator=(
            const sc::_ptr_tree_node && p_tree) = delete;

        //add a child
        [[nodiscard]] sc::_ptr_tree_node * add_child(
            const int id,
            const cm_lst_node * area_node,
            const uintptr_t own_addr,
            const uintptr_t ptr_addr) noexcept;

        //determine if this is a leaf node
        [[nodiscard]] bool has_children() const noexcept;

        //get children
        [[nodiscard]] const cm_lst /* <sc::_ptr_tree_node> */ &
            get_children() const noexcept;
};

#define _GET_NODE_PTR_TREE_NODE(node) \
    ((sc::_ptr_tree_node *) (node->data))


class _ptr_tree : public _lockable, public _ctor_failable {

    _SC_DBG_PRIVATE:
        // -- [attributes]
        int next_id;

        //nodes at each level of the pointer tree
        cm_vct /* cm_vct <sc::_ptrscan_tree_node *> */ depth_lvls;
        sc::_ptr_tree_node * root_node;

    public:
        //[methods]
        //ctors & dtor
        _ptr_tree() noexcept;
        _ptr_tree(const sc::_ptr_tree & p_tree) = delete;
        _ptr_tree(const sc::_ptr_tree && p_tree) = delete;
        ~_ptr_tree() noexcept;

        //operators
        sc::_ptr_tree & operator=(const sc::_ptr_tree & p_tree) = delete;
        sc::_ptr_tree & operator=(const sc::_ptr_tree && p_tree) = delete;

        //reset
        [[nodiscard]] int reset() noexcept;

        //set a new root node
        [[nodiscard]] int init_root_node(
            const cm_lst_node * area_node,
            const uintptr_t own_addr,
            const uintptr_t ptr_addr) noexcept;

        //depth level operations
        [[nodiscard]] int inc_depth_lvl() noexcept;

        [[nodiscard]] int add_to_lvl(
            const int lvl,
            const sc::_ptr_tree_node * p_tree_node) noexcept;

        //getters
        [[nodiscard]] int get_next_id() noexcept;
        
        [[nodiscard]] const cm_vct /* <sc::_ptr_tree_node *> */ *
            get_depth_lvl(const int lvl) const noexcept;
            
        [[nodiscard]] const sc::_ptr_tree_node *
            get_root_node() const noexcept;
};

//state flags shared by all scan types
namespace _ptrscan_sf {
    const constexpr cm_byte chains_data = 0b1 << 2; //chains constructed
    const constexpr cm_byte file_data   = 0b1 << 3; //file data ready
}


} //namespace sc
#endif //ifdef __cplusplus


#endif //define SCANCRY_INTERNAL_H
