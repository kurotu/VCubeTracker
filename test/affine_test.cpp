#include <gtest/gtest.h>

#include "common.h"

TEST(AffineTest, BasicCase) {
  cv::Vec3d v(1.0, 1.0, 1.0);
  auto affine = getAffine3d(CV_PI, 0.0, 0.0);
  auto dest = affine * v;
  ASSERT_DOUBLE_EQ(dest[0], 1.0);
  ASSERT_DOUBLE_EQ(dest[1], -1.0);
  ASSERT_DOUBLE_EQ(dest[2], -1.0);
}

TEST(AffineTest, Multiply) {
  // (1,0,0)(0,0,90d)でスタートし、(2,0,0)移動する
  cv::Affine3<double> base;
  auto affine = getAffine3d(0.0, 0.0, CV_PI / 2.0);
  affine.translation({1.0, 0.0, 0.0});
  auto affine2 = getAffine3d(0.0, 0.0, 0.0);
  affine2.translation({2.0, 0.0, 0.0});
  auto result = base * affine * affine2;
  ASSERT_DOUBLE_EQ(result.translation()[0], 1.0);
  ASSERT_DOUBLE_EQ(result.translation()[1], 2.0);
  ASSERT_DOUBLE_EQ(result.translation()[2], 0.0);
}

class Trackers {
 public:
  cv::Vec3d hips, lleg, rleg;
};

class GeometryTest : public ::testing::TestWithParam<Trackers> {
  // ここは，通常のフィクスチャクラスのメンバと同じように書くことができます．
  // テストパラメータにアクセスするには，クラスから GetParam()
  // を呼び出します．
  // TestWithParam<T>.
};

// hips  0.0, 0.5, 0.0
// lleg -0.1, 0.0, 0.0
// rleg  0.1, 0.0, 0.0
TEST_P(GeometryTest, Calib) {
  const auto trackers = GetParam();
  // Todo hmdのy軸回転の反映
  const auto affineToOrigin =
      getAffine3dToOrigin(trackers.hips, trackers.lleg, trackers.rleg);
  const auto t = affineToOrigin.translation();
  const auto r = affineToOrigin.rvec();
  const auto hips = affineToOrigin * trackers.hips;
  const auto lleg = affineToOrigin * trackers.lleg;
  const auto rleg = affineToOrigin * trackers.rleg;
  const auto eps = 0.0001;
  ASSERT_NEAR(hips[0], 0.0, eps);
  ASSERT_NEAR(hips[1], 0.5, eps);
  ASSERT_NEAR(hips[2], 0.0, eps);
  ASSERT_NEAR(lleg[0], -0.1, eps);
  ASSERT_NEAR(lleg[1], 0.0, eps);
  ASSERT_NEAR(lleg[2], 0.0, eps);
  ASSERT_NEAR(rleg[0], 0.1, eps);
  ASSERT_NEAR(rleg[1], 0.0, eps);
  ASSERT_NEAR(rleg[2], 0.0, eps);
}

INSTANTIATE_TEST_SUITE_P(
    InstantiationName, GeometryTest,
    ::testing::Values(
        // 特に回転しないケース
        Trackers{cv::Vec3d(0, 0.5, 0), cv::Vec3d(-0.1, 0, 0),
                 cv::Vec3d(0.1, 0, 0)},
        // x軸で90度回す
        Trackers{cv::Vec3d(0, 0.0, 0.5), cv::Vec3d(-0.1, 0, 0),
                 cv::Vec3d(0.1, 0, 0)},
        // y軸で90度回す
        Trackers{cv::Vec3d(0, 0.5, 0.0), cv::Vec3d(0, -0, -0.1),
                 cv::Vec3d(0, 0, 0.1)},
        // 平行移動 1,2,3
        Trackers{cv::Vec3d(1, 2.5, 3.0), cv::Vec3d(1 - 0.1, 2, 3),
                 cv::Vec3d(1 + 0.1, 2, 3)},
        // y軸で回してから平行移動 1,2,3
        Trackers{cv::Vec3d(1, 2.5, 3.0), cv::Vec3d(1, 2, 3 - 0.1),
                 cv::Vec3d(1, 2, 3 + 0.1)},
        // zで180度回す
        Trackers{cv::Vec3d(0, -0.5, 0), cv::Vec3d(0.1, 0, 0),
                 cv::Vec3d(-0.1, 0, 0)}));

class GeometryAsymmetryTest : public ::testing::TestWithParam<Trackers> {
  // ここは，通常のフィクスチャクラスのメンバと同じように書くことができます．
  // テストパラメータにアクセスするには，クラスから GetParam()
  // を呼び出します．
  // TestWithParam<T>.
};

