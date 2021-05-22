#include "geometry.h"

#include <Eigen/Eigen>
#include <opencv2/core/affine.hpp>
#include <opencv2/core/eigen.hpp>

// 姿勢を原点に移動させる変換を計算する。キャリブレーション用
cv::Affine3d getAffine3dToOrigin(const cv::Vec3d& hips, const cv::Vec3d& lleg,
                                 const cv::Vec3d& rleg) {
  const auto base = (rleg + lleg) / 2.0;
  Eigen::Vector3d llegVector, rlegVector, hipsVector;
  cv::cv2eigen(hips, hipsVector);
  cv::cv2eigen(lleg, llegVector);
  cv::cv2eigen(rleg, rlegVector);
  const Eigen::Vector3d legsVectorL2R = rlegVector - llegVector;
  const Eigen::Vector3d baseVector = (rlegVector + llegVector) / 2.0;
  const Eigen::Vector3d legsVectorN = legsVectorL2R.normalized();
  const Eigen::Vector3d hipsNormalV =
      ((llegVector - hipsVector).dot(legsVectorN)) * legsVectorN -
      (llegVector - hipsVector);

  // 足の向きを合わせた後に腰の向きを合わせる
  const auto legsRotation = Eigen::Quaterniond::FromTwoVectors(
      legsVectorL2R, Eigen::Vector3d::UnitX());
  const auto hipsNormalV2 = legsRotation * hipsNormalV;
  const auto hipsRotation = Eigen::Quaterniond::FromTwoVectors(
      hipsNormalV2, Eigen::Vector3d::UnitY());

  // Affine3dにする
  const auto legsRvec = quaternionToRvec(legsRotation);
  const auto hipsRvec = quaternionToRvec(hipsRotation);
  auto result = cv::Affine3d(hipsRvec) * cv::Affine3d(legsRvec) *
                cv::Affine3d(cv::Vec3d(), -base);
  // cv::Vec3d tvec = result * -base;
  // result = result * cv::Affine3d(cv::Vec3d(), tvec);

  const auto h = result * hips;
  const auto l = result * lleg;
  const auto r = result * rleg;
  return result;
}

double getAreaFromTriangle(const cv::Point2f& a, const cv::Point2f& b,
                           const cv::Point2f& c) {
  auto v1 = b - a;
  auto v2 = c - a;
  return abs(v1.x * v2.y - v2.x * v1.y) / 2.0;
}

double getAreaFromCorners(const std::vector<cv::Point2f>& corners) {
  auto s1 = getAreaFromTriangle(corners[0], corners[1], corners[2]);
  auto s2 = getAreaFromTriangle(corners[2], corners[3], corners[0]);
  return s1 + s2;
}

double getAreaFromTriangle(const cv::Vec3d& a, const cv::Vec3d& b,
                           const cv::Vec3d& c) {
  auto v1 = b - a;
  auto v2 = c - a;
  return cv::norm(v1.cross(v2)) / 2.0;
}

double getAreaFromCorners(const std::vector<cv::Vec3d>& corners) {
  auto s1 = getAreaFromTriangle(corners[0], corners[1], corners[2]);
  auto s2 = getAreaFromTriangle(corners[2], corners[3], corners[0]);
  return s1 + s2;
}
