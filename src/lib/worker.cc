//C++ standard library
#include <new>

//C standard library
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <cerrno>

//system headers
#include <unistd.h>
#include <sys/time.h>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry.h"
#include "scancry_impl.h"
#include "common.hh"
#include "error.hh"

//debug headers
#ifdef SC_DEBUG
#include "debug.hh"
#endif



      /* =============== * 
 ===== *  C++ INTERFACE  * =====
       * =============== */

/*
 *  --- [WORKER_CONCURRENCY | INTERNAL] ---
 */

//ctor & dtor
sc::_worker_concurrency::_worker_concurrency() noexcept
    : _ctor_failable(),
      release_count_cond(PTHREAD_COND_INITIALIZER),
      release_count_lock(PTHREAD_MUTEX_INITIALIZER),
      release_count(0),
      alive_count_cond(PTHREAD_COND_INITIALIZER),
      alive_count_lock(PTHREAD_MUTEX_INITIALIZER),
      alive_count(0),
      flags_lock(PTHREAD_MUTEX_INITIALIZER),
      flags(0),
      threads_ready_cond(PTHREAD_COND_INITIALIZER),
      threads_ready_lock(PTHREAD_MUTEX_INITIALIZER),
      exit_uids_lock(PTHREAD_MUTEX_INITIALIZER),
      errno_lock(PTHREAD_MUTEX_INITIALIZER),
      wkr_errno(0) {

    int ret;
    

    //initialise a new vector to hold unique IDs of workers to kill
    ret = cm_new_vct(&this->exit_uids, sizeof(int));
    if (ret != 0) this->_set_ctor_failed(true);

    return;
}


sc::_worker_concurrency::~_worker_concurrency() noexcept {

    //delete exit uids vector
    _CTOR_VCT_DELETE_IF_INIT(this->exit_uids)

    return;
}


//await release by the worker pool
void sc::_worker_concurrency::wkr_release_wait() noexcept {

    pthread_mutex_lock(&this->release_count_lock);

    //increment release count
    ++this->release_count;
    
    //if all threads are waiting, set the release ready flag
    pthread_mutex_lock(&this->alive_count_lock);
    if (this->release_count == this->alive_count) {
        this->set_flags(sc::_worker_flag::release_ready);
        pthread_cond_broadcast(&this->threads_ready_cond);
    }
    pthread_mutex_unlock(&this->alive_count_lock);

    //await release by the worker pool
    pthread_cond_wait(&this->release_count_cond,
                      &this->release_count_lock);

    //decrement the release count
    --this->release_count;

    pthread_mutex_unlock(&this->release_count_lock);
    return;
}


//perform concurrency sensitive operations related to worker entry
void sc::_worker_concurrency::wkr_enter() noexcept {

    pthread_mutex_lock(&this->alive_count_lock);
    ++this->alive_count;
    pthread_mutex_unlock(&this->alive_count_lock);

    return;
}


//perform concurrency sensitive operations related to worker exit
void sc::_worker_concurrency::wkr_exit(const bool is_error) noexcept {

    pthread_mutex_lock(&this->release_count_lock);
    pthread_mutex_lock(&this->alive_count_lock);

    //decrement alive count
    --this->alive_count;

    //signal other workers to exit on error
    if (is_error == true)
        this->set_flags(sc::_worker_flag::error | sc::_worker_flag::exit);

    //if all other threads are waiting, set the release ready flag
    if (this->release_count == this->alive_count) {
        this->set_flags(sc::_worker_flag::release_ready);
        pthread_cond_broadcast(&this->threads_ready_cond);
    }

    pthread_mutex_unlock(&this->release_count_lock);
    pthread_mutex_unlock(&this->alive_count_lock);

    return;
}


//check if the worker pool requested some workers to exit
[[nodiscard]] int sc::_worker_concurrency::wkr_check_kill(
    const int uid, bool & do_exit) noexcept {

    int ret;
    int * iter_uid;

    int fn_ret = 0;

    
    pthread_mutex_lock(&this->exit_uids_lock);

    //assume no exit by default
    do_exit = false;

    //see if this thread was requested to leave
    for (int i = 0; i < this->exit_uids.len; ++i) {

        //get the next address
        iter_uid = (int *) cm_vct_get_p(&this->exit_uids, i);
        if (iter_uid == nullptr) {

            this->wkr_set_errno(SC_ERR_CMORE);
            fn_ret = -1;
            goto _wkr_check_kill_cleanup;
        }

        //if uid matches terminate this thread
        if (*iter_uid == uid) {

            do_exit = true;
            ret = cm_vct_rmv(&this->exit_uids, i);
            if (ret != 0) {
                this->wkr_set_errno(SC_ERR_CMORE);
                fn_ret = -1;
            }
            break;
        }
    }

    _wkr_check_kill_cleanup:
    pthread_mutex_unlock(&this->exit_uids_lock);

    return fn_ret;
}


