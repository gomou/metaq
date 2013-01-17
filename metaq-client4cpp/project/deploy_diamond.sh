#!/bin/sh

export LWPR_ENV_RELEASE=1
export LWPR_ENV_ARCH=64

make clean all

MULU_TMP=DEP_DIAMOND_`date +%s`

program_name=libdiamond_linux_x64_1.0.0_build_970
package_name=$MULU_TMP/$program_name
mkdir $MULU_TMP
mkdir $package_name
mkdir $package_name/include
mkdir $package_name/include/DIAMOND
mkdir $package_name/lib
mkdir $package_name/samples

cp ../src/DiamondClient.h $package_name/include/DIAMOND/
cp ../lib/libdiamond.a $package_name/lib/
cp ../lib/libdiamond.so $package_name/lib/
cp ../diamond_examples/*.cpp $package_name/samples/
cp ../diamond_examples/makefile $package_name/samples/

cd $MULU_TMP
tar cvf ${program_name}.tar $program_name
gzip ${program_name}.tar
