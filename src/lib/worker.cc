//standard template library
#include <optional>
#include <type_traits>
#include <vector>
#include <unordered_set>
#include <algorithm>

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


#define RET_PTHREAD_ERR { sc_errno = SC_ERR_PTHREAD; return std::nullopt; }


/*
 *  NOTE: C-style `void *` were chosen to pass type ambiguous parameters
 *        instead of using templates or RTTI-related methods.
 *
 */


/*
 *  --- [INTERNAL] ---
 */
void * _bootstrap_worker(void * arg) {

    //typecast worker
    sc::_worker * worker = (sc::_worker *) arg;

    //call into main
    worker->main();

    return nullptr;
}


/*
 *  --- [WORKER | PRIVATE] ---
 */

_SC_DBG_INLINE std::optional<int>
sc::_worker::read_buffer_smart(struct _scan_arg & arg) noexcept {

    int ret;

    ssize_t read_sz, area_sz, left_sz, buf_real_sz, buf_off, buf_min_sz = 0;
    mc_vm_area * area;


    //fetch this area
    area = MC_GET_NODE_AREA(arg.area_node);

    /*
     *  The read buffer is one page in size; the minimum size of a vm_area
     *  is also one page. We can therefore attempt to read the maximum
     *  amount on a fresh area.
     */

    //read from beginning
    if (arg.area_off == 0) {

        read_sz = this->session->page_size;
        buf_off = 0;

    //continue reading
    } else {

        //calculate relevant sizes & offsets
        area_sz     = area->end_addr - area->end_addr;
        left_sz     = area_sz - arg.area_off;
        buf_real_sz = this->session->page_size - (*this->opts)->addr_width;

        //copy the end of the buffer to the beginning
        std::memcpy(this->buf,
                    this->buf + buf_real_sz, (*this->opts)->addr_width);

        //clamp read size between some minimum and the real buffer size
        read_sz = std::clamp(left_sz, buf_min_sz, buf_real_sz);
        buf_off = (*this->opts)->addr_width;
    }

    //perform the read
    ret = mc_read(this->session, arg.addr, this->buf + buf_off, read_sz);
    if (ret != 0) {
        sc_errno = SC_ERR_MEMCRY;
        return std::nullopt;
    }

    //reset `_scan_arg` state related to the read buffer
    arg.buf_left = this->session->page_size;
    arg.cur_byte = this->buf;

    return 0;
}


std::optional<int> sc::_worker::release_wait() noexcept {

    int ret;


    //lock waiting count
    ret = pthread_mutex_lock(&this->concur.release_count_lock);
    if (ret != 0) RET_PTHREAD_ERR

    //increment waiting count
    ++this->concur.release_count;

    //if all threads are now waiting
    if (this->concur.release_count == this->concur.alive_count) {

        //lock flags
        ret = pthread_mutex_lock(&this->concur.flags_lock);
        if (ret != 0) RET_PTHREAD_ERR

        //set the workers ready flag
        this->concur.flags |= _worker_flag_release_ready;

        //unlock flags
        ret = pthread_mutex_unlock(&this->concur.flags_lock);
        if (ret != 0) RET_PTHREAD_ERR
    
    }

    //wait for manager's signal
    ret = pthread_cond_wait(&this->concur.release_count_cond,
                            &this->concur.release_count_lock);
    if (ret != 0) RET_PTHREAD_ERR

    return 0;
}


std::optional<int> sc::_worker::exit() noexcept {

    int ret;


    //lock the alive count    
    ret = pthread_mutex_lock(&this->concur.alive_count_lock);
    if (ret != 0) RET_PTHREAD_ERR

    //lock the waiting count
    ret = pthread_mutex_lock(&this->concur.release_count_lock);
    if (ret != 0) RET_PTHREAD_ERR

    //decrement alive count
    --this->concur.alive_count;

    //set the workers ready flag if other threads are waiting
    if (this->concur.alive_count == this->concur.release_count) {
        
        //lock flags
        ret = pthread_mutex_lock(&this->concur.flags_lock);
        if (ret != 0) RET_PTHREAD_ERR

        //set the workers ready flag
        this->concur.flags |= _worker_flag_release_ready;

        //unlock flags
        ret = pthread_mutex_unlock(&this->concur.flags_lock);
        if (ret != 0) RET_PTHREAD_ERR
    }

    //unlock the waiting count
    ret = pthread_mutex_unlock(&this->concur.release_count_lock);
    if (ret != 0) RET_PTHREAD_ERR

    //send a signal to the manager if the live count is now zero
    if (this->concur.alive_count == 0) {
        ret = pthread_cond_broadcast(&this->concur.alive_count_cond);
        if (ret != 0) RET_PTHREAD_ERR
    }
    
    //unlock alive count
    ret = pthread_mutex_unlock(&this->concur.alive_count_lock);
    if (ret != 0) RET_PTHREAD_ERR

    pthread_exit(0);
}


