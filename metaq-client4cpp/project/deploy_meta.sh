#!/bin/sh

export LWPR_ENV_RELEASE=1
export LWPR_ENV_ARCH=64

make clean all

MULU_TMP=DEP_META_`date +%s`
program_name=libmeta_linux_x64_1.1.0_build_973
package_name=$MULU_TMP/$program_name
mkdir $MULU_TMP
mkdir $package_name
mkdir $package_name/include
mkdir $package_name/include/META
mkdir $package_name/include/ZK
mkdir $package_name/include/lwpr
mkdir $package_name/lib
mkdir $package_name/samples
mkdir $package_name/project
mkdir $package_name/bin

cp ../src/FetchConfigThread.h			$package_name/include/META/
cp ../src/Message.h	$package_name/include/META/
cp ../src/MessageProducer.h	$package_name/include/META/
cp ../src/MessageSessionFactory.h	$package_name/include/META/
cp ../src/MetaClientConfig.h	$package_name/include/META/
cp ../src/RemotingClientWrapper.h     	$package_name/include/META/
cp ../src/Shutdownable.h	$package_name/include/META/
cp ../src/UtilModule.h	$package_name/include/META/
cp ../src/ZKClient.h	$package_name/include/META/

cp ../include/lwpr.h	$package_name/include/
cp ../include/ZK/*.h	$package_name/include/ZK/
cp ../include/lwpr/*.h	$package_name/include/lwpr/


cp ../project/makstand	$package_name/project/



cp ../lib/libmeta.a $package_name/lib/
cp ../lib/libmeta.so $package_name/lib/
cp ../samples/*.cpp $package_name/samples/
cp ../samples/makefile $package_name/samples/
cp ../samples/*.mak $package_name/samples/

cd $MULU_TMP
tar cvf ${program_name}.tar $program_name
gzip ${program_name}.tar
