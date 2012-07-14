#!/bin/sh
test -e build || mkdir build && (cd build && cmake .. && make && sudo make install)
