#include <gtest/gtest.h>

#include "common.h"

TEST(LowPassFilterTest, Vector) {
  cv::Vec3d a(1, 2, 3), b(4, 5, 6);
  LowPassFilter<cv::Vec3d> lpf;
  auto gain = 0.8;
  lpf.gain = gain;
  lpf.lastValue = a;
  auto result = lpf.process(b);
  ASSERT_DOUBLE_EQ(result[0], a[0] * gain + (1.0 - gain) * b[0]);
  ASSERT_DOUBLE_EQ(result[1], a[1] * gain + (1.0 - gain) * b[1]);
  ASSERT_DOUBLE_EQ(result[2], a[2] * gain + (1.0 - gain) * b[2]);
}

TEST(LowPassFilterTest, Quaternion) {
  Eigen::AngleAxisd a(0, Eigen::Vector3d::UnitX());
  Eigen::AngleAxisd b(CV_PI, Eigen::Vector3d::UnitX());
  auto gain = 0.8;
  LowPassFilter<Eigen::Quaterniond> lpf;
  lpf.gain = gain;
  lpf.lastValue = a;
  auto q = lpf.process(Eigen::Quaterniond(b));
  auto result = Eigen::AngleAxisd(q);
  ASSERT_DOUBLE_EQ(result.angle(), CV_PI * (1.0 - gain));
  ASSERT_DOUBLE_EQ(result.axis().x(), Eigen::Vector3d::UnitX().x());
  ASSERT_DOUBLE_EQ(result.axis().y(), Eigen::Vector3d::UnitX().y());
  ASSERT_DOUBLE_EQ(result.axis().z(), Eigen::Vector3d::UnitX().z());
}
