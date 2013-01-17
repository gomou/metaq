#!/bin/sh

LAST_DIR=`pwd`
cd ../src
make -f libdiamond.so.mak  clean all
cd $LAST_DIR
