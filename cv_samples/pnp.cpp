#include <algorithm>
#include <iostream>
#include <opencv2/calib3d.hpp>

#define PRINT(var) (std::cout << #var << ": " << var << std::endl)

float getRandom() {
  constexpr auto rand_max = 1000;
  return (float)(std::rand() % (rand_max + 1)) / 1000.0f;
}

int main() {
  auto cameraYml = "./camera_unity.yaml";
  cv::FileStorage fs(cameraYml, cv::FileStorage::READ);
  if (!fs.isOpened()) {
    std::cout << "Failed to open " << cameraYml << std::endl;
    return 1;
  }
  auto cameraMatrix = fs["camera_matrix"].mat();
  // auto distCoeffs = fs["distortion_coefficients"].mat();
  cv::Mat distCoeffs = cv::Mat();
  PRINT(cameraMatrix);
  PRINT(distCoeffs);

  std::vector<cv::Point3f> objectPoints;
  objectPoints.push_back(cv::Point3f(0.0, 0.5, 0.0));   // h
  objectPoints.push_back(cv::Point3f(-0.1, 0.0, 0.0));  // l
  objectPoints.push_back(cv::Point3f(0.1, 0.0, 0.0));   // r
  auto N = 10000;
  for (auto i = 0; i < N; i++) {
    auto x = getRandom() / 10.0f - 0.05f;
    auto y = getRandom() / 10.0f - 0.05f;
    auto z = getRandom() / 10.0f - 0.05f;
    objectPoints.push_back(cv::Point3f(x, y, z));
  }
  cv::Affine3d cameraPose(cv::Vec3d(0.0, CV_PI / 2.0, CV_PI),
                          cv::Vec3d(1, 2, 3));
  PRINT(cameraPose.translation());
  PRINT(cameraPose.rvec());
  // cv::Affine3d cameraPose(cv::Vec3d(0.0,CV_PI/2.0,CV_PI),
  // cv::Vec3d(1.0,2.0,3.0));
  std::vector<cv::Point2f> imagePoints;
  cv::projectPoints(objectPoints, cameraPose.rvec(), cameraPose.translation(),
                    cameraMatrix, distCoeffs, imagePoints);
  for (int i = 0; i < std::min(4, (int)imagePoints.size()); i++) {
    PRINT(imagePoints[i]);
  }
  // std::vector<cv::Vec3d> rvecs, tvecs;
  // cv::solveP3P(objectPoints, imagePoints, cameraMatrix, distCoeffs, rvecs,
  // tvecs,cv::SOLVEPNP_AP3P); PRINT(tvecs[0]); PRINT(rvecs[0]);
  cv::Vec3d rvec, tvec;
  for (auto i = 0; i < objectPoints.size(); i++) {
    objectPoints[i].z = objectPoints[i].z + 0.1;
  }
  cv::solvePnPRansac(objectPoints, imagePoints, cameraMatrix, distCoeffs, rvec,
                     tvec);
  PRINT(tvec);
  PRINT(rvec);

  cv::Mat rmat;
  cv::Rodrigues(rvec, rmat);
  cv::Mat rmat_t = rmat.t();
  auto pos = rmat.t() * tvec;
  cv::Affine3d est(rmat_t);
  PRINT(est.rvec());
  return 0;
}