/*
 *  --- [WORKER | PUBLIC] ---
 */

sc::_worker::_worker(opt ** const opts,
                     _opt_scan ** const opts_scan,
                     _scan ** scan,
                     const std::vector<std::vector<const cm_lst_node *>>
                        & scan_area_sets,
                     const int scan_area_index,
                     const mc_session * session,
                     struct _worker_concurrency & concur)
 : opts(opts),
   opts_scan(opts_scan),
   scan(scan),
   scan_area_sets(scan_area_sets),
   scan_area_index(scan_area_index),
   session(session),
   concur(concur) {

    //allocate the read buffer
    this->buf = new cm_byte[this->session->page_size];
} 


sc::_worker::~_worker() {

    //deallocate the read buffer
    delete this->buf;
}


void sc::_worker::main() {

    std::optional<int> ret;


    //repeatedly perform requested scans
    while (true) {

        //wait for the manager to release this worker
        this->release_wait();

        //fetch own scan areas
        const std::vector<const cm_lst_node *> & scan_set
            = this->scan_area_sets[this->scan_area_index];


        //for every area in this worker's scan set
        for (auto area_iter
                = this->scan_area_sets[this->scan_area_index].begin();
             area_iter
                != this->scan_area_sets[this->scan_area_index].end();
             ++area_iter) {

            //if the exit bit is set, kill this worker
            if ((this->concur.flags | _worker_flag_exit) == true)
                this->exit();

            //if the cancel bit is set, reset
            if ((this->concur.flags | _worker_flag_cancel) == true)
                continue;

            //fetch this scan area
            mc_vm_area * area = MC_GET_NODE_AREA((*area_iter));

            //create a new `_scan_arg`
            struct _scan_arg scan_arg =
                _scan_arg(area->start_addr, 0, 0, this->buf, *area_iter);


            //process every address
            while (scan_arg.addr < area->end_addr) {

                //fetch next buffer if current buffer has run out
                if (scan_arg.buf_left <= (*this->opts)->addr_width) {
                    ret = this->read_buffer_smart(scan_arg);
                    if (ret.has_value() == false) this->exit();
                }

                //send this address to the scanner
                (*this->scan)->_process_addr(scan_arg);

                //increment `_scan_arg` state
                ++scan_arg.addr;
                ++scan_arg.area_off;
                --scan_arg.buf_left;
                ++scan_arg.cur_byte;
                
            } //end process every address
                
        } //end for every area in this worker's scan set

    } //end repeatedly perform requested scans
}


/*
 *  --- [WORKER MANAGER | PRIVATE] ---
 */

std::optional<int> sc::worker_mngr::spawn_workers() {

    std::optional<int> ret;

    
    //check sessions have been provided
    const std::vector<const mc_session *> & sessions = opts->get_sessions();
    if (opts->get_sessions().size() == 0) {
        sc_errno = SC_ERR_OPT_NOSESSION;
        return std::nullopt;
    }

    //for every provided session
    for (int i = 0; i < sessions.size(); ++ i) {

        //create a new worker
        this->workers.emplace_back(sc::_worker(&this->opts,
                                               &this->opts_scan,
                                               &this->scan,
                                               this->scan_area_sets,
                                               i,
                                               sessions[i],
                                               this->concur));

        //add a pthread_id for this worker
        this->worker_ids.push_back(0);

        //start a thread for this worker
        ret = pthread_create(&this->worker_ids.back(), nullptr,
                             _bootstrap_worker, &this->workers.back());
        if (ret != 0) {
            sc_errno = SC_ERR_PTHREAD;
            return std::nullopt;
        }

    } //end for every provided session

    return 0;
}


