cmake_minimum_required(VERSION 3.15)

project(VCubeTracker)
set(CMAKE_CXX_STANDARD 17)

include(GoogleTest.cmake)

set(EXTERNAL_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/build")

#set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
set(Eigen3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/build/eigen-prefix/share/eigen3/cmake")
find_package (Eigen3 3.3 REQUIRED NO_MODULE)
#message(eigen: ${EIGEN3_INCLUDE_DIR})
#include_directories(${EIGEN3_INCLUDE_DIR})
#include_directories(${Eigen3_DIR}/include/eigen3)

set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/build/opencv-prefix")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/vendor/ewclib")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/vendor/winsdk6.1")

include(GoogleTest)
enable_testing()

function(install_bin_target target)
    install(TARGETS ${target}
            CONFIGURATIONS Debug
            RUNTIME DESTINATION Debug/bin)
    install(TARGETS ${target}
            CONFIGURATIONS Release
            RUNTIME DESTINATION Release/bin)
endfunction()

add_subdirectory(common)
include_directories(common)

add_subdirectory(vcube_tracker)
add_subdirectory(calibration)
add_subdirectory(calibrate_marker)
add_subdirectory(create_marker)
add_subdirectory(cv_samples)
# add_subdirectory(test)
add_subdirectory(vmt_samples)

install(FILES
    "${EXTERNAL_BUILD_DIR}/opencv-prefix/x64/vc16/bin/opencv_world420.dll"
    "${EXTERNAL_BUILD_DIR}/openvr-prefix/src/openvr/bin/win64/openvr_api.dll"
    CONFIGURATIONS Debug
    RUNTIME DESTINATION Debug/bin
)

install(FILES
    "${EXTERNAL_BUILD_DIR}/opencv-prefix/x64/vc16/bin/opencv_world420.dll"
    "${EXTERNAL_BUILD_DIR}/openvr-prefix/src/openvr/bin/win64/openvr_api.dll"
    CONFIGURATIONS Release
    RUNTIME DESTINATION Release/bin
)
