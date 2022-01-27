#!/bin/bash 

libs_arr=( "lua" "zlib" "sdl")
apps_arr=("uhttpd" "sqlite")


function usage(){
  echo "====================="
  echo "Usage: ./build.sh [options]"
  echo ""
  echo "Options:"
  echo "  all  - build all libs and apps"
  echo "  libs - build all libs"
  echo "  apps - build all apps  "
  echo "  lib [pkgname] - build the [pkgname] lib only, Options: ${libs_arr[@]}"
  echo "  app [pkgname] - build the [pkgname] app only, Options: ${apps_arr[@]}"
  echo "  clean - clean all *.a *.h and binary"
  echo ""
  echo "Examples:"
  echo "  ./build.sh all - build all libs and apps"
  echo "  ./build.sh libs - build all libs: ${libs_arr[@]}"
  echo "  ./build.sh apps - build all apps: ${apps_arr[@]}"
  echo "  ./build.sh lib zlib - build zlib only"
  echo "  ./build.sh app wget - build wget only"
  echo "  ./build.sh clean - clean all"
  echo "====================="
}

function make_clean(){
  # clean headers and libs
  rm -rf ../sdk/include/*
  rm -rf ../sdk/lib/*

  # clean binaries
  rm ../root/bin/curl.elf
  rm ../root/bin/lighttpd.elf
  rm ../root/bin/lua.elf
  rm ../root/bin/qjsc.elf
  rm ../root/bin/uhttpd.elf
  rm ../root/bin/wget.elf
  rm ../root/bin/qjs.elf
  rm ../root/bin/sqlite3.elf
  rm ../root/bin/busybox.elf

  echo "====== clean done ======"
}

function make_libs(){
  for lib in ${libs_arr[@]}
    do
      cd $lib
      bash build_$lib.sh
      cd ..
    done
}

function make_apps(){
  for app in ${apps_arr[@]}
    do
      cd $app
      bash build_$app.sh
      cd ..
    done
}

function contains(){
    local n=$#
    local value=${!n}
    for ((i=1;i < $#;i++)) {
        if [ "${!i}" == "${value}" ]; then
            echo "y"
            return 0
        fi
    }
    echo "n"
    return 1
}

if [ $# == 0 ]; then
  usage
  exit 1
elif [ $# == 1 ]; then
  if [ $1 == "all" ]; then
    make_clean
    make_libs
    make_apps
  elif [ $1 == "libs" ]; then
    make_libs
  elif [ $1 == "apps" ]; then
    make_apps
  elif [ $1 == "clean" ]; then
    make_clean
  else
    usage
  fi
else
  if [ $1 == "lib" ]; then
    if [ $(contains "${libs_arr[@]}" "$2") == "y" ]; then
      cd $2
      bash ./build_$2.sh
      cd ..
    else
      echo "[ERROR] libs options: ${libs_arr[@]}"
    fi
  elif [ $1 == "app" ]; then
    if [ $(contains "${apps_arr[@]}" "$2") == "y" ]; then
      cd $2
      bash ./build_$2.sh
      cd ..
    else
      echo "[ERROR] apps options: ${apps_arr[@]}"
    fi
  else
    usage
  fi
fi

