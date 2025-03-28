#pragma once

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry.h"


class _worker {

    _SC_DBG_PRIVATE:
        //[attributes]
        pthread_barrier_t & barrier;
        bool stop_flag;

        const mc_session * session;
        const pthread_t id;

        const std::vector<const cm_lst_node * const> 

    public:
        //[methods]
        worker(pthread_barrier_t & barrier, const mc_session * session)

};
