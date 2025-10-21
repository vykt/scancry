//C standard library
#include <cstring>
#include <cstdio>
#include <cerrno>

//system headers
#include <limits.h>

//external libraries
#include <pthread.h>

//local headers
#include "scancry.h"
#include "common.hh"
#include "error.hh"



/*
 *  --- [_LOCKABLE] ---
 */

//reset the lock during copying
void sc::_lockable::do_copy(const sc::_lockable & lockable) noexcept {

    this->lock = PTHREAD_RWLOCK_INITIALIZER;
    return;
}


//constructor
sc::_lockable::_lockable() noexcept
: lock(PTHREAD_RWLOCK_INITIALIZER) {}


//copy constructor
sc::_lockable::_lockable(const sc::_lockable & lockable) noexcept {

    this->do_copy(lockable);
    return;
}


//copy assignment operator
sc::_lockable & sc::_lockable::operator=(
    const sc::_lockable & lockable) noexcept {

    if (this != &lockable) this->do_copy(lockable);
    return *this;
}


//acquire a read lock
int sc::_lockable::_lock_read() const noexcept {

    int ret;


    //try to acquire the lock
    ret = pthread_rwlock_tryrdlock(&this->lock);
    if (ret != 0) {
        if (ret == EBUSY) sc_errno = SC_ERR_IN_USE;
        else sc_errno = SC_ERR_PTHREAD;

        return -1;
    }

    return 0;
}


//acquire a write lock
int sc::_lockable::_lock_write() const noexcept {

    int ret;


    //try to acquire the lock
    ret = pthread_rwlock_trywrlock(&this->lock);
    if (ret != 0) {
        if (ret == EBUSY) sc_errno = SC_ERR_IN_USE;
        else sc_errno = SC_ERR_PTHREAD;

        return -1;
    }

    return 0;
}


//acquire a read lock (blocking)
int sc::_lockable::_await_read() const noexcept {

    int ret;


    //try to acquire the lock
    ret = pthread_rwlock_rdlock(&this->lock);
    if (ret != 0) {
        sc_errno = SC_ERR_PTHREAD;
        return -1;
    }

    return 0;
}


//acquire a write lock (blocking)
int sc::_lockable::_await_write() const noexcept {

    int ret;


    //try to acquire the lock
    ret = pthread_rwlock_wrlock(&this->lock);
    if (ret != 0) {
        sc_errno = SC_ERR_PTHREAD;
        return -1;
    }

    return 0;
}


//release a read or write lock
void sc::_lockable::_unlock() const noexcept {

    pthread_rwlock_unlock(&this->lock);
    return;
}



/*
 *  --- [_CTOR_FAILABLE] ---
 */

//copy constructor status
void sc::_ctor_failable::do_copy(
    const sc::_ctor_failable & ctor_failable) noexcept {

    this->ctor_failed = ctor_failable.ctor_failed;
    return;
}


//constructor
sc::_ctor_failable::_ctor_failable() noexcept
 : ctor_failed(false) {}


//copy constructor
sc::_ctor_failable::_ctor_failable(
    const sc::_ctor_failable & ctor_failable) noexcept {

    this->do_copy(ctor_failable);
    return;
}


//copy assignment operator
sc::_ctor_failable & sc::_ctor_failable::operator=(
    const sc::_ctor_failable & ctor_failable) noexcept {

    if (this != &ctor_failable) this->do_copy(ctor_failable);
    return *this;
}


//setter & getter
[[nodiscard]] bool sc::_ctor_failable::get_ctor_failed() const noexcept {
    return this->ctor_failed;
}


void sc::_ctor_failable::_set_ctor_failed(const bool failed) noexcept {
    this->ctor_failed = failed;
}



/*
 *  --- [_STATEFUL] ---
 */

sc::_stateful::_stateful() noexcept
 : state_bitset(0b0) {};


//set bits
void sc::_stateful::_set_bits(const cm_byte bitset) noexcept {
    this->state_bitset |= bitset;
    return;
}


//unset bits
void sc::_stateful::_unset_bits(const cm_byte bitset) noexcept {
    this->state_bitset &= ~bitset;
    return;
}


