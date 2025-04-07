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


_SC_DBG_INLINE std::optional<int> sc::_lockable::_lock() noexcept {

    int ret;
    std::optional<int> ret_opt;

    ret = pthread_mutex_trylock(&this->in_use_lock);
    if (ret != 0) {
        if (ret == EBUSY) sc_errno = SC_ERR_IN_USE;
        else sc_errno = SC_ERR_PTHREAD;
        return std::nullopt;
    }

    if (in_use == true) {
        sc_errno = SC_ERR_PTHREAD;
        ret_opt = std::nullopt;
    } else {
        in_use = true;
        ret_opt = 0;
    }
    
    ret = pthread_mutex_unlock(&this->in_use_lock);
    if (ret != 0) {
        sc_errno = SC_ERR_PTHREAD;
        return std::nullopt;
    }

    return ret_opt;
}


_SC_DBG_INLINE std::optional<int> sc::_lockable::_unlock() noexcept {

    int ret;
    ret = pthread_mutex_trylock(&this->in_use_lock);
    if (ret != 0) {
        if (ret == EBUSY) return 1;
        sc_errno = SC_ERR_PTHREAD;
        return std::nullopt;
    }

    in_use = false;
    
    ret = pthread_mutex_unlock(&this->in_use_lock);
    if (ret != 0) {
        sc_errno = SC_ERR_PTHREAD;
        return std::nullopt;
    }

    return 0;
}


_SC_DBG_INLINE bool sc::_lockable::_get_lock() const noexcept {
    return this->in_use;
} 
