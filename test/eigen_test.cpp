#define _USE_MATH_DEFINES
#include <gtest/gtest.h>

#include <Eigen/Eigen>
#include <cmath>

TEST(EigenTest, QuaternionFromTwoVectors) {
  Eigen::Vector3d a(1, 0, 0), b(0, 1, 0);
  auto q = Eigen::Quaterniond::FromTwoVectors(a, b);
  Eigen::AngleAxisd ax(q);
  ASSERT_NEAR(ax.angle(), M_PI_2, 0.0001);
  ASSERT_NEAR(ax.axis()[0], 0.0, 0.0001);
  ASSERT_NEAR(ax.axis()[1], 0.0, 0.0001);
  ASSERT_NEAR(ax.axis()[2], 1.0, 0.0001);
  // b = q * a (‰ñ“]‚ÉŒÀ‚é)
  auto v = q * a;
  ASSERT_NEAR(v[0], b[0], 0.0001);
  ASSERT_NEAR(v[1], b[1], 0.0001);
  ASSERT_NEAR(v[2], b[2], 0.0001);
}
