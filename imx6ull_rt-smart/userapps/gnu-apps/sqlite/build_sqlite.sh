#!/bin/bash

# Get initial variables
BUILD_CONF="../conf.sh"
if [ ! -f ${BUILD_CONF} ]; then
  echo "conf.sh file not exist!"
  exit 1;
fi
source ${BUILD_CONF} $1

APP_NAME="sqlite-autoconf"
VERSION="3340000"
APP_DIR=${APP_NAME}-${VERSION}

# Check openssl and zlib exist
LIB_DIR=${ROOTDIR}/sdk/lib
INC_DIR=${ROOTDIR}/sdk/include

RT_DIR=${ROOTDIR}/sdk/rt-thread
RT_INC=" -I. -I${RT_DIR}/include -I${RT_DIR}/components/dfs -I${RT_DIR}/components/drivers -I${RT_DIR}/components/finsh -I${RT_DIR}/components/net -I${INC_DIR}"

export CPPFLAGS=${RT_INC}
export LDFLAGS="-L${LIB_DIR} -L${RT_DIR}/lib"

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
        touch "$DbgFile"
    else
        touch "$TagFile"
    fi
    patch -Np1 -i ../1-*.patch
    patch -Np1 -i ../2-*.patch
    patch -Np1 -i ../3-*.patch
}

function preparesrc() {
    if [ ! -f ${APP_DIR}.tar.gz ]; then
        echo "========= Downloading ${APP_DIR}.tar.gz ========="
        wget https://distfiles.macports.org/sqlite3/${APP_DIR}.tar.gz
    fi
    tar xzf ${APP_DIR}.tar.gz
    cd ${APP_DIR}
    # copy test code
    cp ../sqlite_test/sqlite_test.c .
    cp ../sqlite_test/sqlite_demo.c .
    cp ../art-pi-demo-cgi/login.c .
    cp ../art-pi-demo-cgi/adduser.c .
    cp ../art-pi-demo-cgi/dellog.c .
    cp ../art-pi-demo-cgi/deluser.c .
    cp ../art-pi-demo-cgi/chpass.c .
    cp ../art-pi-demo-cgi/resetpass.c .
    cp ../art-pi-demo-cgi/getuserlist.c .
    cp ../art-pi-demo-cgi/getloglist.c .
    cp ../art-pi-demo-cgi/getsysconf.c .
    cp ../art-pi-demo-cgi/gettime.c .
    cp ../art-pi-demo-cgi/artpi_cgi.c .
    cp ../art-pi-demo-cgi/artpi_cgi.h .
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

    ./configure \
	--prefix=${ROOTDIR}/build \
	--target=${CROSS_COMPILE} \
	--host=${CROSS_COMPILE} \
	--build=i686-pc-linux-gnu \
	--enable-threadsafe=yes
    make clean
    if [ "$2" == "verbose" ]; then
        make V=1
    else
        make
    fi

    # for test code build
    echo "sqlite_test:" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o sqlite_test.o  sqlite_test.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o sqlite_test sqlite_test.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o sqlite_demo.o  sqlite_demo.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o sqlite_demo sqlite_demo.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    make sqlite_test

    # for art-pi-demo-cgi build
    echo "demo_cgi:" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o artpi_cgi.o artpi_cgi.c" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o login.o login.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o login.elf login.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o adduser.o adduser.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o adduser.elf adduser.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o deluser.o deluser.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o deluser.elf deluser.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o chpass.o chpass.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o chpass.elf chpass.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o resetpass.o resetpass.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o resetpass.elf resetpass.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o getuserlist.o getuserlist.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o getuserlist.elf getuserlist.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o getloglist.o getloglist.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o getloglist.elf getloglist.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o getsysconf.o getsysconf.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o getsysconf.elf getsysconf.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o dellog.o dellog.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o dellog.elf dellog.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o gettime.o gettime.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o gettime.elf gettime.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    make demo_cgi

    # for art-pi-demo-cgi/sysinfo.c build
    cd ../art-pi-demo-cgi
    make clean
    make
    cd -
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

    ./configure \
	--prefix=${ROOTDIR}/build \
	--target=${CROSS_COMPILE} \
	--host=${CROSS_COMPILE} \
	--build=i686-pc-linux-gnu \
	--enable-threadsafe=yes
    make clean
    if [ "$1" == "verbose" ]; then
        make V=1
    else
        make
    fi

    # for test code make
    echo "sqlite_test:" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o sqlite_test.o  sqlite_test.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o sqlite_test sqlite_test.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o sqlite_demo.o  sqlite_demo.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o sqlite_demo sqlite_demo.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    make sqlite_test

    # for art-pi-demo-cgi build
    echo "demo_cgi:" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o artpi_cgi.o artpi_cgi.c" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o login.o login.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o login.elf login.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o adduser.o adduser.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o adduser.elf adduser.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o deluser.o deluser.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o deluser.elf deluser.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o chpass.o chpass.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o chpass.elf chpass.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o resetpass.o resetpass.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o resetpass.elf resetpass.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o getuserlist.o getuserlist.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o getuserlist.elf getuserlist.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o getloglist.o getloglist.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o getloglist.elf getloglist.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o getsysconf.o getsysconf.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o getsysconf.elf getsysconf.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o dellog.o dellog.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o dellog.elf dellog.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    echo -e "\t\$(CC) \$(DEFS) \$(DEFAULT_INCLUDES) \$(INCLUDES) \$(AM_CPPFLAGS) \$(CPPFLAGS) \$(sqlite3_CFLAGS) \$(CFLAGS) -c -o gettime.o gettime.c" >> Makefile
    echo -e "\t\$(sqlite3_LINK) -o gettime.elf gettime.o artpi_cgi.o -L.libs -lsqlite3  \$(sqlite3_LDADD) \$(LIBS)" >> Makefile
    make demo_cgi
    
    # for art-pi-demo-cgi/sysinfo.c build
    cd ../art-pi-demo-cgi
    make clean
    make
    cd -

    # strip
    arm-linux-musleabi-strip sqlite3
    arm-linux-musleabi-strip sqlite_test
    arm-linux-musleabi-strip sqlite_demo
    arm-linux-musleabi-strip login.elf
    arm-linux-musleabi-strip adduser.elf
    arm-linux-musleabi-strip dellog.elf
    arm-linux-musleabi-strip deluser.elf
    arm-linux-musleabi-strip chpass.elf
    arm-linux-musleabi-strip resetpass.elf
    arm-linux-musleabi-strip getuserlist.elf
    arm-linux-musleabi-strip getloglist.elf
    arm-linux-musleabi-strip getsysconf.elf
    arm-linux-musleabi-strip gettime.elf
    arm-linux-musleabi-strip ../art-pi-demo-cgi/sysinfo.elf
}


