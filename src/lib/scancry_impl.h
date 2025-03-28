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


// [debugging & unit testing support]
#ifdef DEBUG 
#define _SC_DBG_STATIC
#define _SC_DBG_INLINE
#define _SC_DBG_PRIVATE public
#else
#define _SC_DBG_STATIC static
#define _SC_DBG_INLINE inline
#define _SC_DBG_PRIVATE private
#endif


#ifdef __cplusplus
namespace sc {

class opt;

/*
 *  Dependency injection abstract scan class.
 */
struct _scan_arg {

    //[members]
    uintptr_t addr;
    off_t area_off;
    size_t buf_left;
    cm_byte * cur_byte;
    const cm_lst_node * const area_node;

    //[methods]
    _scan_arg(const uintptr_t addr, const off_t area_off, const size_t buf_left,
              cm_byte * cur_byte, const cm_lst_node * const area_node)
     : addr(addr), area_off(area_off), buf_left(buf_left), cur_byte(cur_byte), area_node(area_node) {};
};


class _scan {

    _SC_DBG_PRIVATE:
        //[attributes]
        //TODO any?

    public:
        //[methods]
        //dependency injection function
        virtual void process_addr(const struct _scan_arg arg,
                                  const opt & opts, const void * const opts_custom) = 0;
};


/*
 *  Pointer scanner `arg_custom`
 */
class _ptrscan_tree_node;


/*
 *  This class represents a single thread used for scanning some subset of
 *  a selected `map_area_set`.
 */

const constexpr cm_byte _worker_flag_release_ready = 0x1;
const constexpr cm_byte _worker_flag_layer_ready   = 0x2;
const constexpr cm_byte _worker_flag_exit          = 0x4;
const constexpr cm_byte _worker_flag_cancel        = 0x8;

struct _worker_concurrency {

    //release adaptive barrier
    pthread_cond_t release_count_cond;
    pthread_mutex_t release_count_lock;
    int release_count;


    //layer adaptive barrier
    pthread_cond_t layer_count_cond;
    pthread_mutex_t layer_count_lock;
    int layer_count;

    //number of alive threads
    pthread_cond_t alive_count_cond;
    pthread_mutex_t alive_count_lock;
    int alive_count;

    pthread_mutex_t flags_lock;
    cm_byte flags;

    _worker_concurrency()
     : release_count_cond(PTHREAD_COND_INITIALIZER),
       release_count_lock(PTHREAD_MUTEX_INITIALIZER),
       release_count(0),
       layer_count_cond(PTHREAD_COND_INITIALIZER),
       layer_count_lock(PTHREAD_MUTEX_INITIALIZER),
       layer_count(0),
       alive_count_cond(PTHREAD_COND_INITIALIZER),
       alive_count_lock(PTHREAD_MUTEX_INITIALIZER),
       alive_count(0),
       flags_lock(PTHREAD_MUTEX_INITIALIZER),
       flags(0) {}
};


class _worker {

    _SC_DBG_PRIVATE:
        //[attributes]
        const opt & opts;

        const std::vector<std::vector<const cm_lst_node *>> & scan_area_sets;
        const int scan_area_index;

        const mc_session * session;
        _scan ** scan;

        //concurrency (adaptive barrier)
        struct _worker_concurrency & concur;

        //scanning
        cm_byte * buf;

        //[methods]
        std::optional<int> read_buffer_smart(struct _scan_arg & arg) noexcept;

        //synchronisation
        void release_wait() noexcept;
        void layer_wait() noexcept;
        void exit() noexcept;

    public:
        //[methods]
        //ctor
        _worker(const opt & opts,
                const std::vector<std::vector<const cm_lst_node *>> & scan_area_sets,
                const int scan_area_index,
                const mc_session * session,
                _scan ** scan,
                struct _worker_concurrency & concur);
        ~_worker();

        void main();
};


//used to sort a `map_area_set` hashmap into a vector by size
struct _sa_sort_entry {

    const size_t size;
    const cm_lst_node * area_node;

    _sa_sort_entry(const size_t size, const cm_lst_node * area_node)
     : size(size), area_node(area_node) {}
};


} //namespace sc
#endif //ifdef __cplusplus


#endif //define SCANCRY_INTERNAL_H
