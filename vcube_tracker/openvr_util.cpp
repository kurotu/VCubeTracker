#include "openvr_util.h"

std::unique_ptr<vr::TrackedDevicePose_t> getHmdPose() {
  vr::TrackedDevicePose_t devicePoses[vr::k_unMaxTrackedDeviceCount];

  float fSecondsSinceLastVsync;
  vr::VRSystem()->GetTimeSinceLastVsync(&fSecondsSinceLastVsync, NULL);
  float fDisplayFrequency = vr::VRSystem()->GetFloatTrackedDeviceProperty(
      vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);
  float fFrameDuration = 1.f / fDisplayFrequency;
  float fVsyncToPhotons = vr::VRSystem()->GetFloatTrackedDeviceProperty(
      vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SecondsFromVsyncToPhotons_Float);
  float fPredictedSecondsFromNow =
      fFrameDuration - fSecondsSinceLastVsync + fVsyncToPhotons;
  vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(
      vr::TrackingUniverseRawAndUncalibrated, fPredictedSecondsFromNow,
      devicePoses, vr::k_unMaxTrackedDeviceCount);
  for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount;
       deviceIndex++) {
    if (!vr::VRSystem()->IsTrackedDeviceConnected(deviceIndex)) {
      continue;
    }
    if (vr::VRSystem()->GetTrackedDeviceClass(deviceIndex) ==
        vr::TrackedDeviceClass_HMD) {
      auto pose = devicePoses + deviceIndex;
      return std::unique_ptr<vr::TrackedDevicePose_t>(
          new vr::TrackedDevicePose_t(*pose));
    }
  }
  return nullptr;
}

Eigen::Matrix3d getRotationMatrix(const vr::HmdMatrix34_t& mat) {
  Eigen::Matrix3d rot;
  for (auto r = 0; r < 3; r++) {
    for (auto c = 0; c < 3; c++) {
      rot(r, c) = mat.m[r][c];
    }
  }
  return rot;
}

Eigen::Vector3d getTranslation(const vr::HmdMatrix34_t& mat) {
  return Eigen::Vector3d(mat.m[0][3], mat.m[1][3], mat.m[2][3]);
}
