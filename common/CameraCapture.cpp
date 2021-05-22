#include "CameraCapture.h"
#define WIN32_LEAN_AND_MEAN
#include <ewclib.h>

CameraCapture::CameraCapture(int cameraNum)
    : cameraNum(cameraNum), width(0), height(0) {}

int CameraCapture::Open(int width, int height, double fps) {
  std::scoped_lock lock(io);
  this->width = width;
  this->height = height;
  return EWC_Open(cameraNum, width, height, fps, -1, MEDIASUBTYPE_RGB24,
                  MEDIASUBTYPE_MJPG);
}

void CameraCapture::setExposure(double exposure) {
  std::scoped_lock lock(io);
  EWC_SetManual(cameraNum, EWC_EXPOSURE);
  EWC_SetValue(cameraNum, EWC_EXPOSURE, exposure);
}

double CameraCapture::getExposure() {
  std::scoped_lock lock(io);
  return EWC_GetValue(cameraNum, EWC_EXPOSURE);
}

void CameraCapture::getImage(cv::Mat& dst) {
  std::scoped_lock lock(io);
  constexpr std::chrono::milliseconds wait(1);
  while (EWC_IsCaptured(cameraNum) != 1) {
    std::this_thread::sleep_for(wait);
  }
  EWC_GetImage(cameraNum, dst.data);
}
