// include/PiCameraStream.hpp
#pragma once

#include <cstdio>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <opencv2/core.hpp>

class PiCameraStream {
private:
  FILE* pipe = nullptr;
  int width;
  int height;
  int raw_width;
  int raw_height;
  size_t yuv_frame_size;

  std::thread reader_thread;
  std::mutex mtx;
  std::condition_variable cv;
  std::atomic<bool> running{ false };
  bool has_new_frame = false;

  cv::Mat latest_bgr;

  void captureLoop();

public:
  PiCameraStream(int w, int h, int fps);
  ~PiCameraStream();

  PiCameraStream(const PiCameraStream&) = delete;
  PiCameraStream& operator=(const PiCameraStream&) = delete;

  bool isOpened() const;
  bool read(cv::Mat& bgr_out);
};
