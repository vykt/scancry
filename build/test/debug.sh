#!/bin/sh
gdb -x user.gdb --args ./test -P "$@"
