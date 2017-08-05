#!/bin/bash
DYLD_LIBRARY_PATH=`pwd`/src/ ./tools/qurl/qurl "$@"
