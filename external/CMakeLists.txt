cmake_minimum_required(VERSION 3.15)

set(PARALLEL_JOBS 32)

project(VCubeTrackerExternal)

include(ExternalProject)

ExternalProject_Add(eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.zip
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Add(opencv_contrib
    URL https://github.com/opencv/opencv_contrib/archive/4.2.0.zip
    URL_HASH SHA256=a4c0b1fb169a68085f7deefdeca8429aafadf3e1798abec0c1a1a7fc83e11a69
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Add(opencv
    URL https://github.com/opencv/opencv/archive/4.2.0.zip
    URL_HASH SHA256=55bd939079d141a50fca74bde5b61b339dd0f0ece6320ec76859aaff03c90d9f
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF
        -DBUILD_opencv_world=ON
        -DOPENCV_EXTRA_MODULES_PATH=${CMAKE_CURRENT_BINARY_DIR}/opencv_contrib-prefix/src/opencv_contrib/modules
        -DWITH_EIGEN=ON
    DEPENDS opencv_contrib eigen
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG> -j ${PRALLEL_JOBS}
)

ExternalProject_Add(openvr
    URL https://github.com/ValveSoftware/openvr/archive/v1.12.5.tar.gz
    URL_HASH SHA256=f3cdbaa946688553638e6d65978f156311c9b08825316198d925f5eade6cfeb7
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Add(oscpack
    URL https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/oscpack/oscpack_1_1_0.zip
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    INSTALL_COMMAND ""
)
