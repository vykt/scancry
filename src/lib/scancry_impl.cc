//standard template library
#include <optional>

//external libraries
#include <pthread.h>

//local headers
#include "scancry.h"
#include "error.hh"



/*
 *  --- [_LOCKABLE] ---
 */

//reset the lock during copying
void sc::_lockable::do_copy(const sc::_lockable & lockable) noexcept {

    this->lock = PTHREAD_RWLOCK_INITIALIZER;
    return;
}


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
_SC_DBG_INLINE int sc::_lockable::_lock_read() noexcept {

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
_SC_DBG_INLINE int sc::_lockable::_lock_write() noexcept {

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


//release a read or write lock
_SC_DBG_INLINE void sc::_lockable::_unlock() noexcept {

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
