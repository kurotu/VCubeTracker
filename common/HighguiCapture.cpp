#include "HighguiCapture.h"

HighguiCapture::HighguiCapture(int cameraNum)
    : cameraNum(cameraNum), width(0), height(0) {}

int HighguiCapture::Open(int width, int height, double fps) {
  std::scoped_lock lock(io);
  camera.open(cameraNum);
  auto result = camera.isOpened();
  camera.set(cv::CAP_PROP_FRAME_WIDTH, 640);
  camera.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
  camera.set(cv::CAP_PROP_FPS, fps);
  this->width = width;
  this->height = height;
  return 0;
}

void HighguiCapture::setExposure(double exposure) {
  std::scoped_lock lock(io);
  // camera.set(cv::CAP_PROP_EXPOSURE,)
}

double HighguiCapture::getExposure() {
  std::scoped_lock lock(io);
  return 0.0;
}

void HighguiCapture::getImage(cv::Mat& dst) {
  std::scoped_lock lock(io);
  camera.read(dst);
}

int HighguiCapture::getWidth() { return width; }

int HighguiCapture::getHeight() { return height; }
