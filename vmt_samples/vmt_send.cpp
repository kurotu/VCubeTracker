#include <ip/UdpSocket.h>
#include <osc/OscOutboundPacketStream.h>

#include <chrono>
#include <iostream>
#include <thread>

void sendOSC(UdpTransmitSocket& socket, int id, float vx, float vy, float vz,
             float qx, float qy, float qz, float qw) {
  constexpr auto size = 1024;
  char buf[size];
  osc::OutboundPacketStream packet(buf, size);
  packet << osc::BeginMessage("/VMT/Room/Driver") << id << 1 << 0.0f << vx << vy
         << vz << qx << qy << qz << qw << osc::EndMessage;
  socket.Send(packet.Data(), packet.Size());
}

int main() {
  UdpTransmitSocket socket(IpEndpointName("127.0.0.1", 39570));

  const auto start = std::chrono::steady_clock::now();
  while (true) {
    const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    const auto nowf = (float)now.count() / 1000.0f;

    sendOSC(socket, 0, sin(nowf), cos(nowf), 1.0, 0.0, 0.0, 0.0, 1.0);
    sendOSC(socket, 1, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    sendOSC(socket, 2, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
