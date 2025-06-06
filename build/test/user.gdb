# source debug scripts
source memcry.gdb
source scancry.gdb

# place your gdb commands here
tb test_worker_pool.cc:686

run
continue
continue
continue
continue
continue
continue

#b _bootstrap_worker
#set scheduler-locking step

layout src
