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
SRC=
MAIN=
MALLOC=
OUT=
CONFIGC=
CONFIGH=
CONFIG="sizes.cfg 32"

if [ $# -le 1 ]
then
	echo -e "${RED}$0${OFF}: need to provide an argument..."
	echo -e "${RED}$0${OFF}: usage: ./build.sh [type] [test]"
	echo -e "${RED}$0${OFF}: [type]"
	echo -e "${RED}$0${OFF}: backup (make backup zometag allocator)"
	echo -e "${RED}$0${OFF}: default (make default zometag allocator)"
	echo -e "${RED}$0${OFF}: random  (make random zometag allocator)"
	echo -e "${RED}$0${OFF}: test (make test zometag allocator)"
	echo -e ""
	echo -e "${RED}$0${OFF}: [test]"
	echo -e "${RED}$0${OFF}: yes (make allocator w/ 61 regions)"
	echo -e "${RED}$0${OFF}: no (make complete allocator with all regions)"
	echo -e ""
	exit
fi

if [ $1 = default ]
then
	CONFIG="sizes.cfg 32"
	MAIN="lowfat_fixed.c"
	MALLOC="lowfat_malloc_fixed.c"
	OUT="libzometag_fixed.preload.so"
elif [ $1 = backup ]
then
	MAIN="lowfat_fixed.c"
	MALLOC="lowfat_malloc_backup.c"
	OUT="libzometag_backup.preload.so"
elif [ $1 = random ]
then
	MAIN="lowfat_random.c"
	MALLOC="lowfat_malloc_random.c"
	OUT="libzometag_random.preload.so"
elif [ $1 = test ]
then
	MAIN="lowfat_test.c"
	MALLOC="lowfat_malloc_test.c"
	OUT="libzometag_test.preload.so"
else
	echo -e "${RED}$0${OFF}: invalid argument..."
	exit
fi

if [ $2 = yes ]
then
	CONFIGC="lowfat_config_64.c"
	CONFIGH="lowfat_config_64.h"
elif [ $2 = no ]
then
	CONFIGC="lowfat_config_zometag.c"
	CONFIGH="lowfat_config_zometag.h"
fi

#echo -e "${GREEN}$0${OFF}: using the default LowFat configuration... ($CONFIG)..."
#echo -e "${GREEN}$0${OFF}: building the Lowfat config builder..."
#(cd config; CC=$CLANG CFLAGS="-std=gnu99" CXX=$CLANGXX make >/dev/null)
#echo -e "${GREEN}$0${OFF}: building the LowFat config..."
#(cd config; ./lowfat-config $CONFIG > lowfat-config.log)

echo -e "${GREEN}$0${OFF}: copying the LowFat config files..."
cp -f src/${CONFIGC} ./lowfat_config.c
cp -f src/${CONFIGH} ./lowfat_config.h

echo -e "${GREEN}$0${OFF}: copying the Lowfat malloc files..."
cp -f src/${MAIN} ./lowfat.c
cp -f src/${MALLOC} ./lowfat_malloc.c

echo -e "${GREEN}$0${OFF}: creating liblowfat.preload.so standalone..."
$CLANG -D_GNU_SOURCE -DLOWFAT_STANDALONE -DLOWFAT_NO_PROTECT -fPIC -shared \
	-o ${OUT} -std=gnu99 -m64 "-I./" \
	-DLOWFAT_LINUX  -O2 "./lowfat.c"

echo -e "${GREEN}$0${OFF}: clean up..."
(rm lowfat.c lowfat_malloc.c lowfat_config.h lowfat_config.c)
