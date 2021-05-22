add_executable(vmt_send vmt_send.cpp)
target_include_directories(vmt_send PRIVATE
    "${EXTERNAL_BUILD_DIR}/oscpack-prefix/src/oscpack"
)
target_link_directories(vmt_send
    PUBLIC
    "${EXTERNAL_BUILD_DIR}/oscpack-prefix/src/oscpack-build/Release"
)
target_link_libraries(vmt_send oscpack Winmm Ws2_32)
