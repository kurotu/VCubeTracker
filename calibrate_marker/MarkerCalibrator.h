#pragma once

#include <deque>

#include "common.h"

class MarkerCalibratorData {
 public:
  TrackerData keyPose;
  TrackerData originalPose;
};

class MarkerCalibrator {
 private:
  const int maxBuffer = 10;
  const int keyId;
  std::array<std::deque<MarkerCalibratorData>, 4> buffers;
  cv::Affine3d calibrate(int index);
  std::array<cv::Affine3d, 4> calibrate_();

 public:
  MarkerCalibrator(int keyId) : keyId(keyId) {}
  void pushData(const TrackerData& keyPose, int id, const TrackerData& subPose);
  MarkerCube calibrate();
  bool isFullFilled();
};