//allow a worker to report an error
void sc::_worker_concurrency::wkr_set_errno(
    const int wkr_sc_errno) noexcept {

    pthread_mutex_lock(&this->errno_lock);

    //if errno is already set, do not overwrite it
    if (this->wkr_errno != 0) goto _wkr_set_errno_cleanup;

    //set a new scancry errno
    this->wkr_errno = wkr_sc_errno;

    //set the error flag    
    pthread_mutex_lock(&this->flags_lock);
    this->flags |= sc::_worker_flag::error;
    pthread_mutex_unlock(&this->flags_lock);

    _wkr_set_errno_cleanup:
    pthread_mutex_unlock(&this->errno_lock);

    return;
}


_SC_DBG_STATIC
const constexpr useconds_t _single_run_sleep_ival_nsec = 10000000;
const constexpr unsigned long _nsec_in_sec = 1000000000;
const constexpr useconds_t _release_timeout_sec = 10;

//allow a worker pool to wait for workers to ready
[[nodiscard]] int sc::_worker_concurrency::wp_await_wkrs(
    const bool do_timeout) noexcept {

    int ret;

    struct timeval cur_time;
    struct timeval timeout_time;
    struct timespec wake_time;


    //get the timeout time if requested
    if (do_timeout == true ) {
        ret = gettimeofday(&timeout_time, nullptr);
        if (ret != 0) {
            sc_errno = SC_ERR_TIMESPEC;
            return -1;
        }
        timeout_time.tv_sec += _release_timeout_sec;
    }


    //wait for workers to be released until a timeout is hit
    while (true) {

        //check if workers are ready
        pthread_mutex_lock(&this->flags_lock);
        if (this->flags & sc::_worker_flag::release_ready) {
            pthread_mutex_unlock(&this->flags_lock);
            return 0;
        }
        pthread_mutex_unlock(&this->flags_lock);

        //check if an error occurred
        pthread_mutex_lock(&this->errno_lock);
        if (this->flags & sc::_worker_flag::error) {
            sc_errno = this->wkr_errno;
            pthread_mutex_unlock(&this->errno_lock);
            return -1;
        }
        pthread_mutex_unlock(&this->errno_lock);

        //get time of day
        ret = gettimeofday(&cur_time, nullptr);
        if (ret != 0) {
            sc_errno = SC_ERR_TIMESPEC;
            return -1;
        }

        //exit if timeout exceeded
        if (do_timeout == true) {
            if ((cur_time.tv_sec > timeout_time.tv_sec)
                || ((cur_time.tv_sec == timeout_time.tv_sec)
                    && (cur_time.tv_usec > timeout_time.tv_sec))) {
                sc_errno = SC_ERR_WORKER_TIMEOUT;
                return -1;
            }
        }

        //calculate absolute wake time
        wake_time.tv_sec = cur_time.tv_sec;
        wake_time.tv_nsec = cur_time.tv_usec * 1000
                            + _single_run_sleep_ival_nsec;
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wsign-compare"
        if (wake_time.tv_nsec >= _nsec_in_sec) {
        #pragma GCC diagnostic pop
            wake_time.tv_sec += wake_time.tv_nsec / _nsec_in_sec;
            wake_time.tv_nsec = wake_time.tv_nsec % _nsec_in_sec;
        }

        //wait for workers to ready
        pthread_mutex_lock(&this->threads_ready_lock);
        pthread_cond_timedwait(&this->threads_ready_cond,
                               &this->threads_ready_lock, &wake_time);
        pthread_mutex_unlock(&this->threads_ready_lock);
    }
}


//allow a worker pool to release workers
void sc::_worker_concurrency::wp_release_wkrs() noexcept {

    //clear the release flag
    this->unset_flags(sc::_worker_flag::release_ready);

    //broadcast a worker release
    pthread_mutex_lock(&this->release_count_lock);
    pthread_cond_broadcast(&this->release_count_cond);
    pthread_mutex_unlock(&this->release_count_lock);

    return;
}


//allow a worker pool to kill N workers
[[nodiscard]] int
    sc::_worker_concurrency::wp_wkr_kill(const int uid) noexcept {

    int ret;
    int fn_ret = 0;
    

    pthread_mutex_lock(&this->exit_uids_lock);

    //add unique ID to the list of workers to kill
    ret = cm_vct_apd(&this->exit_uids, &uid);
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        fn_ret = -1;
    }

    pthread_mutex_unlock(&this->exit_uids_lock);
    return fn_ret;
}


