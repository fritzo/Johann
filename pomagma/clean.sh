#!/bin/sh
rm -rf lib build
find . -type f | grep '\.log$' | xargs rm -f
find . -type f | grep '^core' | xargs rm -f