//get bits
[[nodiscard]] cm_byte
    sc::_stateful::_get_bits(const cm_byte bitset) const noexcept {
    return (this->state_bitset & bitset);
}



/*
 *  --- [_OPT_SCAN] ---
 */

//call parent's copy assignments
void sc::_opt_scan::do_copy(sc::_opt_scan & opts_scan) noexcept {
    
    //call parent copy assignment operators
    _lockable::operator=(opts_scan);
    _ctor_failable::operator=(opts_scan);

    return;
}


//constructor
sc::_opt_scan::_opt_scan() noexcept
 : _lockable(), _ctor_failable() {}


//copy constructor
sc::_opt_scan::_opt_scan(sc::_opt_scan & opts_scan) noexcept
 : _lockable(), _ctor_failable() {

    this->do_copy(opts_scan);
    return;
}


//destructor
sc::_opt_scan::~_opt_scan() noexcept {}


//copy assignment operator
sc::_opt_scan & sc::_opt_scan::operator=(
    sc::_opt_scan & opts_scan) noexcept {

    if (this != &opts_scan) this->do_copy(opts_scan);
    return *this;    
}



/*
 *  --- [_SCAN_ARG] ---
 */

//reset buffer related attributes
void sc::_scan_arg::reset_buf(
    const size_t new_buf_left, const cm_byte * new_cur_byte) noexcept {

    this->buf_left = new_buf_left;
    this->cur_byte = new_cur_byte;

    return;
}


//advance the buffer
void sc::_scan_arg::advance_buf(const size_t advance) noexcept {

    this->addr     += advance;
    this->area_off += advance;
    this->buf_left -= advance;
    this->cur_byte += advance;

    return;
}


//getters
[[nodiscard]] uintptr_t sc::_scan_arg::get_addr() const noexcept {
    return this->addr;
}

[[nodiscard]] const cm_lst_node *
    sc::_scan_arg::get_area_node() const noexcept {

    return this->area_node;
}

[[nodiscard]] off_t sc::_scan_arg::get_area_off() const noexcept {
    return this->area_off;
}

[[nodiscard]] const cm_byte * sc::_scan_arg::get_cur_byte() const noexcept {
    return this->cur_byte;
}

[[nodiscard]] size_t sc::_scan_arg::get_buf_left() const noexcept {
    return this->buf_left;
}


/*
 *  --- [_SCAN] ---
 */

//constructor
sc::_scan::_scan() noexcept
 : _lockable(), _ctor_failable(), _stateful() {}


//destructor
sc::_scan::~_scan() noexcept {}


//acquire locks & check state on entry
[[nodiscard]] int sc::_scan::handle_entry(
    const sc::opt * opts,
    const sc::_opt_scan * opts_scan,
    const cm_byte query_bitset,
    const cm_byte assert_bitset,
    const bool is_write_lock) const noexcept {

    int ret;
    cm_byte state_bitset;


    //acquire lock
    if (is_write_lock) { _LOCK_WRITE(-1) }
    else { _LOCK_READ(-1) }

    if (opts != nullptr) {
        ret = opts->_lock_read();
        if (ret != 0) goto _scan_handle_entry_fail_1;
    }

    if (opts_scan != nullptr) {
        ret = opts->_lock_read();
        if (ret != 0) goto _scan_handle_entry_fail_2;
    }

    //assert state
    if (query_bitset != 0b0) {
        state_bitset = this->_get_bits(query_bitset);
        if (state_bitset != assert_bitset) {
            sc_errno = SC_ERR_STATE;
            goto _scan_handle_entry_fail_3;
        }
    }

    return 0;

    //release read locks
    _scan_handle_entry_fail_3:
    if (opts_scan != nullptr) opts_scan->_unlock();

    _scan_handle_entry_fail_2:
    if (opts != nullptr) opts->_unlock();

    _scan_handle_entry_fail_1:
    _UNLOCK

    return -1;
}


//release locks on exit
void sc::_scan::handle_exit(
    const sc::opt * opts,
    const sc::_opt_scan * opts_scan) const noexcept {

    //release read locks
    if (opts_scan != nullptr) opts_scan->_unlock();
    if (opts != nullptr) opts->_unlock();
    _UNLOCK

    return;
}