// hipsがllegの上にある
// hips -0.1, 0.5, 0.0
// lleg -0.1, 0.0, 0.0
// rleg  0.1, 0.0, 0.0
TEST_P(GeometryAsymmetryTest, Calib) {
  const auto trackers = GetParam();
  // Todo hmdのy軸回転の反映
  const auto affineToOrigin =
      getAffine3dToOrigin(trackers.hips, trackers.lleg, trackers.rleg);
  const auto t = affineToOrigin.translation();
  const auto r = affineToOrigin.rvec();
  const auto hips = affineToOrigin * trackers.hips;
  const auto lleg = affineToOrigin * trackers.lleg;
  const auto rleg = affineToOrigin * trackers.rleg;
  const auto eps = 0.0001;
  ASSERT_NEAR(hips[0], -0.1, eps);
  ASSERT_NEAR(hips[1], 0.5, eps);
  ASSERT_NEAR(hips[2], 0.0, eps);
  ASSERT_NEAR(lleg[0], -0.1, eps);
  ASSERT_NEAR(lleg[1], 0.0, eps);
  ASSERT_NEAR(lleg[2], 0.0, eps);
  ASSERT_NEAR(rleg[0], 0.1, eps);
  ASSERT_NEAR(rleg[1], 0.0, eps);
  ASSERT_NEAR(rleg[2], 0.0, eps);
}

INSTANTIATE_TEST_SUITE_P(
    InstantiationName, GeometryAsymmetryTest,
    ::testing::Values(
        // 特に回転しないケース
        Trackers{cv::Vec3d(-0.1, 0.5, 0), cv::Vec3d(-0.1, 0, 0),
                 cv::Vec3d(0.1, 0, 0)},
        // x軸で90度回す
        Trackers{cv::Vec3d(-0.1, 0.0, 0.5), cv::Vec3d(-0.1, 0, 0),
                 cv::Vec3d(0.1, 0, 0)},
        // x軸で-90度回す
        Trackers{cv::Vec3d(-0.1, 0.0, -0.5), cv::Vec3d(-0.1, 0, 0),
                 cv::Vec3d(0.1, 0, 0)},
        // y軸で90度回す
        Trackers{cv::Vec3d(0, 0.5, -0.1), cv::Vec3d(0, -0, -0.1),
                 cv::Vec3d(0, 0, 0.1)},
        // 平行移動 1,2,3
        Trackers{cv::Vec3d(1 - 0.1, 2.5, 3.0), cv::Vec3d(1 - 0.1, 2, 3),
                 cv::Vec3d(1 + 0.1, 2, 3)},
        // y軸で回してから平行移動 1,2,3
        Trackers{cv::Vec3d(1, 2.5, 3.0 - 0.1), cv::Vec3d(1, 2, 3 - 0.1),
                 cv::Vec3d(1, 2, 3 + 0.1)},
        // zで180度回す
        Trackers{cv::Vec3d(0.1, -0.5, 0.0), cv::Vec3d(0.1, 0, 0),
                 cv::Vec3d(-0.1, 0, 0)},
        // zで180度回して、xで回す
        Trackers{cv::Vec3d(0.1, 0.0, -0.5), cv::Vec3d(0.1, 0, 0),
                 cv::Vec3d(-0.1, 0, 0)}));

TEST(GeometryTest, CalibTrackers) {
  const cv::Vec3d hips{0.0028028849622411256, -0.33787069873395287,
                       1.3723879574925824};
  const cv::Vec3d lleg{0.064611665502477059, 0.41268927727910182,
                       1.5723177584112802};
  const cv::Vec3d rleg{-0.057469195909992075, 0.40951516022598700,
                       1.5603777206882528};
  const auto affineToOrigin = getAffine3dToOrigin(hips, lleg, rleg);
  const auto h = affineToOrigin * hips;
  const auto l = affineToOrigin * lleg;
  const auto r = affineToOrigin * rleg;

  const auto eps = 0.0001;
  const auto legsDistance = cv::norm(rleg - lleg);
  ASSERT_NEAR(l[0], -legsDistance / 2.0, eps);
  ASSERT_NEAR(l[1], 0.0, eps);
  ASSERT_NEAR(l[2], 0.0, eps);
  ASSERT_NEAR(r[0], legsDistance / 2.0, eps);
  ASSERT_NEAR(r[1], 0.0, eps);
  ASSERT_NEAR(r[2], 0.0, eps);
  ASSERT_NEAR(h[2], 0.0, eps);
}
