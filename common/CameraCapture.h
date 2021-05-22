#pragma once

#include <mutex>
#include <opencv2/core.hpp>

#include "CameraCaptureBase.h"

class CameraCapture : public CameraCaptureBase {
 private:
  int cameraNum;
  int width;
  int height;
  std::mutex io;

 public:
  CameraCapture(int index);
  virtual int Open(int width, int height, double fps);
  virtual void setExposure(double exposure);
  virtual double getExposure();
  virtual void getImage(cv::Mat& dst);
  virtual int getWidth() { return width; }
  virtual int getHeight() { return height; }
};
