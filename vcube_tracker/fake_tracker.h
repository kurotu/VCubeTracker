#pragma once

//#include <vrinputemulator.h>

#include <ip/UdpSocket.h>

#include <Eigen/Geometry>
#include <cstdint>
#include <opencv2/core.hpp>

extern uint32_t hipID;
extern uint32_t leftFootID;
extern uint32_t rightFootID;

bool findTrackers();
/*
uint32_t createTracker(vrinputemulator::VRInputEmulator& inputEmulator);
void setVirtualDevicePosition(vrinputemulator::VRInputEmulator& inputEmulator,
                              uint32_t id, const cv::Vec3d& pos,
                              const Eigen::Quaterniond& rot);
void onClose(vrinputemulator::VRInputEmulator& inputEmulator);
*/

void setVirtualDevicePosition(UdpTransmitSocket& socket, uint32_t id,
                              const cv::Vec3d& pos,
                              const Eigen::Quaterniond& rot);
