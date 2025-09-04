//C standard library
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <ctime>

//system headers
#include <unistd.h>

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry.h"
#include "scancry_impl.h"
#include "error.hh"

//debug headers
#ifdef SC_TRACE
#include "debug.hh"
#endif



/*
 *  --- [INTERNAL | WORKER_CONCURRENCY] ---
 */

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


//allow a worker to report an error
void sc::_worker_concurrency::wkr_set_errno(const int errno) noexcept {

    pthread_mutex_lock(&this->errno_lock);

    //if errno is already set, do not overwrite it
    if (this->wkr_errno != 0) goto _wkr_set_errno_cleanup;

    //set a new scancry errno
    this->wkr_errno = errno;

    //set the error flag    
    pthread_mutex_lock(&this->flags_lock);
    this->flags |= sc::_worker_flag::error;
    pthread_mutex_unlock(&this->flags_lock);

    _wkr_set_errno_cleanup:
    pthread_mutex_unlock(&this->errno_lock);

    return;
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

    int errno;

    pthread_mutex_lock(&this->errno_lock);
    errno = this->wkr_errno;
    pthread_mutex_unlock(&this->errno_lock);

    return errno;
}


//enable given flags
void sc::_worker_concurrency::set_flags(const cm_byte bitmask) noexcept {

    pthread_mutex_lock(&this->flags_lock);
    this->flags |= bitmask;
    pthread_mutex_unlock(&this->flags_lock);

    return;
}


//get current flags
[[nodiscard]] cm_byte sc::_worker_concurrency::get_flags() const noexcept {

    pthread_mutex_lock(&this->flags_lock);
    cm_byte flags = this->flags;
    pthread_mutex_unlock(&this->flags_lock);

    return flags;
}



/*
 *  --- [INTERNAL | WORKER_POOL_CACHE] ---
 */

//getters & setters
void sc::_worker_pool_cache::set_opts(const sc::opt * opts) noexcept {
    this->opts = opts;
    return;
}

[[nodiscard]] const sc::opt *
    sc::_worker_pool_cache::get_opts() const noexcept {

    return this->opts;
}

void sc::_worker_pool_cache::set_opts_scan(
    const sc::_opt_scan * opts_scan) noexcept {

    this->opts_scan = opts_scan;
    return;
}

[[nodiscard]] const sc::_opt_scan *
    sc::_worker_pool_cache::get_opts_scan() const noexcept {

    return this->opts_scan;
}


void sc::_worker_pool_cache::set_scan(const sc::_scan * scan) noexcept {
    this->scan = (sc::_scan *) scan;
    return;
}

[[nodiscard]] sc::_scan *
    sc::_worker_pool_cache::get_scan() const noexcept {

    return this->scan;
}



/*
 *  --- [INTERNAL | WORKER] ---
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
    const mc_session *& session) noexcept
    : _ctor_failable(),
      uid(wkr_next_uid),
      scan_area_subset(scan_area_subset),
      pool_cache(pool_cache),
      concur(concur) {

    //increment next worker ID
    ++wkr_next_uid;

    //acquire a read buffer
    this->buf = (cm_byte *) std::malloc(this->session->page_size);
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
    namespace _trace {
        mc_vm_obj * obj;
    }
    #endif


    //fetch this area
    area = MC_GET_NODE_AREA(arg.get_area_node());

    #ifdef SC_TRACE_WORKER
    //log object & area starting address of current buffer read
    _trace::obj = nullptr;
    if (area->obj_node_p != nullptr)
        _trace::obj = MC_GET_NODE_OBJ(area->obj_node_p);
    dbg::print_trace("buffer from: %s - 0x%lx\n",
                     (_trace::obj == nullptr)
                     ? "N/A" : _trace::obj->basename, area->start_addr);
    #endif

    /*
     *  NOTE: The read buffer is one page in size; the minimum size of
     *        a vm_area is also one page. We can therefore attempt to
     *        read the maximum amount on a fresh area.
     */

    //read from beginning
    if (arg.get_area_off() == 0) {

        read_sz = this->session->page_size;
        buf_off = 0;
        addr_off = 0;

    //continue reading
    } else {

        //cache address width
        ret = this->pool_cache.get_opts()->get_addr_width(addr_w);
        if (ret != 0) return -1;

        //calculate relevant sizes & offsets
        area_sz     = area->end_addr - area->start_addr;
        left_sz     = area_sz - arg.get_area_off() - (int) addr_w;
        buf_real_sz = this->session->page_size - (int) addr_w;

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
    ret = mc_read(this->session, arg.get_addr() + addr_off,
                  this->buf + buf_off, read_sz);
    if (ret != 0) {
        return -1;
    }

    //reset `_scan_arg` state related to the read buffer
    arg.reset_buf(this->session->page_size, this->buf);

    return 0;
}


void sc::_worker::main() noexcept {

    int ret;
    off_t buf_adv;

    cm_lst_node * area_node;
    mc_vm_area * area;

    sc::addr_width addr_width;

    #ifdef SC_TRACE_WORKER
    namespace trace {
        int iter;
        mc_vm_obj * obj;
    }
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
        
        //await release
        this->concur.wkr_release_wait();

        #ifdef SC_TRACE_WORKER
        dbg::print_trace("[worker %d] released\n", this->uid);
        #endif

        //exit if requested
        if ((this->concur.get_flags() & sc::_worker_flag::exit) == true) {
            this->concur.wkr_exit(false);
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
                _trace::obj = MC_GET_NODE_OBJ(area_node);
            dbg::print_trace("[worker %d] scan area: %s - 0x%lx\n",
                             area->obj_node_p == nullptr ? "<anon>"
                             : _trace::obj->basename);
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