std::optional<int> sc::worker_mngr::kill_workers() {

    int ret;
    bool join_good = true;
    std::timespec time;

    //just return if there are no workers
    if (this->workers.size() == 0) return 0;


    //set the exit bit

    //lock flags
    do {
        ret = pthread_mutex_lock(&this->concur.flags_lock);
    } while (ret != 0);

    //set the worker exit bit
    this->concur.flags |= _worker_flag_exit;

    //unlock flags
    do {
        ret = pthread_mutex_unlock(&this->concur.flags_lock);
    } while (ret != 0);


    //repeatedly fire the release condition & wait for
    //all threads to exit
    while (true) {

        //if all workers exited, stop waiting
        if (this->concur.alive_count == 0) break;

        //fire the release condition
        do {
            ret = pthread_cond_broadcast(&this->concur.release_count_cond);
        } while (ret != 0);

        //get absolute time for the timed wait call to expire
        ret = clock_gettime(CLOCK_MONOTONIC, &time);
        if (ret == -1) {
            sc_errno = SC_ERR_TIMESPEC;
            return std::nullopt;
        }
        time.tv_nsec += 500000;

        //performe a timed wait
        do {
            ret = pthread_cond_timedwait(&this->concur.alive_count_cond,
                                         &this->concur.alive_count_lock,
                                         &time);
        } while (ret != 0);

    } //end while


    //for every worker
    for (auto worker_iter = this->worker_ids.begin();
         worker_iter != this->worker_ids.end(); ++worker_iter) {

        //try to join & defer error reporting
        ret = pthread_join(*worker_iter, nullptr);
        if (ret != 0) join_good = false;

    } //end for every worker

    //report any error now
    if (join_good == false) {
        sc_errno = SC_ERR_PTHREAD;
        return std::nullopt;
    }

    //empty worker-related vectors
    this->workers.clear();
    this->worker_ids.clear();

    return 0;
}


/*
 *  NOTE: This is a very basic linear sort with O(n^2) complexity.
 *        Improvements are very welcome.
 */

std::optional<int> sc::worker_mngr::sort_by_size(const map_area_set & ma_set) {

    mc_vm_area * area;
    size_t area_size;
    bool inserted;


    //fetch the scan areas set
    const std::unordered_set<cm_lst_node *> scan_area_nodes
        = ma_set.get_area_nodes();

    //if scan areas set is empty, return an error
    if (scan_area_nodes.empty() == true) {
        sc_errno = SC_ERR_SCAN_EMPTY;
        return std::nullopt;
    }

    //add first element to initialise iteration
    area = MC_GET_NODE_AREA((*(scan_area_nodes.begin())));
    area_size = area->end_addr - area->start_addr;
    struct sc::_sa_sort_entry first_entry
        = sc::_sa_sort_entry(area_size, *scan_area_nodes.begin());
    this->sorted_entries.push_back(first_entry);

    //for every area in the scan areas hashmap
    for (auto sa_set_iter = scan_area_nodes.begin()++;
         sa_set_iter != scan_area_nodes.end(); ++sa_set_iter) {

        inserted = false;

        //create a new sorted entry
        area = MC_GET_NODE_AREA((*sa_set_iter));
        area_size = area->end_addr - area->start_addr;
        struct sc::_sa_sort_entry iter_entry
            = sc::_sa_sort_entry(area_size, *sa_set_iter);

        //for every existing sorted entry
        for (auto sorted_iter = this->sorted_entries.begin();
             sorted_iter != this->sorted_entries.end(); ++sorted_iter) {

            //add the new sorted entry at this position
            if (iter_entry.size >= sorted_iter->size) {
                this->sorted_entries.insert(sorted_iter, iter_entry);
                inserted = true;
                break;
            }

        } //end for every existing sorted entry

        //if the new entry has not yet been inserted, it must be the smallest
        this->sorted_entries.push_back(iter_entry);

    } //end for every area in the scan areas hashmap

    return 0;
}



/*
 *  NOTE: Dividing areas between workers is currently done using the
 *        Largest Differencing Method (greedy algorithm).
 */

std::optional<int>
sc::worker_mngr::update_scan_area_set(const map_area_set & ma_set) {

    std::optional<int> ret;


    //reset existing scan_area_sets
    this->scan_area_sets.clear();

    //get a sorted version of the scan areas hashmap
    ret = sort_by_size(ma_set);
    if (ret == std::nullopt) return std::nullopt;


    /*
     *  Greedily process each sorted entry.
     */

    //workers can't be empty
    std::vector<size_t> greedy_size_sum(this->workers.size(), 0);

    //for every sorted entry
    for (auto entry_iter = this->sorted_entries.begin();
         entry_iter != this->sorted_entries.end(); ++entry_iter) {

        //find smallest size sum
        auto min_iter = std::min_element(greedy_size_sum.begin(),
                                         greedy_size_sum.end());
        int min_index = std::distance(greedy_size_sum.begin(), min_iter);

        //add this entry to the smallest size sum
        *min_iter += entry_iter->size;
        this->scan_area_sets[min_index].push_back(entry_iter->area_node);

    } //end for every serted entry

    return 0;
}


/*
 *  --- [WORKER MANAGER | INTERNAL INTERFACE] ---
 */

