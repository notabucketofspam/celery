// include/FramebufferDisplay.hpp
#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <cstddef>

class FramebufferDisplay {
private:
  int fbfd = -1;
  char* fbp = nullptr;
  size_t screensize = 0;
  int width = 0;
  int height = 0;
  int bpp = 0;
  cv::Mat resized_buf;
  cv::Mat bgr565_buf;

public:
  FramebufferDisplay(const std::string& dev = "/dev/fb0");
  ~FramebufferDisplay();

  FramebufferDisplay(const FramebufferDisplay&) = delete;
  FramebufferDisplay& operator=(const FramebufferDisplay&) = delete;

  bool isOpened() const;
  void show(const cv::Mat& frame);
};
