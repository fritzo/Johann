#!/bin/sh
test -e build || mkdir build && \
(cd build && cmake .. && make && rm test.log && \
POMAGMA_LOG_LEVEL=3 POMAGMA_LOG_FILE=../test.log make test || \
 grep -C3 -i error test.log)
