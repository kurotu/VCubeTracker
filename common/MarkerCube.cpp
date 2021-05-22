#include "MarkerCube.h"

#include "tracker.h"

cv::Affine3d estimateConvert(const std::vector<cv::Vec3d>& targetRvec,
                             const std::vector<cv::Vec3d>& targetTvec,
                             const std::vector<cv::Vec3d>& originalRvec,
                             const std::vector<cv::Vec3d>& originalTvec) {
  cv::Affine3d target(targetRvec[0], targetTvec[0]);
  cv::Affine3d original(originalRvec[0], originalTvec[0]);
  // target = A * original
  // A = target * original_inv
  auto result = target * original.inv();
  return result;
}

cv::Affine3d estimateConvert(const TrackerData& target,
                             const TrackerData& original) {
  return target.affine3d() * original.affine3d().inv();
}

void MarkerCube::convertToKeyPose(int id, const cv::Vec3d& rvec,
                                  const cv::Vec3d& tvec, cv::Vec3d& out_rvec,
                                  cv::Vec3d& out_tvec) {
  if (id == keyId) {
    out_tvec = tvec;
    out_rvec = rvec;
    return;
  }
  auto index = id - keyId - 1;
  cv::Affine3d pose(rvec, tvec);
  auto result = pose * affines[index];
  out_tvec = result.translation();
  out_rvec = result.rvec();
}

void MarkerCube::convertToKeyPose(int id, const TrackerData& pose,
                                  TrackerData& out_pose) {
  convertToKeyPose(id, pose.rvec, pose.tvec, out_pose.rvec, out_pose.tvec);
}

void MarkerCube::saveParams(cv::FileStorage& storage) {
  storage << "keyId" << keyId;
  storage << "affines"
          << "{";
  storage << "sub0" << cv::Mat(affines[0].matrix, false);
  storage << "sub1" << cv::Mat(affines[1].matrix, false);
  storage << "sub2" << cv::Mat(affines[2].matrix, false);
  storage << "sub3" << cv::Mat(affines[3].matrix, false);
  storage << "}";
}

void MarkerCube::loadParams(cv::FileNode& storage) {
  keyId = static_cast<int>(storage["keyId"]);
  auto fileNode = storage["affines"];
  cv::Matx44d affine;
  affine = cv::Matx44d(reinterpret_cast<double*>(fileNode["sub0"].mat().ptr()));
  affines[0] = cv::Affine3d(affine);
  affine = cv::Matx44d(reinterpret_cast<double*>(fileNode["sub1"].mat().ptr()));
  affines[1] = cv::Affine3d(affine);
  affine = cv::Matx44d(reinterpret_cast<double*>(fileNode["sub2"].mat().ptr()));
  affines[2] = cv::Affine3d(affine);
  affine = cv::Matx44d(reinterpret_cast<double*>(fileNode["sub3"].mat().ptr()));
  affines[3] = cv::Affine3d(affine);
}