//reset the worker kill list
void sc::_worker_concurrency::wp_wkr_kill_reset() noexcept {

    pthread_mutex_lock(&this->exit_uids_lock);

    //remove all uniqie IDs from the list of workers to kill
    cm_vct_emp(&this->exit_uids);

    pthread_mutex_unlock(&this->exit_uids_lock);
}


//allow a worker pool to reset the error state
void sc::_worker_concurrency::wp_reset_error() noexcept {

    pthread_mutex_lock(&this->errno_lock);
    pthread_mutex_lock(&this->flags_lock);

    //reset errno & error flag
    this->wkr_errno = 0;
    this->flags &= ~sc::_worker_flag::error;

    pthread_mutex_unlock(&this->flags_lock);
    pthread_mutex_unlock(&this->errno_lock);

    return;
}


//fetch the current scancry errno
[[nodiscard]] int sc::_worker_concurrency::get_errno() noexcept {

    int concur_errno;

    pthread_mutex_lock(&this->errno_lock);
    concur_errno = this->wkr_errno;
    pthread_mutex_unlock(&this->errno_lock);

    return concur_errno;
}


//enable given flags
void sc::_worker_concurrency::set_flags(const cm_byte bitmask) noexcept {

    pthread_mutex_lock(&this->flags_lock);
    this->flags |= bitmask;
    pthread_mutex_unlock(&this->flags_lock);

    return;
}


//disable given flags
void sc::_worker_concurrency::unset_flags(const cm_byte bitmask) noexcept {

    pthread_mutex_lock(&this->flags_lock);
    this->flags &= ~bitmask;
    pthread_mutex_unlock(&this->flags_lock);
}


//get current flags
[[nodiscard]] cm_byte sc::_worker_concurrency::get_flags() const noexcept {

    pthread_mutex_lock(&this->flags_lock);
    cm_byte flags = this->flags;
    pthread_mutex_unlock(&this->flags_lock);

    return flags;
}



/*
 *  --- [WORKER_POOL_CACHE | INTERNAL] ---
 */

//dtor
sc::_worker_pool_cache::~_worker_pool_cache() noexcept {

    if (this->is_locked == true) this->unlock();
    return;
}


//lock & unlock cache
[[nodiscard]] int sc::_worker_pool_cache::lock() noexcept {

    int ret;

    int iter;
    bool is_err = false;

    const sc::_lockable * lockables[3] = {
        this->opts,
        this->opts_scan,
        this->scan
    };


    //for all cached objects to lock
    for (iter = 0; iter < 3; ++iter) {
        ret = lockables[iter]->_lock_read();
        if (ret != 0) { is_err = true; break; }
    }

    //unlock locked objects on error
    if (is_err == false) { this->is_locked = true; return 0; }
    for (int i = 0; i < iter; ++i) {
        lockables[i]->_unlock();
    }

    return -1;
}


void sc::_worker_pool_cache::unlock() noexcept {

    const sc::_lockable * lockables[3] = {
        this->opts,
        this->opts_scan,
        this->scan
    };


    //exit pre-emptively if not locked
    if (this->is_locked == false) return;

    //for all cached objects
    for (int i = 0; i < 3; ++i) {
        lockables[i]->_unlock();
    }

    //mark cache as unlocked
    this->is_locked = false;

    return;
}


//getters
[[nodiscard]] const sc::opt *
    sc::_worker_pool_cache::get_opts() const noexcept {

    return this->opts;
}

[[nodiscard]] const sc::_opt_scan *
    sc::_worker_pool_cache::get_opts_scan() const noexcept {

    return this->opts_scan;
}

[[nodiscard]] sc::_scan *
    sc::_worker_pool_cache::get_scan() const noexcept {

    return this->scan;
}



/*
 *  --- [WORKER | INTERNAL] ---
 */

//worker globals
static int wkr_next_uid = 0;

void * _bootstrap_worker(void * arg) {

    //typecast worker
    sc::_worker * worker = (sc::_worker *) arg;

    //call main
    worker->main();

    return nullptr;
}


