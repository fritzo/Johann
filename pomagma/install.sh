#!/bin/sh
test -e build || mkdir build && (cd build && cmake .. && make && make install)
