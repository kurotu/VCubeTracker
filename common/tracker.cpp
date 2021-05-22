#include "tracker.h"

#include "geometry.h"

cv::Affine3d TrackerData::affine3d() const {
  cv::Affine3d affine(this->rvec, this->tvec);
  return affine;
}

cv::Mat TrackerData::affine3dMat() const {
  cv::Mat m(this->affine3d().matrix);
  return m;
}

Eigen::Quaterniond TrackerData::quaternion() const {
  auto q = rvecToQuaternion(this->rvec);
  return std::move(q);
}

cv::Affine3d getAffine3dFromCameraToVR() {
  return getAffine3d(0.0, 0.0, CV_PI);  // —¼•û‰EŽèŒn, ZŽ²‰ñ“]
}

cv::Mat getMatrixFromCameraToVR(TrackerData& hmd, TrackerData& hips,
                                TrackerData& lleg, TrackerData& rleg) {
  TrackerData center = {(lleg.tvec + rleg.tvec) / 2.0,
                        (lleg.rvec + rleg.rvec) / 2.0};
  cv::Mat result;
  return result;
}
