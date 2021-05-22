#include <gtest/gtest.h>

#include "common.h"

TEST(AngleAxis, Simple) {
  cv::Vec3d rvec(CV_PI, 0.0, 0.0);
  auto angleAxis = rvecToAngleAxis(rvec);
  ASSERT_DOUBLE_EQ(angleAxis.angle(), CV_PI);
  ASSERT_DOUBLE_EQ(angleAxis.axis()[0], 1.0);
  ASSERT_DOUBLE_EQ(angleAxis.axis()[1], 0.0);
  ASSERT_DOUBLE_EQ(angleAxis.axis()[2], 0.0);
}

TEST(Quarternion, Simple) {
  cv::Vec3d rvec(CV_PI, 0.0, 0.0);
  auto q = rvecToQuaternion(rvec);
  Eigen::AngleAxisd angleAxis(q);
  ASSERT_DOUBLE_EQ(angleAxis.angle(), CV_PI);
  ASSERT_DOUBLE_EQ(angleAxis.axis()[0], 1.0);
  ASSERT_DOUBLE_EQ(angleAxis.axis()[1], 0.0);
  ASSERT_DOUBLE_EQ(angleAxis.axis()[2], 0.0);
}

TEST(Quaternion, Q2Rvec) {
  cv::Vec3d rvec(CV_PI, 0.0, 0.0);
  auto q = rvecToQuaternion(rvec);
  auto v = quaternionToRvec(q);
  ASSERT_DOUBLE_EQ(v[0], rvec[0]);
  ASSERT_DOUBLE_EQ(v[1], rvec[1]);
  ASSERT_DOUBLE_EQ(v[2], rvec[2]);
}

TEST(TrackerData, Affine3d) {
  TrackerData t = {{1.0, 2.0, 3.0}, {0.1, 0.2, 0.3}};
  auto affine = t.affine3d();
  ASSERT_DOUBLE_EQ(t.tvec[0], affine.translation()[0]);
  ASSERT_DOUBLE_EQ(t.tvec[1], affine.translation()[1]);
  ASSERT_DOUBLE_EQ(t.tvec[2], affine.translation()[2]);
  ASSERT_NEAR(t.rvec[0], affine.rvec()[0], 0.000001);
  ASSERT_NEAR(t.rvec[1], affine.rvec()[1], 0.000001);
  ASSERT_NEAR(t.rvec[2], affine.rvec()[2], 0.000001);
}

TEST(Tracker, CameraToVR) {
  TrackerData tracker_in_camera{
      {-0.14062685118967683, 0.043236578506480938, 1.6387977114105616},
      {-2.2358121679970653, -2.1075561381715535, 0.36552644015675767}};
  // 多分OpenCVもSteamVRも右手系のはず→Z周りに180度回せばいいはず
  auto rot = getAffine3dFromCameraToVR();
  auto result = rot * tracker_in_camera.affine3d();
  // x,yが逆になる
  ASSERT_DOUBLE_EQ(tracker_in_camera.tvec[0], -result.translation()[0]);
  ASSERT_DOUBLE_EQ(tracker_in_camera.tvec[1], -result.translation()[1]);
  ASSERT_DOUBLE_EQ(tracker_in_camera.tvec[2], result.translation()[2]);
}

TEST(Tracker, DISABLED_CalibrationSimple) {
  TrackerData hmd = {{0.0, 1.6, 0.0},
                     {0.0, 0.0, 0.0}};  // x:自分の右,y:自分の上,z:手前向き方向
  TrackerData hips = {
      {0.0, -1.0, 0.0},
      {0.0, 0.0,
       0.0}};  // x:カメラから見て右, y:カメラから見て下,z:カメラから見て奥
  TrackerData lleg = {{1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  TrackerData rleg = {{-1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  auto mat = getMatrixFromCameraToVR(hmd, hips, lleg, rleg);
  auto vr_hips = mat * hips.affine3dMat();
}
