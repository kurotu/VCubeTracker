#include <gtest/gtest.h>

#include "common.h"

int Factorial(int n) {
  if (n == 0) return 1;
  return n * Factorial(n - 1);
}

// Tests factorial of 0.
TEST(FactorialTest, HandlesZeroInput) { EXPECT_EQ(Factorial(0), 1); }

// Tests factorial of positive numbers.
TEST(FactorialTest, HandlesPositiveInput) {
  EXPECT_EQ(Factorial(1), 1);
  EXPECT_EQ(Factorial(2), 2);
  EXPECT_EQ(Factorial(3), 6);
  EXPECT_EQ(Factorial(8), 40320);
}

TEST(GeometryTest, Area) {
  std::vector<cv::Vec3d> corners = {cv::Vec3d(0, 0, 0), cv::Vec3d(1, 0, 0),
                                    cv::Vec3d(1, 1, 0), cv::Vec3d(0, 1, 0)};
  auto s = getAreaFromCorners(corners);
  ASSERT_DOUBLE_EQ(s, 1.0);
}
