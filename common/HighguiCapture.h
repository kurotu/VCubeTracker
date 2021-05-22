#pragma once

#include <mutex>
#include <opencv2/highgui.hpp>

#include "CameraCaptureBase.h"

class HighguiCapture : public CameraCaptureBase {
 private:
  int cameraNum;
  int width;
  int height;
  cv::VideoCapture camera;
  std::mutex io;

 public:
  HighguiCapture(int cameraNum);
  virtual int Open(int width, int height, double fps) override;
  virtual void setExposure(double exposure) override;
  virtual double getExposure() override;
  virtual void getImage(cv::Mat& dst) override;
  virtual int getWidth() override;
  virtual int getHeight() override;
};