//ctor & dtor
sc::_worker::_worker(
    const struct sc::_worker_pool_cache & pool_cache,
    struct sc::_worker_concurrency & concur,
    const cm_vct /* <const cm_lst_node *> */ & scan_area_subset,
    const int session_idx) noexcept
    : _ctor_failable(),
      uid(wkr_next_uid),
      scan_area_subset(scan_area_subset),
      session_idx(session_idx),
      pool_cache(pool_cache),
      concur(concur) {

    long page_size;


    //increment next worker ID
    ++wkr_next_uid;

    /*
     *  NOTE: '_SC_' prefix here stands for 'sysconf', not 'scancry'.
     */

    //find the page size
    page_size = sysconf(_SC_PAGESIZE);
    if (page_size < 0) {
        sc_errno = SC_ERR_PAGESIZE;
        this->_set_ctor_failed(true);
        return;
    }

    //acquire a read buffer
    this->buf = (cm_byte *) std::malloc(page_size);
    if (this->buf == NULL) {
        sc_errno = SC_ERR_MEM;
        this->_set_ctor_failed(true);
    }

    return;
}


sc::_worker::~_worker() noexcept {

    std::free(this->buf);
    return;
}


[[nodiscard]] _SC_DBG_INLINE int
    sc::_worker::read_buf_smart(_scan_arg & arg) noexcept {

    int ret;

    ssize_t area_sz     = 0;
    ssize_t left_sz     = 0;
    ssize_t read_sz     = 0;
    ssize_t buf_real_sz = 0;
    ssize_t buf_off     = 0;
    sc::addr_width addr_w = sc::ADDR_WIDTH_UNSET;
    
    mc_vm_area * area;
    off_t addr_off;

    #ifdef SC_TRACE_WORKER
        mc_vm_obj * _trace_obj;
    #endif


    //fetch this area
    area = MC_GET_NODE_AREA(arg.get_area_node());

    #ifdef SC_TRACE_WORKER
    //log object & area starting address of current buffer read
    _trace_obj = nullptr;
    if (area->obj_node_p != nullptr)
        _trace_obj = MC_GET_NODE_OBJ(area->obj_node_p);
    dbg::print_trace("buffer from: %s - 0x%lx\n",
                     (_trace_obj == nullptr)
                     ? "N/A" : _trace_obj->basename, area->start_addr);
    #endif

    /*
     *  NOTE: The read buffer is one page in size; the minimum size of
     *        a vm_area is also one page. We can therefore attempt to
     *        read the maximum amount on a fresh area.
     */

    //read from beginning
    if (arg.get_area_off() == 0) {

        read_sz  = this->cached_session->page_size;
        buf_off  = 0;
        addr_off = 0;

    //continue reading
    } else {

        //cache address width
        ret = this->pool_cache.get_opts()->get_addr_width(addr_w);
        if (ret != 0) return -1;

        //calculate relevant sizes & offsets
        area_sz     = area->end_addr - area->start_addr;
        left_sz     = area_sz - arg.get_area_off() - (int) addr_w;
        buf_real_sz = this->cached_session->page_size - (int) addr_w;

        //copy the end of the buffer to the beginning
        std::memcpy(this->buf,
                    this->buf + buf_real_sz, 
                    (int) addr_w);

        //clamp read size between some minimum and the real buffer size
        read_sz = cm_clamp(left_sz, 0, buf_real_sz);
        buf_off = (ssize_t) addr_w;
        addr_off = (ssize_t) addr_w;
    }
    
    #ifdef SC_TRACE_WORKER
    //log buffer reading parameters
    dbg::print_trace("  - area_sz:     0x%lx\n", area_sz);
    dbg::print_trace("  - area_off:    0x%lx\n", arg.area_off);
    dbg::print_trace("  - left_sz:     0x%lx\n", left_sz);
    dbg::print_trace("  - buf_real_sz: 0x%lx\n", buf_real_sz);
    dbg::print_trace("  - read_sz:     0x%lx\n", read_sz);
    #endif

    //perform the read
    ret = mc_read(this->cached_session, arg.get_addr() + addr_off,
                  this->buf + buf_off, read_sz);
    if (ret != 0) {
        return -1;
    }

    //reset `_scan_arg` state related to the read buffer
    arg.reset_buf(this->cached_session->page_size, this->buf);

    return 0;
}


