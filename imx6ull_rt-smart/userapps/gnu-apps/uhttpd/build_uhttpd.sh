#!/bin/bash

APP_NAME="uhttpd"
APP_DIR=${APP_NAME}

# userapps is ROOTDIR
ROOTDIR=${PWD}/../..

# Check lua exist
LIB_PATH=${ROOTDIR}/sdk/lib
if [ ! -f ${LIB_PATH}/liblua.a ]; then
        echo "========= Error: not find lua, please build them first! ========="
        exit 1;
fi

export RTT_EXEC_PATH=${ROOTDIR}/../tools/gnu_gcc/arm-linux-musleabi_for_x86_64-pc-linux-gnu/bin
export PATH=$PATH:$RTT_EXEC_PATH:$RTT_EXEC_PATH/arm-linux-musleabi/bin

TagFile=".patched"
DbgFile=".dbg_patched"

function usage() {
  echo "====================="
  echo "Usage: $0 [options] [verbose]"
  echo ""
  echo "  $0 - build without debug info and strip, if build option not same with the exist source dir, backup and re-unzip"
  echo "  $0 verbose - default build with detail log"
  echo "  $0 debug - build with debug info and not strip, if build option not same with the exist source dir, backup and re-unzip"
  echo "  $0 debug verbose - debug build with detail log"
  echo "  $0 help - show help info"
  echo "====================="
}

# create tag file and do patch if has
function dopatch() {
    if [ "$1" == "debug" ]; then
        touch "$DbgFile"
        patch -Np2 -i ../../dbg-1-*.patch
    else
        touch "$TagFile"
        patch -Np2 -i ../../1-*.patch
    fi
    patch -Np2 -i ../../2-*.patch
    patch -Np2 -i ../../3-*.patch
    patch -Np2 -i ../../4-*.patch
    patch -Np2 -i ../../5-*.patch
    patch -Np2 -i ../../6-*.patch
    patch -Np2 -i ../../7-*.patch
}

function preparesrc() {
    if [ ! -f ${APP_DIR}.tar.gz ]; then
        echo "Please download uhttpd source first."
	exit 1
    fi
    tar xzf ${APP_DIR}.tar.gz
    cd ${APP_DIR}/src
}

# debug build
function builddbg() {
    if [ -d ${APP_DIR} ] && [ -f ${APP_DIR}/src/${DbgFile} ]; then
        cd ${APP_DIR}/src
    else
        if [ -d ${APP_DIR} ]; then
            # bank old source dir, default build
            mv ${APP_DIR} ${APP_DIR}.$(date "+%Y%m%d%H%M%S")
        fi
        preparesrc
        dopatch $1
    fi

    make clean
    if [ "$2" == "verbose" ]; then
        make V=1
    else
        make
    fi
}

# default build
function builddef() {
    if [ -d ${APP_DIR} ] && [ -f ${APP_DIR}/src/${TagFile} ]; then
        cd ${APP_DIR}/src
    else
        if [ -d ${APP_DIR} ]; then
            # bank old source dir, debug build
            mv ${APP_DIR} ${APP_DIR}.$(date "+%Y%m%d%H%M%S").dbg
        fi
        preparesrc
        dopatch
    fi

    make clean
    if [ "$1" == "verbose" ]; then
        make V=1
    else
        make
    fi
    # strip
    arm-linux-musleabi-strip uhttpd
}


if [ "$1" == "help" ]; then
    usage
    exit 0
elif [ "$1" == "debug" ]; then
    builddbg $1 $2
else
    builddef $1
fi


echo "========= Copy uhttpd to root/bin ========="
test -f ${ROOTDIR}/root/bin/uhttpd.elf || rm -rf ${ROOTDIR}/root/bin/uhttpd.elf
cp uhttpd ${ROOTDIR}/root/bin/uhttpd.elf
echo "========= uHttpd building finished. ========="
