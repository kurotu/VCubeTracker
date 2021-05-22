#include "fake_tracker.h"

#include <openvr.h>

#include "osc/OscOutboundPacketStream.h"

uint32_t hipID;
uint32_t leftFootID;
uint32_t rightFootID;
/*
static std::vector<uint32_t> virtualDeviceIndexes;

void updateVirtualDevices(vrinputemulator::VRInputEmulator& inputEmulator) {
  int count = inputEmulator.getVirtualDeviceCount();
  if (virtualDeviceIndexes.size() != count) {
    virtualDeviceIndexes.clear();
    for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount;
         deviceIndex++) {
      try {
        virtualDeviceIndexes.push_back(
            inputEmulator.getVirtualDeviceInfo(deviceIndex).openvrDeviceId);
      } catch (vrinputemulator::vrinputemulator_exception e) {
        // skip
      }
    }
  }
}

bool isVirtualDevice(uint32_t deviceIndex) {
  if (virtualDeviceIndexes.empty()) {
    return false;
  }
  return std::find(virtualDeviceIndexes.begin(), virtualDeviceIndexes.end(),
                   deviceIndex) != virtualDeviceIndexes.end();
}
*/
bool findTrackers() {
  hipID = 0;
  leftFootID = 1;
  rightFootID = 2;
  return true;
}

void onClose() {}

void sendOSC(UdpTransmitSocket& socket, int id, float vx, float vy, float vz,
             float qx, float qy, float qz, float qw) {
  constexpr auto size = 1024;
  char buf[size];
  osc::OutboundPacketStream packet(buf, size);
  packet << osc::BeginMessage("/VMT/Room/Driver") << id << 1 << 0.0f << vx << vy
         << vz << qx << qy << qz << qw << osc::EndMessage;
  socket.Send(packet.Data(), packet.Size());
}

void setVirtualDevicePosition(UdpTransmitSocket& socket, uint32_t id,
                              const cv::Vec3d& pos,
                              const Eigen::Quaterniond& rot) {
  sendOSC(socket, id, pos[0], pos[1], pos[2], rot.x(), rot.y(), rot.z(),
          rot.w());
}
