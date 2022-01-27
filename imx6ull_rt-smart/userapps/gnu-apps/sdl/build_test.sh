#!/bin/bash

# Get initial variables
BUILD_CONF="../conf.sh"
if [ ! -f ${BUILD_CONF} ]; then
  echo "conf.sh file not exist!"
  exit 1;
fi
source ${BUILD_CONF} $1

APP_NAME="SDL2"
VERSION="2.0.14"
APP_DIR=${APP_NAME}-${VERSION}

LIB_DIR=${ROOTDIR}/sdk/lib
INC_DIR=${ROOTDIR}/sdk/include/sdl
RT_DIR=${ROOTDIR}/sdk/rt-thread

RT_INC="-I${APP_DIR}/include -I${ROOTDIR}/apps/${APP_DIR} -I${RT_DIR}/include -I${RT_DIR}/components/dfs -I${RT_DIR}/components/drivers -I${RT_DIR}/components/finsh -I${RT_DIR}/components/net -I${INC_DIR}"

export CFLAGS=${RT_INC}
export LIBS=" -L${LIB_DIR}  ${RT_INC} -T ${ROOTDIR}/linker_scripts/arm/cortex-a/link.lds -march=armv7-a -marm -msoft-float -L${RT_DIR}/lib -Wl,--whole-archive -lrtthread -Wl,--no-whole-archive -n -static -Wl,--start-group -lc -lgcc -lrtthread -Wl,--end-group -lSDL2"

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
    else
        touch "$TagFile"
    fi
}

# debug build
function builddbg() {
    if [ -d ${APP_DIR}/test ] && [ -f ${APP_DIR}/test/${DbgFile} ]; then
        cd ${APP_DIR}/test
    else
        if [ -f ${APP_DIR}/test/${TagFile} ]; then
            rm ${APP_DIR}/test/${TagFile}
        fi
        cd ${APP_DIR}/test
        dopatch $1
    fi

    ./configure \
	--target=${CROSS_COMPILE} \
	--host=${CROSS_COMPILE} \
	--build=i686-pc-linux-gnu \
	--with-sdl-prefix=${LIB_DIR} 
    make clean
    if [ "$2" == "verbose" ]; then
        make V=1
    else
        make
    fi
}

# default build
function builddef() {
    if [ -d ${APP_DIR}/test ] && [ -f ${APP_DIR}/test/${TagFile} ]; then
        cd ${APP_DIR}/test
    else
        if [ -f ${APP_DIR}/test/${DbgFile} ]; then
            rm ${APP_DIR}/test/${DbgFile}
        fi
	cd ${APP_DIR}/test
        dopatch
    fi

    ./configure \
	--target=${CROSS_COMPILE} \
	--host=${CROSS_COMPILE} \
	--build=i686-pc-linux-gnu \
	--with-sdl-prefix=${LIB_DIR} 
    make clean
    if [ "$1" == "verbose" ]; then
        make V=1
    else
        make
    fi
    # strip
    arm-linux-musleabi-strip testrtt
}


if [ "$1" == "help" ]; then
    usage
    exit 0                                                                                                 
elif [ "$1" == "debug" ]; then
    builddbg $1 $2
else
    builddef $1
fi

echo "========= Copy sdl_test.elf and test.bmp to ${ROOTDIR}/root/bin ========="
cp testrtt ${ROOTDIR}/root/bin/sdl_test.elf
cd ../..
cp test.bmp ${ROOTDIR}/root/bin/
echo "======== Finished building sdl_test ========"
