#include "MarkerCalibrator.h"

double getMedian(std::vector<double>& v) {
  auto med = v.begin() + v.size() / 2;
  std::nth_element(v.begin(), med, v.end());
  return *med;
}

cv::Vec3d getMedian(const std::vector<cv::Vec3d>& vecs) {
  const auto size = vecs.size();
  std::vector<double> vx(size), vy(size), vz(size);
  std::transform(vecs.begin(), vecs.end(), vx.begin(),
                 [](auto v) { return v[0]; });
  std::transform(vecs.begin(), vecs.end(), vy.begin(),
                 [](auto v) { return v[1]; });
  std::transform(vecs.begin(), vecs.end(), vz.begin(),
                 [](auto v) { return v[2]; });
  auto x = getMedian(vx);
  auto y = getMedian(vy);
  auto z = getMedian(vz);
  auto max = *std::max_element(vx.begin(), vx.end());
  auto min = *std::min_element(vx.begin(), vx.end());
  return {x, y, z};
}

cv::Affine3d MarkerCalibrator::calibrate(int index) {
  const auto size = buffers[index].size();
  std::vector<cv::Affine3d> estimates(size);
  std::transform(buffers[index].begin(), buffers[index].end(),
                 estimates.begin(), [](auto data) {
                   return estimateConvert(data.keyPose, data.originalPose);
                 });
  std::vector<cv::Vec3d> tvecs(size);
  std::vector<cv::Vec3d> rvecs(size);
  std::transform(estimates.begin(), estimates.end(), tvecs.begin(),
                 [](auto affine) { return affine.translation(); });
  std::transform(estimates.begin(), estimates.end(), rvecs.begin(),
                 [](auto affine) { return affine.rvec(); });
  // ’†‰›’l‚ğo‚·
  auto tvec_median = getMedian(tvecs);
  auto rvec_median = getMedian(rvecs);
  cv::Affine3d result(rvec_median, tvec_median);
  cv::Affine3d assertOrigin(buffers[index][0].originalPose.affine3d());
  cv::Vec3d assertTarget(buffers[index][0].keyPose.tvec);
  auto assertKeyPose = (result * assertOrigin).translation();
  assert(abs(assertKeyPose[0] - assertTarget[0]) < 0.00001);
  assert(abs(assertKeyPose[1] - assertTarget[1]) < 0.00001);
  assert(abs(assertKeyPose[2] - assertTarget[2]) < 0.00001);
  return result;
}

void MarkerCalibrator::pushData(const TrackerData& keyPose, int id,
                                const TrackerData& subPose) {
  auto index = id - keyId - 1;
  buffers[index].push_back({keyPose, subPose});
  if (buffers[index].size() > maxBuffer) {
    buffers[index].pop_front();
  }
}

std::array<cv::Affine3d, 4> MarkerCalibrator::calibrate_() {
  std::array<cv::Affine3d, 4> results;
  for (auto i = 0; i < results.size(); i++) {
    results[i] = calibrate(i);
  }
  return results;
}

MarkerCube MarkerCalibrator::calibrate() {
  MarkerCube cube(keyId);
  cube.affines = calibrate_();
  const auto assertTarget = buffers[0][0].keyPose;
  const auto assertOriginal = buffers[0][0].originalPose;
  TrackerData estimated;
  cube.convertToKeyPose(1, assertOriginal, estimated);
  assert(abs(assertTarget.tvec[0] - estimated.tvec[0]) < 0.00001);
  assert(abs(assertTarget.tvec[1] - estimated.tvec[1]) < 0.00001);
  assert(abs(assertTarget.tvec[2] - estimated.tvec[2]) < 0.00001);
  return cube;
}

bool MarkerCalibrator::isFullFilled() {
  for (auto& b : buffers) {
    if (b.size() != maxBuffer) {
      return false;
    }
  }
  return true;
}
