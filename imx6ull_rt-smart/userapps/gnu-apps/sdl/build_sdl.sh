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
INC_DIR=${ROOTDIR}/sdk/include

RT_DIR=${ROOTDIR}/sdk/rt-thread
RT_INC=" -I. -Iinclude -I${ROOTDIR} -I${RT_DIR}/include -I${RT_DIR}/components/dfs -I${RT_DIR}/components/drivers -I${RT_DIR}/components/finsh -I${RT_DIR}/components/net -I${INC_DIR} -DHAVE_CCONFIG_H"
RT_INC+=" -I${ROOTDIR}/../kernel/bsp/qemu-vexpress-a9/drivers/"

export CPPFLAGS=${RT_INC}
export LDFLAGS="-L${LIB_DIR} "

export LIBS="-T ${ROOTDIR}/linker_scripts/arm/cortex-a/link.lds -march=armv7-a -marm -msoft-float -L${RT_DIR}/lib -Wl,--whole-archive -lrtthread -Wl,--no-whole-archive -n -static -Wl,--start-group -lc -lgcc -lrtthread -Wl,--end-group"

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
        touch ${APP_DIR}/${DbgFile}
    else
        touch ${APP_DIR}/${TagFile}
    fi
    patch -Np1 -i 3-*.diff
    patch -Np1 -i 4-*.diff
    patch -Np1 -i 5-*.diff
    patch -Np1 -i 6-*.diff
    patch -Np1 -i 7-*.diff
    patch -Np1 -i 8-*.diff
}

function preparesrc() {
    if [ ! -f ${APP_DIR}.tar.gz ]; then
        echo "========= Downloading ${APP_DIR}.tar.gz ========="
        wget http://117.143.63.254:9012/www/rt-smart/packages/${APP_DIR}.tar.gz
    fi
    tar xzf ${APP_DIR}.tar.gz
}

# debug build
function builddbg() {
    if [ -d ${APP_DIR} ] && [ -f ${APP_DIR}/${DbgFile} ]; then
        echo "find ${APP_DIR}"
    else
        if [ -d ${APP_DIR} ]; then
            # bank old source dir, default build
            mv ${APP_DIR} ${APP_DIR}.$(date "+%Y%m%d%H%M%S")
        fi
        preparesrc
        dopatch $1
    fi

    cd ${APP_DIR}
    ./autogen.sh
    ./configure \
	--prefix=${ROOTDIR}/build \
	--target=${CROSS_COMPILE} \
	--host=${CROSS_COMPILE} \
	--build=i686-pc-linux-gnu \
	--enable-joystick-virtual=no \
	--enable-render-d3d=no \
	--enable-sdl-dlopen=no \
	--enable-joystick=no \
	--enable-joystick-mfi=no \
	--enable-hidapi=no \
	--enable-hidapi-libusb=no \
	--enable-shared=no \
	--enable-threads=no \
	--enable-3dnow=no \
	--enable-jack-shared=no \
	--enable-pulseaudio-shared=no \
	--enable-pulseaudio=no \
	--enable-cpuinfo=no \
	--enable-video-directfb \
	--enable-directfb-shared=no
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
        echo "find ${APP_DIR}"
    else
        if [ -d ${APP_DIR} ]; then
            # bank old source dir, debug build
            mv ${APP_DIR} ${APP_DIR}.$(date "+%Y%m%d%H%M%S").dbg
        fi
        preparesrc
        dopatch
    fi

    cd ${APP_DIR}
    ./autogen.sh
    ./configure \
	--prefix=${ROOTDIR}/build \
	--target=${CROSS_COMPILE} \
	--host=${CROSS_COMPILE} \
	--build=i686-pc-linux-gnu \
	--enable-joystick-virtual=no \
	--enable-render-d3d=no \
	--enable-sdl-dlopen=no \
	--enable-joystick=no \
	--enable-joystick-mfi=no \
	--enable-hidapi=no \
	--enable-hidapi-libusb=no \
	--enable-shared=no \
	--enable-threads=no \
	--enable-3dnow=no \
	--enable-jack-shared=no \
	--enable-pulseaudio-shared=no \
	--enable-pulseaudio=no \
	--enable-cpuinfo=no \
	--enable-video-directfb \
	--enable-directfb-shared=no
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


echo "========= Copy libSDL2.a and .h to sdk path ========="
test -d ${ROOTDIR}/sdk/include/sdl || mkdir -p ${ROOTDIR}/sdk/include/sdl
test -d ${ROOTDIR}/sdk/include || mkdir -p ${ROOTDIR}/sdk/include
test -d ${ROOTDIR}/sdk/lib || mkdir -p ${ROOTDIR}/sdk/lib

cp include/*.h ${ROOTDIR}/sdk/include/sdl
cp ./build/.libs/libSDL2.a ${ROOTDIR}/sdk/lib
cp ./build/.libs/libSDL2_test.a ${ROOTDIR}/sdk/lib

cd ..
./build_test.sh

echo "========= sdl building finished. ========="
