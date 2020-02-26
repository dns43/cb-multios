#!/usr/bin/env bash

set -e

# Root cb-multios directory
DIR=$(cd "$(dirname ${BASH_SOURCE[0]})" && pwd)
TOOLS="$DIR/tools"

# Install necessary python packages
if ! /usr/bin/env python -c "import xlsxwriter; import Crypto" 2>/dev/null; then
    echo "Please install required python packages" >&2
    echo "  $ sudo pip install xlsxwriter pycrypto" >&2
    exit 1
fi

  echo "Creating build directory"
  mkdir -p ${DIR}/build
  cd ${DIR}/build

  echo "Creating Makefiles"
  CMAKE_OPTS="${CMAKE_OPTS} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"

CMAKE_OPTS="$CMAKE_OPTS -DEMIT_NEGOTIATION=ON"
CMAKE_OPTS="$CMAKE_OPTS -DEMIT_PAYLOAD=ON"
CMAKE_OPTS="$CMAKE_OPTS -DEMIT_COMM=ON"

if [ "$1" == 'cb' ] || [ "$1" == '' ]; then

  echo 'Creating Makefile for CBs instrumented with GRSAN...'

  CC=/local/dennis/new//gradtest/llvm-7.0.0.build/`uname -m`/llvm-7.0/bin/bin/clang  #${CC:-clang}
  CXX=/local/dennis/new/gradtest/llvm-7.0.0.build/`uname -m`/llvm-7.0/bin/bin/clang++  #${CXX:-clang++}

  CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_C_COMPILER=$CC"
  CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_ASM_COMPILER=$CC"
  CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_CXX_COMPILER=$CXX"

  LINK=${LINK:-SHARED}
  case $LINK in
      SHARED) CMAKE_OPTS="$CMAKE_OPTS -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF";;
      STATIC) CMAKE_OPTS="$CMAKE_OPTS -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON";;
  esac
  cmake $CMAKE_OPTS ..
  echo '... DONE creating Makefile for CBs instrumented with GRSAN'
  echo 'Invoke "cd pov/ && merge.sh <target>" to build target and copy the POVs into BUILD'

fi
if [ "$1" == 'pov' ] || [ "$1" == '' ]; then
  mkdir -p ${DIR}/pov
  cd ${DIR}/pov
  CC=clang
  CXX=clang++

  CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_C_COMPILER=$CC"
  CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_ASM_COMPILER=$CC"
  CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_CXX_COMPILER=$CXX"

  LINK=${LINK:-SHARED}
  case $LINK in
      SHARED) CMAKE_OPTS="$CMAKE_OPTS -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF";;
      STATIC) CMAKE_OPTS="$CMAKE_OPTS -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON";;
  esac
  cmake $CMAKE_OPTS ..
  echo '... DONE creating Makefile for uninstrumented POVs'
  echo 'Invoke "make.sh <target>" to build target and copy the POVs into BUILD'
fi
