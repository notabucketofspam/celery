#pragma once

#include <cstdio>
#include <opencv2/core.hpp>

class PiCameraStream {
private:
  FILE* pipe = nullptr;
  int width;
  int height;
  int raw_width;
  int raw_height;
  size_t yuv_frame_size;
  cv::Mat yuv_buffer;
  cv::Mat full_bgr;

public:
  PiCameraStream(int w, int h, int fps);
  ~PiCameraStream();

  PiCameraStream(const PiCameraStream&) = delete;
  PiCameraStream& operator=(const PiCameraStream&) = delete;

  bool isOpened() const;
  bool read(cv::Mat& bgr_out);
};
