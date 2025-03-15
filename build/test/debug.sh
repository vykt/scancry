#!/bin/sh
gdb -x scancry.gdb --args ./test -O "$@"
