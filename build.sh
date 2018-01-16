#!/bin/sh

set -x                                #set去追踪一段代码的显示情况，执行后在整个脚本有效   set -x 开启

SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-../build}                        #  :-符号:  build_dir 为空则等于 ../build
BUILD_TYPE=${BUILD_TYPE:-release}                       #  默认为release
INSTALL_DIR=${INSTALL_DIR:-../${BUILD_TYPE}-install}    #  默认为../release-install
BUILD_NO_EXAMPLES=${BUILD_NO_EXAMPLES:-0}               #  默认为0

mkdir -p $BUILD_DIR/$BUILD_TYPE \                       #  默认为../build/release
  && cd $BUILD_DIR/$BUILD_TYPE \
  && cmake \
           -DCMAKE_BUILD_TYPE=$BUILD_TYPE \             #  默认为-DCMAKE_BUILD_TYPE = release
           -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \        #  默认为-DCMAKE_INSTALL_PREFIX =  ../release-install
           -DCMAKE_BUILD_NO_EXAMPLES=$BUILD_NO_EXAMPLES \   # 默认为-DCMAKE_BUILD_NO_EXAMPLES = 0
           $SOURCE_DIR \
  && make $*

# Use the following command to run all the unit tests
# at the dir $BUILD_DIR/$BUILD_TYPE :
# CTEST_OUTPUT_ON_FAILURE=TRUE make test

# cd $SOURCE_DIR && doxygen

