#!/bin/bash

if [ -t 1 ]
then
	RED="\033[31m"
	GREEN="\033[32m"
	YELLOW="\033[33m"
	BOLD="\033[1m"
	OFF="\033[0m"
else
	RED=
	GREEN=
	YELLOW=
	BOLD=
	OFF=
fi

CLANG=/home/odroid/build/bin/clang
CLANGXX=/home/odroid/build/bin/clang++
CONFIG="sizes.cfg 32"
LOWFAT_SRC=/home/odroid/zomtag-llvm/zometag-alloc/ref/LowFat
RUNTIME_PATH=${LOWFAT_SRC}/llvm-4.0.0.src/projects/compiler-rt/lib/lowfat/
INSTRUMENTATION_PATH=${LOWFAT_SRC}/llvm-4.0.0.src/lib/Transforms/Instrumentation/
CLANGLIB_PATH=${LOWFAT_SRC}/llvm-4.0.0.src/tools/clang/lib/Basic/

echo -e \
	"${GREEN}$0${OFF}: using the default LowFat configuration... ($CONFIG)..."

echo -e \
	"${GREEN}$0${OFF}: building the Lowfat config builder..."
cd ${LOWFAT_SRC}/config
CC=$CLANG CFLAGS="-std=gnu99" CXX=$CLANGXX make >/dev/null

echo -e "${GREEN}$0${OFF}: building the LowFat config..."
cd ${LOWFAT_SRC}/config
./lowfat-config $CONFIG > lowfat-config.log

#echo -e "${GREEN}$0${OFF}: copying the LowFat config files..."
#cd ${LOWFAT_SRC}/config
#cp lowfat_config.h lowfat_config.c /home/odroid/allocator/

echo -e "${GREEN}$0${OFF}: creating liblowfat.preload.so standalone..."
#cd /home/odroid/zomtag-llvm/zometag-alloc/ref/LowFat/llvm-4.0.0.src/projects/compiler-rt/lib/lowfat/jwseo
cd /home/odroid/zomtag-llvm/zometag-alloc/jsyou
$CLANG -D_GNU_SOURCE -DLOWFAT_STANDALONE -DLOWFAT_NO_PROTECT -fPIC -shared -g\
	-o liblowfat.preload.so -std=gnu99 -m64 "-I./" \
	-DLOWFAT_LINUX "./lowfat.c"
