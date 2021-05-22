#pragma once

#include <opencv2/core.hpp>

class CameraCaptureBase {
 public:
  virtual ~CameraCaptureBase(){};
  virtual int Open(int width, int height, double fps) = 0;
  virtual void setExposure(double exposure) = 0;
  virtual double getExposure() = 0;
  virtual void getImage(cv::Mat& dst) = 0;
  virtual int getWidth() = 0;
  virtual int getHeight() = 0;
};
