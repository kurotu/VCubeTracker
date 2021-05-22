add_executable(vcube_tracker vcube_tracker.cpp
    fake_tracker.h fake_tracker.cpp
    openvr_util.h openvr_util.cpp
    ThreadPool.h
)
target_include_directories(vcube_tracker PRIVATE
    "${EXTERNAL_BUILD_DIR}/openvr-prefix/src/openvr/headers"
    "${EXTERNAL_BUILD_DIR}/oscpack-prefix/src/oscpack"
)
target_link_directories(vcube_tracker
    PUBLIC
    "${EXTERNAL_BUILD_DIR}/openvr-prefix/src/openvr/lib/win64"
    "${EXTERNAL_BUILD_DIR}/oscpack-prefix/src/oscpack-build/Release"
#    "${EXTERNAL_BUILD_DIR}/oscpack-prefix/src/oscpack-build/Debug"
)
target_link_libraries(vcube_tracker common ${OpenCV_LIBS} openvr_api oscpack Winmm ws2_32)
install_bin_target(vcube_tracker)