void sc::_worker::main() noexcept {

    int ret;
    off_t buf_adv;
    bool do_exit;

    cm_lst_node * area_node;
    mc_vm_area * area;

    sc::addr_width addr_width;

    #ifdef SC_TRACE_WORKER
    int _trace_iter;
    mc_vm_obj * _trace_obj;
    #endif
    

    //notify concurrency object of a new worker
    this->concur.wkr_enter();

    //fetch address width
    ret = this->pool_cache.get_opts()->get_addr_width(addr_width);
    if (ret != 0) {
        this->concur.wkr_set_errno(sc_errno); //sc_errno is thread local
        this->concur.wkr_exit(true);
        return;
    }


    //repeatedly perform requested scans
    while (true) {

        //allow control runs, which do not perform a scan
        do {
        
            //await release
            this->concur.wkr_release_wait();

            #ifdef SC_TRACE_WORKER
            dbg::print_trace("[worker %d] released\n", this->uid);
            #endif


            // - exit if requested

            //check if this thread was asked to exit
            ret = this->concur.wkr_check_kill(this->uid, do_exit);
            if (ret != 0) {
                this->concur.wkr_exit(true);
                return;
            }

            //exit if requested
            if ((do_exit == true)
                || ((this->concur.get_flags()
                    & sc::_worker_flag::exit) == true)) {

                this->concur.wkr_exit(false);
                return;
            }

        } while ((this->concur.get_flags()
                 & sc::_worker_flag::ctrl_run) != 0);


        //cache memcry session
        ret = cm_vct_get(&this->pool_cache.get_opts()->get_sessions(),
                         this->session_idx, &this->cached_session);
        if (ret != 0) {
            this->concur.wkr_set_errno(SC_ERR_CMORE);
            this->concur.wkr_exit(true);
            return;
        }

        #ifdef SC_TRACE_WORKER
        //log scan set
        dbg::print_trace("[worker %d] scan set size: %d",
                         this->uid, this->scan_area_subset.len);
        #endif


        //for every area in the scan set
        for (int i = 0; i < this->scan_area_subset.len; ++i) {

            //fetch the next area
            ret = cm_vct_get(&this->scan_area_subset, i, &area_node);
            if (ret != 0) {
                this->concur.wkr_set_errno(SC_ERR_CMORE);
                this->concur.wkr_exit(true);
                return;
            }
            area = MC_GET_NODE_AREA(area_node);

            #ifdef SC_TRACE_WORKER
            //log area
            if (area->obj_node_p != nullptr)
                _trace_obj = MC_GET_NODE_OBJ(area_node);
            dbg::print_trace("[worker %d] scan area: %s - 0x%lx\n",
                             area->obj_node_p == nullptr ? "<anon>"
                             : _trace_obj->basename);
            #endif

            //create a new scan arg
            sc::_scan_arg scan_arg(area->start_addr, area_node,
                                   0x0, nullptr, 0x0);


            //process every address in this area
            while (scan_arg.get_addr() < area->end_addr) {

                //if current buffer has run out, fetch the next one
                if (scan_arg.get_buf_left() <= (size_t) addr_width)
                    ret = this->read_buf_smart(scan_arg);

                //send address to the scanner
                buf_adv = this->pool_cache.get_scan()->_process_addr(
                    scan_arg,
                    *this->pool_cache.get_opts(),
                    *this->pool_cache.get_opts_scan());
                if (buf_adv == -1) {
                    this->concur.wkr_set_errno(sc_errno);
                    this->concur.wkr_exit(true);
                    return;
                }

                //advance buffer
                scan_arg.advance_buf(buf_adv);

            } //end process every address in this area

        } //end for every area in the scan set

    } //end repeatedly perform requested scans
}


[[nodiscard]] int sc::_worker::get_uid() noexcept {
    return this->uid;
}



/*
 *  --- [WORKER_BUNDLE | INTERNAL] ---
 */

//ctor & dtor
sc::_worker_bundle::_worker_bundle(
    const struct sc::_worker_pool_cache & pool_cache,
    sc::_worker_concurrency & concur,
    const int session_idx) noexcept
    : _ctor_failable(),
      wkr(pool_cache, concur, scan_area_subset, session_idx) {

    int ret;


    //abort early if worker constructor failed
    if (this->wkr.get_ctor_failed() == true) {
        this->_set_ctor_failed(true);
        return;
    }

    //create a new scan area subset vector
    ret = cm_new_vct(&this->scan_area_subset, sizeof(cm_lst_node *));
    if (ret != 0) {
        this->_set_ctor_failed(true);
        return;
    }

    //start a new thread
    ret = pthread_create(&this->thread_id, nullptr,
                         _bootstrap_worker, &wkr);
    if (ret != 0) {
        cm_del_vct(&this->scan_area_subset);
        this->_set_ctor_failed(true);
        return;
    }

    return;
}


sc::_worker_bundle::~_worker_bundle() noexcept {

    //if a thread was never started
    if (this->get_ctor_failed() == true) return;

    //join the worker thread
    pthread_join(this->thread_id, nullptr);

    //destroy the scan area subset
    cm_del_vct(&this->scan_area_subset);

    return;
}


//getters
[[nodiscard]] int sc::_worker_bundle::get_wkr_uid() noexcept {
    return this->wkr.get_uid();
}


