#!/bin/sh

echo "building pantheios with STLSOFT at $1 ..."

export STLSOFT=$1

cd build/gcc48.unix
make build

exit $?
