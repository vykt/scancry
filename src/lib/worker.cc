//C standard library
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


//increment the alive count
void sc::_worker_concurrency::wkr_enter() noexcept {

    pthread_mutex_lock(&this->alive_count_lock);
    ++this->alive_count;
    pthread_mutex_unlock(&this->alive_count_lock);

    return;
}


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

[[nodiscard]] _SC_DBG_INLINE int
    sc::_worker::read_buffer_smart(_scan_arg & arg) noexcept {

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
    arg.reset_buffer(this->session->page_size, this->buf);

    return 0;
}
