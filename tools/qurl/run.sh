#!/bin/bash
DYLD_LIBRARY_PATH=`pwd`/src/:`pwd`/about ./tools/qurl/qurl "$@"
