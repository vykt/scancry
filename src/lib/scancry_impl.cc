//standard template library
#include <optional>

//external libraries
#include <pthread.h>

//local headers
#include "scancry.h"
#include "error.hh"



//copy constructor
sc::_lockable::_lockable(const sc::_lockable & lockable)
 : lock(PTHREAD_RWLOCK_INITIALIZER) {}


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
