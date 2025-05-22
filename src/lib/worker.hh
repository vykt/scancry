#pragma once

//external libraries
#include <cmore.h>
#include <memcry.h>
#include <pthread.h>

//local headers
#include "scancry.h"

#ifdef __cplusplus
extern "C" {
#endif

//worker_pool - external
sc_worker_pool sc_new_worker_pool();
int sc_del_worker_pool(sc_worker_pool w_pool);
int sc_wp_free_workers(sc_worker_pool w_pool);

#ifdef __cplusplus
} //extern "C"
#endif
