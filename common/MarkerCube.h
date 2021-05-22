#pragma once

#include <opencv2/core/affine.hpp>

#include "tracker.h"

cv::Affine3d estimateConvert(const std::vector<cv::Vec3d>& targetRvec,
                             const std::vector<cv::Vec3d>& targetTvec,
                             const std::vector<cv::Vec3d>& originalRvec,
                             const std::vector<cv::Vec3d>& originalTvec);

cv::Affine3d estimateConvert(const TrackerData& target,
                             const TrackerData& original);

class MarkerCube {
 private:
  int keyId;
  void convertToKeyPose(int id, const cv::Vec3d& rvec, const cv::Vec3d& tvec,
                        cv::Vec3d& out_rvec, cv::Vec3d& out_tvec);

 public:
  std::array<cv::Affine3d, 4> affines;
  MarkerCube(int keyId) : keyId(keyId) {}
  void convertToKeyPose(int id, const TrackerData& pose, TrackerData& out_pose);
  void saveParams(cv::FileStorage& storage);
  void loadParams(cv::FileNode& storage);
};
