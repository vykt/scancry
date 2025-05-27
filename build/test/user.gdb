# source debug scripts
source memcry.gdb
source scancry.gdb

# place your gdb commands here
#tb main
tb test_worker_pool.cc:303
run
continue
continue
#b _bootstrap_worker
directory /home/vykt/ext-repos/debian/glibc-2.36
#set scheduler-locking step
layout src
