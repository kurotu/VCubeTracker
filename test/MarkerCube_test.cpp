#include <gtest/gtest.h>

#include "common.h"

TEST(MarkerCubeTest, EstimateConvertMatrix) {
  std::vector<cv::Vec3d> targetTvecs, targetRvecs, tvecs, rvecs;
  targetTvecs.push_back({1, 2, 3});
  targetRvecs.push_back({4, 5, 6});
  tvecs.push_back(targetTvecs.back() + cv::Vec3d{1.0, 1.0, 1.0});
  rvecs.push_back({6, 5, 4});

  targetTvecs.push_back({2, 3, 4});
  targetRvecs.push_back({4, 5, 6});
  tvecs.push_back(targetTvecs.back() + cv::Vec3d{1.0, 1.0, 1.0});
  rvecs.push_back({6, 5, 4});

  targetTvecs.push_back({3, 4, 5});
  targetRvecs.push_back({4, 5, 6});
  tvecs.push_back(targetTvecs.back() + cv::Vec3d{1.0, 1.0, 1.0});
  rvecs.push_back({6, 5, 4});

  targetTvecs.push_back({4, 5, 6});
  targetRvecs.push_back({4, 5, 6});
  tvecs.push_back(targetTvecs.back() + cv::Vec3d{1.0, 1.0, 1.0});
  rvecs.push_back({6, 5, 4});

  targetTvecs.push_back({5, 6, 7});
  targetRvecs.push_back({4, 5, 6});
  tvecs.push_back(targetTvecs.back() + cv::Vec3d{1.0, 1.0, 1.0});
  rvecs.push_back({6, 5, 4});

  targetTvecs.push_back({6, 7, 8});
  targetRvecs.push_back({4, 5, 6});
  tvecs.push_back(targetTvecs.back() + cv::Vec3d{1.0, 1.0, 1.0});
  rvecs.push_back({6, 5, 4});

  auto mat = estimateConvert(targetRvecs, targetTvecs, rvecs, tvecs);
  cv::Affine3d target(targetRvecs[0], targetTvecs[0]);
  cv::Affine3d original(rvecs[0], tvecs[0]);
  auto result = mat * original;
  ASSERT_NEAR(result.translation()[0], target.translation()[0], 0.000001);
  ASSERT_NEAR(result.translation()[1], target.translation()[1], 0.000001);
  ASSERT_NEAR(result.translation()[2], target.translation()[2], 0.000001);
  ASSERT_NEAR(result.rvec()[0], target.rvec()[0], 0.000001);
  ASSERT_NEAR(result.rvec()[1], target.rvec()[1], 0.000001);
  ASSERT_NEAR(result.rvec()[2], target.rvec()[2], 0.000001);
}

TEST(MarkerCubeTest, SaveLoad) {
  MarkerCube cube(0);
  for (int i = 0; i < cube.affines.size(); i++) {
    cv::Vec3d tvec = {1.0 * (double)i, 1.0 * (double)i, 1.0 * (double)i};
    cv::Vec3d rvec = {0.1 * (double)i, 0.1 * (double)i, 0.1 * (double)i};
    cube.affines[i] = cv::Affine3d(rvec, tvec);
  }
  cv::FileStorage writeYaml("tmp.yaml", cv::FileStorage::WRITE);
  cube.saveParams(writeYaml);
  writeYaml.release();
  cv::FileStorage readYaml("tmp.yaml", cv::FileStorage::READ);
  ASSERT_TRUE(readYaml.isOpened());
  MarkerCube dest(0);
  auto node = readYaml.root();
  dest.loadParams(node);
  ASSERT_NEAR(dest.affines[0].translation()[0],
              cube.affines[0].translation()[0], 0.00001);
}
