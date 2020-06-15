#!/bin/sh

cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_ECLIPSE_MAKE_ARGUMENTS=-j1 -D_ECLIPSE_VERSION=4.4
sed 's/\( include=".*\/git\/src" kind="inc" path="" system=\)"true"/\1"false"/' .cproject > .cproject_
mv .cproject_ .cproject
