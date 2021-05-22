#pragma once

#include <Eigen/Geometry>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/affine.hpp>
#include <opencv2/core/eigen.hpp>

#include "geometry.h"

class TrackerData {
 public:
  cv::Vec3d tvec;
  cv::Vec3d rvec;
  cv::Affine3d affine3d() const;
  cv::Mat affine3dMat() const;
  Eigen::Quaterniond quaternion() const;

  TrackerData() {}
  TrackerData(const cv::Vec3d& rvec, const cv::Vec3d& tvec)
      : tvec(tvec), rvec(rvec) {}
};

cv::Affine3d getAffine3dFromCameraToVR();
cv::Mat getMatrixFromCameraToVR(TrackerData& hmd, TrackerData& hips,
                                TrackerData& lleg, TrackerData& rleg);
