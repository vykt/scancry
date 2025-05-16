# source debug scripts
source memcry.gdb
source scancry.gdb

# place your gdb commands here
#tb main
tb test_map_area_set.cc:133
run
layout src
