#!/bin/bash

# Get initial variables
BUILD_CONF="../conf.sh"
if [ ! -f ${BUILD_CONF} ]; then
  echo "conf.sh file not exist!"
  exit 1;
fi
source ${BUILD_CONF} $1

APP_NAME="zlib"
VERSION="1.2.8"
APP_DIR=${APP_NAME}-${VERSION}


RT_DIR=${ROOTDIR}/sdk/rt-thread
RT_INC=" -I. -I${RT_DIR}/include -I${RT_DIR}/components/dfs -I${RT_DIR}/components/drivers -I${RT_DIR}/components/finsh -I${RT_DIR}/components/net"
CFLAGS+=${RT_INC}
export CFLAGS

TagFile=".patched"
DbgFile=".dbg_patched"

export TARGETMACH=${CROSS_COMPILE}
export BUILDMACH=i686-pc-linux-gnu

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
    else
        touch "$TagFile"
    fi
}

function preparesrc() {
    if [ ! -f ${APP_DIR}.tar.gz ]; then
        echo "========= Downloading ${APP_DIR}.tar.gz ========="
        wget http://117.143.63.254:9012/www/rt-smart/packages/${APP_DIR}.tar.gz
    fi
    tar xzf ${APP_DIR}.tar.gz
    cd ${APP_DIR}
}

# debug build
function builddbg() {
    if [ -d ${APP_DIR} ] && [ -f ${APP_DIR}/${DbgFile} ]; then
        cd ${APP_DIR}
    else
        if [ -d ${APP_DIR} ]; then
            # bank old source dir, default build
            mv ${APP_DIR} ${APP_DIR}.$(date "+%Y%m%d%H%M%S")
        fi
        preparesrc
        dopatch $1
    fi

    ./configure --prefix=${ROOTDIR}/build --static
    make clean
    if [ "$2" == "verbose" ]; then
        make V=1
    else
        make
    fi
}

# default build
function builddef() {
    if [ -d ${APP_DIR} ] && [ -f ${APP_DIR}/${TagFile} ]; then
        cd ${APP_DIR}
    else
        if [ -d ${APP_DIR} ]; then
            # bank old source dir, debug build
            mv ${APP_DIR} ${APP_DIR}.$(date "+%Y%m%d%H%M%S").dbg
        fi
        preparesrc
        dopatch
    fi

    ./configure --prefix=${ROOTDIR}/build --static
    make clean
    if [ "$1" == "verbose" ]; then
        make V=1
    else
        make
    fi
}


if [ "$1" == "help" ]; then
    usage
    exit 0
elif [ "$1" == "debug" ]; then
    builddbg $1 $2
else
    builddef $1
fi


echo "========= copy .h .a to sdk path ========="
test -d ${ROOTDIR}/sdk/include || mkdir -p ${ROOTDIR}/sdk/include
test -d ${ROOTDIR}/sdk/lib || mkdir -p ${ROOTDIR}/sdk/lib

rm -rf ${ROOTDIR}/sdk/include/zconf.h
rm -rf ${ROOTDIR}/sdk/include/zlib.h
rm -rf ${ROOTDIR}/sdk/lib/libz.a

cp zconf.h zlib.h ${ROOTDIR}/sdk/include
cp libz.a ${ROOTDIR}/sdk/lib

echo "========= Zlib building finished. ========="
