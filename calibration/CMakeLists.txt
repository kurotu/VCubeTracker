add_executable(calibrate_camera_charuco calibrate_camera_charuco.cpp)
target_link_libraries(calibrate_camera_charuco common ${OpenCV_LIBS})
install_bin_target(calibrate_camera_charuco)