/*
 *  --- [OBJ_TABLE] ---
 */

//destroy the pathname table
void sc::obj_table::del_pathname_tbl() noexcept {

    void * pathname;


    if (this->pathname_tbl.is_init == false) return;

    //for all pathnames in the pathname table
    for (int i = 0; i < this->pathname_tbl.len; ++i) {
        pathname = *(void **) cm_vct_get_p(&this->pathname_tbl, i);
        std::free(pathname);
    }

    //delete the pathname table itself
    cm_del_vct(&this->pathname_tbl);
    
    return;
}


//perform a copy
void sc::obj_table::do_copy(const sc::obj_table & obj_tbl) noexcept {

    int ret;

    void * pathname;
    char * new_pathname;
    size_t len;


    //call the parent copy assignment operators
    _ctor_failable::operator=(obj_tbl);

    //delete old pathnames table if one exists
    this->del_pathname_tbl();

    //create a new pathname table
    ret = cm_new_vct(&this->pathname_tbl, sizeof(const char *));
    if (ret != 0) {
        sc_errno = SC_ERR_CMORE;
        this->_set_ctor_failed(true);
        return;
    }

    //copy the pathname table
    for (int i = 0; i < obj_tbl.get_pathname_tbl().len; ++i) {

        //get the next path
        pathname = *(void **) cm_vct_get_p(&this->pathname_tbl, i);
        len = strnlen((const char *) pathname, PATH_MAX);

        //allocate a new pathname
        new_pathname = (char *) std::malloc(len + 1);

        //copy the pathname
        std::strncpy(new_pathname, (const char *) pathname, len);

        //copy the pathname
        pathname = cm_vct_apd(&this->pathname_tbl, &new_pathname);
        if (pathname == nullptr) {
            sc_errno = SC_ERR_CMORE;
            this->_set_ctor_failed(true);
            cm_del_vct(&this->pathname_tbl);
            return;
        }
    }

    return;
}


//add a pathname to the pathname table
[[nodiscard]] int sc::obj_table::do_add(const char * pathname) noexcept {

    void * ret_data;

    char * new_pathname;
    size_t len;


    //get the length of the pathname
    len = strnlen(pathname, PATH_MAX);

    //allocate a new pathname
    new_pathname = (char *) std::malloc(len + 1); 
    if (new_pathname == nullptr) {
        sc_errno = SC_ERR_MEM;
        return -1;
    }

    //copy the pathname
    std::strncpy(new_pathname, (const char *) pathname, len);

    //append the new pathname to the pathname table
    ret_data = cm_vct_apd(&this->pathname_tbl, &new_pathname);
    if (ret_data == nullptr) {
        sc_errno = SC_ERR_CMORE;
        std::free(new_pathname);
        return -1;
    }

    return (this->pathname_tbl.len - 1);
}


//serialise to a file
[[nodiscard]] int sc::obj_table::serialise(FILE * fs) const noexcept {

    uint32_t str_sz;
    const char * pathname;

    size_t wr_ents;


    //for every string in the pathname table
    for (int i = 0; i < this->pathname_tbl.len; ++i) {

        //get the next pathname
        pathname = *(const char **) cm_vct_get_p(&this->pathname_tbl, i);

        //get the length of this string
        str_sz = (uint32_t) strnlen(pathname, PATH_MAX) + 1;

        //write the length of this string to the file stream
        wr_ents = std::fwrite(&str_sz, sizeof(str_sz), 1, fs);
        if (wr_ents != 1) { sc_errno = SC_ERR_FILE_IO; return -1; }

        //write this string to the file stream
        wr_ents = std::fwrite(pathname, sizeof(char), str_sz, fs);
        if (wr_ents != str_sz+1) { sc_errno = SC_ERR_FILE_IO; return -1; }
    }

    return 0;
}


