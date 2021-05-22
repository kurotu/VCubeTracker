#!/bin/env bash
set -eu
EXTERNAL_BUILD_DIR="$(pwd)/external/build"
export PATH="${PATH}:${EXTERNAL_BUILD_DIR}/libzmq-prefix/bin:${EXTERNAL_BUILD_DIR}/opencv-prefix/x64/vc16/bin"
cd build
cmake --build . --config Release --target common_test
./test/Release/common_test
