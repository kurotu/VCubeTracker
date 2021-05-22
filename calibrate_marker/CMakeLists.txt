add_executable(calibrate_marker
    calibrate_marker.cpp
    MarkerCalibrator.h MarkerCalibrator.cpp
)
target_link_libraries(calibrate_marker common ${OpenCV_LIBS})
install_bin_target(calibrate_marker)
