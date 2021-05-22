#pragma once;

#include <openvr.h>

#include <Eigen/Eigen>
#include <memory>

std::unique_ptr<vr::TrackedDevicePose_t> getHmdPose();
Eigen::Matrix3d getRotationMatrix(const vr::HmdMatrix34_t& mat);
Eigen::Vector3d getTranslation(const vr::HmdMatrix34_t& mat);
