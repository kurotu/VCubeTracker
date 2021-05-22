#pragma once

#include "ArUcoBoard.h"
#include "CameraCapture.h"
#include "HighguiCapture.h"
#include "LowPassFilter.h"
#include "MarkerCube.h"
#include "concurrent_queue.h"
#include "geometry.h"
#include "tracker.h"

std::string createTrackerData(const float hips_pos[3], const float hips_rot[4],
                              const float lleg_pos[3], const float lleg_rot[4],
                              const float rleg_pos[3], const float rleg_rot[4]);