[[nodiscard]] cm_vct & sc::_worker_bundle::get_scan_area_subset() noexcept {
    return this->scan_area_subset;
}



/*
 *  --- [WORKER_POOL | PRIVATE] ---
 */

//signal a single run
[[nodiscard]] int sc::worker_pool::do_run() noexcept {

    int ret;


    //wait for workers to be ready
    ret = this->concur.wp_await_wkrs(false);
    if (ret != 0) return -1;

    //perform a run
    this->concur.wp_release_wkrs();

    //check for errors
    if (this->concur.get_flags() & sc::_worker_flag::error) {
        sc_errno = this->concur.get_errno();
        return -1;
    }

    return 0;
}


//await a single run to finish
[[nodiscard]] int sc::worker_pool::await_run() noexcept {

    int ret;


    //wait for workers to be ready
    ret = this->concur.wp_await_wkrs(false);
    if (ret != 0) return -1;

    return 0;
}


//perform a control run to manage workers
[[nodiscard]] int sc::worker_pool::do_ctrl_run() noexcept {

    int ret;


    //setup a control run
    this->concur.set_flags(sc::_worker_flag::ctrl_run);

    //perform a control run
    ret = this->do_run();
    if (ret != 0) return -1;

    return 0;    
}


//perform a scan run
[[nodiscard]] int sc::worker_pool::do_scan_run() noexcept {

    int ret;


    //setup a scan run
    this->concur.unset_flags(sc::_worker_flag::ctrl_run);

    //perform a scan run
    ret = this->do_run();
    if (ret != 0) return -1;

    return 0;    
}


/*
 *  NOTE: Returns `1` if there is a change in worker count,
 *        `0` if there is no change, and `-1` on error.
 */

[[nodiscard]] int
    sc::worker_pool::change_wkr_count(const int count) noexcept {

    int ret;

    int diff;
    int iter_lim;
    
    cm_lst_node * wkr_bndl_node, * rmv_wkr_bndl_node;

    sc::_worker_bundle * wkr_bndl;
    cm_byte wkr_bndl_stub[sizeof(sc::_worker_bundle)] = {0};
    int session_idx;


    //if already have `count` workers, just return
    diff = count - this->wkr_bundles.len;
    if ((this->wkr_bundles.len - count) == 0) return 0;

    //if reducing the worker count is required
    if (diff < 0) {

        // - mark workers to terminate

        //reset the worker kill list
        this->concur.wp_wkr_kill_reset();

        //initialise iteration
        wkr_bndl_node = this->wkr_bundles.head->prev == nullptr
                        ? this->wkr_bundles.head
                        : this->wkr_bundles.head->prev;

        //for all workers that must be stopped
        iter_lim = this->wkr_bundles.len - 1 + diff;
        for (int i = this->wkr_bundles.len - 1; i > iter_lim; --i) {

            //fetch this worker bundle
            wkr_bndl = _SC_GET_NODE_WKR_BUNDLE(wkr_bndl_node);

            //ask this worker to terminate
            ret = this->concur.wp_wkr_kill(wkr_bndl->get_wkr_uid());
            if (ret != 0) return -1; 

            //update iteration
            wkr_bndl_node = wkr_bndl_node->prev;
        }

        // - run a control run
        ret = this->do_ctrl_run();
        if (ret != 0) return -1;

        ret = this->concur.wp_await_wkrs(true);
        if (ret != 0) return -1;

        // - cleanup workers

        //re-initialise iteration
        wkr_bndl_node = this->wkr_bundles.head->prev == nullptr
                        ? this->wkr_bundles.head
                        : this->wkr_bundles.head->prev;

        //for all workers that must be stopped
        iter_lim = this->wkr_bundles.len - 1 + diff;
        for (int i = this->wkr_bundles.len - 1; i > iter_lim; --i) {

            //fetch this worker bundle
            wkr_bndl = _SC_GET_NODE_WKR_BUNDLE(wkr_bndl_node);

            //destroy this worker
            wkr_bndl->~_worker_bundle();

            //remove list node & update iteration
            rmv_wkr_bndl_node = wkr_bndl_node;
            wkr_bndl_node = wkr_bndl_node->prev;
            cm_lst_rmv_n(&this->wkr_bundles, rmv_wkr_bndl_node);
        }
    }

    //if increasing the worker count is required
    if (diff > 0) {

        //check there are enough sessions for the new threads
        if (this->cache.get_opts()->get_sessions().len < count) {
            sc_errno = SC_ERR_OPT_MISSING;
            return -1;
        }

        //for all new requested workers
        for (int i = 0; i < diff; ++i) {

            //find the index of the session for this worker
            session_idx = this->wkr_bundles.len;

            //create a new list node for the worker
            wkr_bndl_node = cm_lst_apd(&this->wkr_bundles, wkr_bndl_stub);
            if (wkr_bndl_node == nullptr) {
                sc_errno = SC_ERR_CMORE;
                return -1;
            }

            //construct the worker in the new node
            wkr_bndl = new (wkr_bndl_node->data) sc::_worker_bundle(
                this->cache, this->concur, session_idx);
        }
    }

    return (diff != 0) ? 1 : 0;
}


