#pragma once
#include <Eigen/Geometry>
#include <opencv2/core/affine.hpp>

#include "geometry.h"

template <class V>
class LowPassFilter {
 public:
  V lastValue;
  double gain;

  V process(const V& newValue) {
    lastValue = lastValue * gain + newValue * (1.0 - gain);
    return lastValue;
  }
};

template <typename _Scalar>
class LowPassFilter<Eigen::Quaternion<_Scalar>> {
 public:
  Eigen::Quaternion<_Scalar> lastValue;
  double gain = 0.0;

  Eigen::Quaternion<_Scalar> process(
      const Eigen::Quaternion<_Scalar>& newValue) {
    lastValue = lastValue.slerp(1.0 - gain, newValue);
    return Eigen::Quaternion<_Scalar>(lastValue);
  }

  Eigen::Quaternion<_Scalar> process(const cv::Vec<_Scalar, 3>& newValue) {
    auto newQuat = rvecToQuaternion(newValue);
    return process(newQuat);
  }
};
