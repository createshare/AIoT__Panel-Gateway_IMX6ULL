#!/bin/sh

# userapps is ROOTDIR
ROOTDIR=${PWD}/../..

# /var/dist is the web home dir
test -d ${ROOTDIR}/root/var/dist || mkdir -p ${ROOTDIR}/root/var/dist

# bak cgi-bin dir
if [ -d ${ROOTDIR}/root/var/dist/cgi-bin ]; then
    rm -rf /tmp/cgi-bin
    mv ${ROOTDIR}/root/var/dist/cgi-bin /tmp
fi

rm -rf ${ROOTDIR}/root/var/dist/*
cp -r dist/* ${ROOTDIR}/root/var/dist
mv /tmp/cgi-bin ${ROOTDIR}/root/var/dist