[[nodiscard]] int sc::worker_pool::cache_areas(
    const sc::map_area_set & ma_set) noexcept {

    int ret;


    //destroy any existing cached areas
    common::del_vct_if_init(this->sorted_areas_cache);

    //convert the set (red-black tree) to a sorted vector
    ret = ma_set.to_size_ord_vct(this->sorted_areas_cache);
    if (ret != 0) return -1;

    return 0;
}


[[nodiscard]] int sc::worker_pool::distrib_areas() noexcept {

    int ret;

    cm_vct sums;
    size_t sum;
    size_t * sum_p;
    size_t min;
    int min_idx;
    
    cm_lst_node * area_node;
    mc_vm_area * area;
    
    sc::_worker_bundle * wkr_bundle;
    

    //create a vector to store size sums for each worker
    ret = cm_new_vct(&sums, sizeof(size_t));
    if (ret != 0) { sc_errno = SC_ERR_CMORE; return -1; }
    ret = cm_vct_rsz(&sums, this->wkr_bundles.len);
    if (ret != 0) { sc_errno = SC_ERR_CMORE; return -1; }
    std::memset(sums.data, 0, sums.data_sz * sums.len);

    //for all cached areas
    for (int i = 0; i < this->sorted_areas_cache.len; ++i) {

        //get next cached area
        ret = cm_vct_get(&this->sorted_areas_cache, i, &area_node);
        if (ret != 0) {
            sc_errno = SC_ERR_CMORE; goto _distrib_areas_fail; }
        area = MC_GET_NODE_AREA(area_node);

        //for all sums
        min = SIZE_MAX;
        for (int j = 0; j < sums.len; ++j) {

            //fetch next sum
            ret = cm_vct_get(&sums, j, &sum);
            if (ret != 0) {
                sc_errno = SC_ERR_CMORE; goto _distrib_areas_fail; }

            //update sum
            if (sum < min) { min = sum; min_idx = j; }

        } //end for all sums


        // - add this area to the smallest sum

        //increment the sum
        sum_p = (size_t *) cm_vct_get_p(&sums, min_idx);
        if (sum_p == nullptr) {
            sc_errno = SC_ERR_CMORE; goto _distrib_areas_fail; }
        *sum_p += (area->end_addr - area->start_addr);

        //fetch relevant worker bundle
        wkr_bundle = (sc::_worker_bundle *)
                         cm_lst_get_p(&this->wkr_bundles, min_idx);
        if (wkr_bundle == nullptr) {
            sc_errno = SC_ERR_CMORE; goto _distrib_areas_fail; }

        //add this area to this worker bundle
        cm_vct & wkr_areas = wkr_bundle->get_scan_area_subset();
        ret = cm_vct_apd(&wkr_areas, &area_node);
        if (ret != 0) {
            sc_errno = SC_ERR_CMORE; goto _distrib_areas_fail; }

    } //end for all cached areas

    cm_del_vct(&sums);
    return 0;

    //cleanup
    _distrib_areas_fail:
    cm_del_vct(&sums);
    return -1;
}


/*
 *  --- [WORKER_POOL | INTERNAL] ---
 */