//deserialise from a file
[[nodiscard]] int sc::obj_table::deserialise(
    FILE * fs, const int pathname_num) noexcept {

    int ret;

    uint32_t str_sz;
    char pathname_buf[PATH_MAX];

    size_t rd_ents;


    //empty the existing pathname table
    this->reset();

    //for every pathname in the file
    for (int i = 0; i < pathname_num; ++i) {

        //read the length of the next pathname
        rd_ents = std::fread(&str_sz, sizeof(str_sz), 1, fs);
        if (rd_ents != 1) { sc_errno = SC_ERR_FILE_IO; return -1; }

        //read the next pathname
        rd_ents = std::fread(pathname_buf, sizeof(char), str_sz, fs);
        if (rd_ents != str_sz) { sc_errno = SC_ERR_FILE_IO; return -1; }

        //insert this pathname into the pathname table
        ret = this->fadd_pathname(pathname_buf);
        if (ret != 0) return -1;
    }

    return 0;
}


//constructor
sc::obj_table::obj_table() noexcept
 : _ctor_failable() {

    //zero-out the pathname table
    std::memset(&this->pathname_tbl, 0, sizeof(this->pathname_tbl));

    return;
}


//copy constructor
sc::obj_table::obj_table(const sc::obj_table & obj_table) noexcept
 : _ctor_failable() {

    this->do_copy(obj_table);
    return;
}


//destructor
sc::obj_table::~obj_table() noexcept {

    this->del_pathname_tbl();
    return;
}


//copy assignment operator
sc::obj_table & sc::obj_table::operator=(
    const sc::obj_table & obj_tbl) noexcept {

    if (this != &obj_tbl) this->do_copy(obj_tbl);
    return *this;
}


//resetter
void sc::obj_table::reset() noexcept {

    void * pathname;


    if (this->pathname_tbl.is_init == false) return;

    //for all pathnames in the pathname table
    for (int i = 0; i < this->pathname_tbl.len; ++i) {
        pathname = *(void **) cm_vct_get_p(&this->pathname_tbl, i);
        std::free(pathname);
    }

    //empty the pathname table itself
    cm_vct_emp(&this->pathname_tbl);
    
    return;
}


//resolve a pathname from an index
[[nodiscard]] const char * sc::obj_table::resolv_pathname(
    const int idx) const noexcept {

    void ** pathname;


    //get pathname at provided index
    pathname = (void **) cm_vct_get_p(&this->pathname_tbl, idx);
    if (pathname == nullptr) {
        sc_errno = SC_ERR_CMORE;
        return nullptr;
    }

    return (const char *) *pathname;
}


//resolve an object node from an index
[[nodiscard]] const cm_lst_node * sc::obj_table::resolv_obj_node(
    const int idx, const mc_vm_map * map) const noexcept {

    void ** pathname;
    cm_lst_node * obj_node;


    //get pathname at provided index
    pathname = (void **) cm_vct_get_p(&this->pathname_tbl, idx);
    if (pathname == nullptr) {
        sc_errno = SC_ERR_CMORE;
        return nullptr;
    }

    //find the corresponding object for this path, if one exists
    obj_node = mc_get_obj_by_pathname(map, (const char *) *pathname);
    if (obj_node == nullptr) {
        sc_errno = SC_ERR_MEMCRY;
        return nullptr;
    }

    return obj_node;
}


//add a pathname to the pathname table and return its index
[[nodiscard]] int
    sc::obj_table::fadd_pathname(const char * pathname) noexcept {

    return this->do_add(pathname);
}


//return an index for a given pathname, add it to the table if not present
[[nodiscard]] int
    sc::obj_table::add_pathname(const char * pathname) noexcept {

    int ret;
    const char * it_pathname;


    //for every pathname in the table
    for (int i = 0; i < this->pathname_tbl.len; ++i) {

        //get the next pathname
        it_pathname
            = *(const char **) cm_vct_get_p(&this->pathname_tbl, i);

        //if its a match, return its index
        ret = strncmp(pathname, it_pathname, PATH_MAX);
        if (ret == 0) return i;
    }

    //add this pathname to the table if it wasn't fouund
    return this->do_add(pathname);
}


//get the length of the pathname table
[[nodiscard]] int sc::obj_table::get_sz() const noexcept {

    return this->pathname_tbl.len;
}


//get a refernece to the pathname table
[[nodiscard]] const cm_vct /* <const char * (alloc)> */ &
    sc::obj_table::get_pathname_tbl() const noexcept {

    return pathname_tbl;
}
