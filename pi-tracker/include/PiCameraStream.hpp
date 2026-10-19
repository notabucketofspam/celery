#pragma once

#include <cstdio>
#include <opencv2/core.hpp>

class PiCameraStream {
private:
  FILE* pipe = nullptr;
  int width;
  int height;
  size_t yuv_frame_size;
  cv::Mat yuv_buffer;

public:
  PiCameraStream(int w, int h, int fps);
  ~PiCameraStream();

  // Disable copying since we manage a raw FILE* pipe
  PiCameraStream(const PiCameraStream&) = delete;
  PiCameraStream& operator=(const PiCameraStream&) = delete;

  bool isOpened() const;
  bool read(cv::Mat& bgr_out);
};
