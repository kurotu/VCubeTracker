#pragma once

#include <Eigen/Geometry>
#include <opencv2/core.hpp>
#include <opencv2/core/affine.hpp>

cv::Affine3d getAffine3dToOrigin(const cv::Vec3d& hips, const cv::Vec3d& lleg,
                                 const cv::Vec3d& rleg);

template <typename FloatType>
Eigen::AngleAxis<FloatType> rvecToAngleAxis(const cv::Vec<FloatType, 3>& rvec) {
  Eigen::Matrix<FloatType, 3, 1> axis;
  cv::cv2eigen(rvec, axis);
  auto theta = axis.norm();
  axis.normalize();
  return std::move(Eigen::AngleAxis<FloatType>(theta, axis));
}

template <typename FloatType>
Eigen::Quaternion<FloatType> rvecToQuaternion(
    const cv::Vec<FloatType, 3>& rvec) {
  auto angleAxis = rvecToAngleAxis(rvec);
  return std::move(Eigen::Quaternion<FloatType>(angleAxis));
  /*
  cv::Mat cvRot;
  cv::Rodrigues(rvec, cvRot);
  Eigen::Matrix3d eRot;
  cv::cv2eigen(cvRot, eRot);
  return std::move(Eigen::Quaternion<FloatType>(eRot));
  */
}

template <typename FloatType>
cv::Vec<FloatType, 3> quaternionToRvec(
    const Eigen::Quaternion<FloatType>& quaternion) {
  Eigen::AngleAxis<FloatType> angleAxis(quaternion);
  Eigen::Matrix<FloatType, 3, 1> v = angleAxis.axis() * angleAxis.angle();
  cv::Vec<FloatType, 3> result;
  cv::eigen2cv(v, result);
  return std::move(result);
}

template <typename FloatType>
cv::Mat getRotationMatrix3D(FloatType role, FloatType pitch, FloatType yaw) {
  cv::Mat x = (cv::Mat_<FloatType>(3, 3) << 1.0, 0.0, 0.0, 0.0, std::cos(role),
               -std::sin(role), 0.0, std::sin(role), std::cos(role));
  cv::Mat y =
      (cv::Mat_<FloatType>(3, 3) << std::cos(pitch), 0.0, std::sin(pitch), 0.0,
       1.0, 0.0, -std::sin(pitch), 0.0, std::cos(pitch));
  cv::Mat z = (cv::Mat_<FloatType>(3, 3) << std::cos(yaw), -std::sin(yaw), 0.0,
               std::sin(yaw), std::cos(yaw), 0.0, 0.0, 0.0, 1.0);
  return z * y * x;
}

template <typename FloatType>
cv::Affine3<FloatType> getAffine3d(FloatType role, FloatType pitch,
                                   FloatType yaw) {
  auto rot = getRotationMatrix3D(role, pitch, yaw);
  cv::Affine3<FloatType> affine(rot);
  return affine;
}

double getAreaFromCorners(const std::vector<cv::Point2f>& corners);
double getAreaFromCorners(const std::vector<cv::Vec3d>& corners);
