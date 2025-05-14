//standard template library
#include <optional>

//external libraries
#include <pthread.h>

//local headers
#include "scancry.h"
#include "error.hh"



sc::_lockable::~_lockable() {

    if (this->in_use == true) {
        print_warning("Destroyed a lockable object while it was locked.");
    }
}


_SC_DBG_INLINE int sc::_lockable::_lock() noexcept {

    int ret, ret_val;

    ret = pthread_mutex_trylock(&this->in_use_lock);
    if (ret != 0) {
        if (ret == EBUSY) sc_errno = SC_ERR_IN_USE;
        else sc_errno = SC_ERR_PTHREAD;
        return -1;
    }

    if (in_use == true) {
        sc_errno = SC_ERR_IN_USE;
        ret_val = -1;
    } else {
        in_use = true;
        ret_val = 0;
    }
    
    ret = pthread_mutex_unlock(&this->in_use_lock);
    if (ret != 0) {
        sc_errno = SC_ERR_PTHREAD;
        return -1;
    }

    return ret_val;
}


_SC_DBG_INLINE int sc::_lockable::_unlock() noexcept {

    /*
     *  NOTE: Not necessary to acquire the mutex; the only possible
     *        race condition is another thread still reading `true`
     *        after the lock is released.
     */

    this->in_use = false;

    return 0;
}


_SC_DBG_INLINE bool sc::_lockable::_get_lock() const noexcept {
    return this->in_use;
} 
