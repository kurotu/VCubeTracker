add_library(common common.h
    geometry.h geometry.cpp
    tracker.h tracker.cpp
    CameraCaptureBase.h
    CameraCapture.h CameraCapture.cpp
    MarkerCube.h MarkerCube.cpp
    HighguiCapture.h HighguiCapture.cpp
    LowPassFilter.h LowPassFilter.cpp
    ArUcoBoard.h ArUcoBoard.cpp
    concurrent_queue.h
)
target_link_libraries(common Eigen3::Eigen)
