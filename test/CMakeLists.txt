add_executable(common_test
    gtest_sample.cpp
    affine_test.cpp
    tracker_test.cpp
    eigen_test.cpp
    MarkerCube_test.cpp
    LowPassFilter_test.cpp
)
target_link_libraries(common_test common gtest_main ${OpenCV_LIBS})
gtest_discover_tests(common_test)