if [ "$1" == "help" ]; then
    usage
    exit 0
elif [ "$1" == "debug" ]; then
    builddbg $1 $2
else
    builddef $1
fi


echo "========= Copy sqlite3.elf and sqlite_test.elf to root/bin ========="
test -f ${ROOTDIR}/root/bin/sqlite3.elf || rm -rf ${ROOTDIR}/root/bin/sqlite3.elf
cp sqlite3 ${ROOTDIR}/root/bin/sqlite3.elf

test -d ${ROOTDIR}/sdk/include || mkdir -p ${ROOTDIR}/sdk/include
test -d ${ROOTDIR}/sdk/lib || mkdir -p ${ROOTDIR}/sdk/lib
rm -rf ${ROOTDIR}/sdk/include/sqlite3ext.h
rm -rf ${ROOTDIR}/sdk/include/sqlite3.h
rm -rf ${ROOTDIR}/sdk/lib/libsqlite3.a

cp sqlite3ext.h sqlite3.h ${ROOTDIR}/sdk/include
cp .libs/libsqlite3.a ${ROOTDIR}/sdk/lib

test -f ${ROOTDIR}/root/bin/sqlite_test.elf || rm -rf ${ROOTDIR}/root/bin/sqlite_test.elf
cp sqlite_test ${ROOTDIR}/root/bin/sqlite_test.elf
test -f ${ROOTDIR}/root/bin/sqlite_demo.elf || rm -rf ${ROOTDIR}/root/bin/sqlite_demo.elf
cp sqlite_demo ${ROOTDIR}/root/bin/sqlite_demo.elf

# var/dist is the home dir for art-pi-demo
test -d ${ROOTDIR}/root/var/dist/cgi-bin || mkdir -p ${ROOTDIR}/root/var/dist/cgi-bin

rm -rf ${ROOTDIR}/root/var/dist/cgi-bin/*.elf
cp ../art-pi-demo-cgi/sysinfo.elf ${ROOTDIR}/root/var/dist/cgi-bin
cp *.elf ${ROOTDIR}/root/var/dist/cgi-bin

echo "========= Sqlite building finished. ========="