_SC_DBG_STATIC
const constexpr useconds_t _single_run_sleep_interval_usec = 10000;

//scan the selected area set once
std::optional<int> sc::worker_mngr::_single_run() {

    int ret;


    //wait for threads to be ready
    while (this->concur.flags | _worker_flag_release_ready) {
        usleep(_single_run_sleep_interval_usec);
    }

    //lock the flags
    do {
        ret = pthread_mutex_lock(&this->concur.flags_lock);
    } while (ret != 0);

    //reset the workers ready flag
    this->concur.flags &= ~_worker_flag_release_ready;

    //unlock flags
    do {
        ret = pthread_mutex_unlock(&this->concur.flags_lock);
    } while (ret != 0);

    //release the threads
    do {
        ret = pthread_cond_broadcast(&this->concur.release_count_cond);
    } while (ret != 0);

    //wait for the threads to finish
    while (this->concur.flags | _worker_flag_release_ready) {
        usleep(_single_run_sleep_interval_usec);
    }

    return 0;
}


/*
 *  --- [WORKER MANAGER | PUBLIC] ---
 */


sc::worker_mngr::worker_mngr()
 : _lockable(),
   opts(nullptr),
   opts_scan(nullptr),
   scan(nullptr) {}


//cleanup
sc::worker_mngr::~worker_mngr() {

    /*
     *  NOTE: This is another instance of favouring RAII over proper error
     *        propagation. While a pthread error here is highly unlikely,
     *        the user isn't notified if one occurs.
     */

    //kill any workers
    this->kill_workers();

    /*
     *  This is technically not necessary on Linux. It also can't fail.
     */
    pthread_cond_destroy(&this->concur.release_count_cond);
    pthread_mutex_destroy(&this->concur.release_count_lock);

    pthread_cond_destroy(&this->concur.alive_count_cond);
    pthread_mutex_destroy(&this->concur.alive_count_lock);

    pthread_mutex_destroy(&this->concur.flags_lock);

    return;
}


std::optional<int> sc::worker_mngr::free_workers() {

    std::optional<int> ret;


    //kill previous workers
    ret = this->kill_workers();
    if (ret.has_value() == false) return std::nullopt;

    return 0;
}


std::optional<int> sc::worker_mngr::do_scan(const sc::opt & opts,
                                            const sc::_opt_scan & opts_scan,
                                            sc::_scan & scan,
                                            const sc::map_area_set & ma_set,
                                            const cm_byte flags) {

    std::optional<int> ret;
    bool same_set_num = true, run_err = false;


    //populate & lock cache
    this->opts = (sc::opt *) &opts;
    ret = this->opts->_lock();
    if (ret.has_value() == false) return std::nullopt; 
    
    this->opts_scan = (sc::_opt_scan *) &opts_scan;
    ret = this->opts_scan->_lock();
    if (ret.has_value() == false) {
        ret = this->opts->_unlock();
        return std::nullopt;
    }
    
    this->scan = (sc::_scan *) &scan;
    ret = this->scan->_lock();
    if (ret.has_value() == false) {
        ret = this->opts->_unlock();
        ret = this->opts_scan->_unlock();
        return std::nullopt; 
    }

    //respawn workers
    if ((flags & sc::WORKER_MNGR_KEEP_WORKERS) == false) {

        //used to check if the number of workers changed
        int worker_num = this->workers.size();

        //kill previous workers
        ret = this->kill_workers();
        if (ret.has_value() == false) {
            run_err = true;
            goto do_scan_cleanup;
        }

        //spawn new workers
        ret = this->spawn_workers();
        if (ret.has_value() == false) {
            run_err = true;
            goto do_scan_cleanup;
        }

        //take note that the number of worker threads changed
        if (this->workers.size() != worker_num) same_set_num = false;
    }

    //re-populate scan area sets
    if (((flags & sc::WORKER_MNGR_KEEP_SCAN_SET) == false)
        || (same_set_num == false)) {

        //update scan set
        ret = this->update_scan_area_set(ma_set);
        if (ret.has_value() == false) {
            run_err = true;
            goto do_scan_cleanup;
        }
    }


    //call this scanner's manager
    ret = scan._manage_scan(*this);
    if (ret.has_value() == false) return std::nullopt;


    do_scan_cleanup:
    ret = this->opts->_lock();
    if (ret.has_value() == false) run_err = true; 
    
    ret = this->opts_scan->_lock();
    if (ret.has_value() == false) run_err = true;
    
    ret = this->scan->_lock();
    if (ret.has_value() == false) run_err = true; 


    if (run_err == true) return std::nullopt;
    return 0;
}