//entry & exit functions for scans
[[nodiscard]] int sc::worker_pool::_setup(
    const sc::opt & opts,
    const sc::_opt_scan & opts_scan,
    sc::_scan & scan,
    const cm_byte flags) noexcept {

    int ret;
    int do_distrib;

    const sc::map_area_set * scan_set;


    //write lock the worker pool
    ret = this->_lock_write();
    if (ret != 0) return -1;

    //setup cache
    this->cache.~_worker_pool_cache();
    new (&this->cache) sc::_worker_pool_cache(&opts, &opts_scan, &scan);
    ret = this->cache.lock();
    if (ret != 0) goto _setup_fail_1;

    //update workers
    ret = this->change_wkr_count(
              this->cache.get_opts()->get_sessions().len);
    if (ret < 0) goto _setup_fail_2;
    do_distrib = ret;

    //re-cache the map area set unless explicitly skipped    
    if ((flags & sc::bits_worker::keep_scan_set) == false) {

        //fetch the scan set
        scan_set = opts.get_scan_set();
        if (scan_set == nullptr) {
            sc_errno = SC_ERR_OPT_MISSING;
            goto _setup_fail_2;
        }

        //cache the scan set
        ret = this->cache_areas(*scan_set);
        if (ret != 0) goto _setup_fail_2;
    }

    //re-distribute the map area set if the scan set has changed
    //OR if the number of workers has changed
    if (((flags & sc::bits_worker::keep_scan_set) == false)
         || (do_distrib == 1)) {

        ret = this->distrib_areas();
        if (ret != 0) goto _setup_fail_2;
    }

    return 0;

    //cleanup
    _setup_fail_2:
    this->cache.unlock();

    _setup_fail_1:
    this->_unlock();

    return -1;
}


void sc::worker_pool::_teardown() noexcept {

    //unlock cache
    this->cache.unlock();

    //unlock read lock
    this->_unlock();
    
    return;
}


//dispatch a single pass over the scan set
[[nodiscard]] int sc::worker_pool::_single_run() noexcept {

    int ret;


    //acquire a read lock
    ret = this->_lock_read();
    if (ret != 0) return -1;

    //signal workers for a single pass
    ret = this->do_scan_run();
    if (ret != 0) { this->_unlock(); return -1; }

    this->_unlock();
    return 0;
}


//await for a single pass over the scan set to finish
[[nodiscard]] int sc::worker_pool::_await_run() noexcept {

    int ret;


    //acquire a read lock
    ret = this->_lock_read();
    if (ret != 0) return -1;

    //await for the workers to be ready
    ret = this->await_run();
    if (ret != 0) { this->_unlock(); return -1; }

    this->_unlock();
    return 0;
}


//cancel a running single pass
[[nodiscard]] int sc::worker_pool::_cancel() noexcept {

    int ret;


    //acquire a read lock
    ret = this->_lock_read();
    if (ret != 0) return -1;

    //set the cancel flag
    this->concur.set_flags(sc::_worker_flag::cancel);

    this->_unlock();
    return 0;
}


/*
 *  --- [WORKER_POOL | PUBLIC] ---
 */

//ctor & dtor
sc::worker_pool::worker_pool() noexcept
    : _ctor_failable(),
      cache(nullptr, nullptr, nullptr),
      concur() {

    //check if concurrency constructor failed
    if (this->concur.get_ctor_failed()) {
        this->_set_ctor_failed(true);
        return;
    }

    //initialise a new worker bundles list
    cm_new_lst(&this->wkr_bundles, sizeof(sc::_worker_bundle));

    //zero out the sorted areas cache
    std::memset(&this->sorted_areas_cache,
                0, sizeof(this->sorted_areas_cache));
    return;
}


sc::worker_pool::~worker_pool() noexcept {

    cm_lst_node * wkr_node;
    sc::_worker_bundle * wkr;


    //destroy sorted scan areas
    cm_del_vct(&this->sorted_areas_cache);

    //if the worker bundles list is initialised
    if (this->wkr_bundles.is_init == true) {

        //destroy all worker bundles
        wkr_node = this->wkr_bundles.head;
        for (int i = 0; i < this->wkr_bundles.len; ++i) {
            wkr = _SC_GET_NODE_WKR_BUNDLE(wkr_node);
            wkr->~_worker_bundle();
        }
        cm_del_lst(&this->wkr_bundles);
    }

    return;
}


//reset - kill all workers & remove cache
[[nodiscard]] int sc::worker_pool::reset() noexcept {

    int ret;


    //acquire a write lock
    ret = this->_lock_write();
    if (ret != 0) return -1;

    //kill workers
    ret = this->change_wkr_count(0);
    if (ret < 0) { this->_unlock(); return -1; }

    //empty scan set cache
    cm_vct_emp(&this->sorted_areas_cache);
    
    this->_unlock();
    return 0;
}



      /* ============= * 
 ===== *  C INTERFACE  * =====
       * ============= */

/*
 *  --- [WORKER POOL | EXTERNAL] ---
 */

//ctor & dtor
_DEFINE_C_CTOR(worker_pool, w_pool, sc);
_DEFINE_C_DTOR(worker_pool, w_pool, sc, w_pool);


int sc_wp_reset(sc_worker_pool * w_pool) {

    int ret;


    sc::worker_pool * cc_w_pool = (sc::worker_pool *) w_pool;
    ret = cc_w_pool->reset();
    if (ret != 0) return -1;

    return 0;    
}
