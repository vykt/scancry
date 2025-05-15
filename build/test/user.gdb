# source debug scripts
source memcry.gdb
source scancry.gdb

# place your gdb commands here
#tb main
tb test_opt.cc:1046
run
layout src
