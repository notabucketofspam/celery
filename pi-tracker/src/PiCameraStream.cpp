// src/PiCameraStream.cpp
#include "PiCameraStream.hpp"
#include <string>
#include <opencv2/imgproc.hpp>

PiCameraStream::PiCameraStream(int w, int h, int fps) : width(w), height(h) {
  raw_width = (width + 63) & ~63;
  raw_height = (height + 15) & ~15;
  yuv_frame_size = static_cast<size_t>(raw_width) * raw_height * 3 / 2;

  std::string cmd = "rpicam-vid -t 0"
    " --mode 1280:720:10:P"
    " --width " + std::to_string(raw_width) +
    " --height " + std::to_string(raw_height) +
    " --framerate " + std::to_string(fps) +
    " --exposure sport"
    " --flush"
    " --codec yuv420 --denoise cdn_off --nopreview -o - 2>/dev/null";

  pipe = popen(cmd.c_str(), "r");
  if (pipe) {
    running = true;
    reader_thread = std::thread(&PiCameraStream::captureLoop, this);
  }
}

PiCameraStream::~PiCameraStream() {
  running = false;
  cv.notify_all();
  if (reader_thread.joinable()) {
    reader_thread.join();
  }
  if (pipe) {
    pclose(pipe);
  }
}

void PiCameraStream::captureLoop() {
  cv::Mat yuv_buffer(raw_height + raw_height / 2, raw_width, CV_8UC1);
  cv::Mat full_bgr(raw_height, raw_width, CV_8UC3);
  cv::Rect crop_roi(0, 0, width, height);

  while (running && pipe) {
    size_t bytes_read = fread(yuv_buffer.data, 1, yuv_frame_size, pipe);
    if (bytes_read != yuv_frame_size) {
      running = false;
      cv.notify_all();
      break;
    }

    cv::cvtColor(yuv_buffer, full_bgr, cv::COLOR_YUV2BGR_I420);

    {
      std::lock_guard<std::mutex> lock(mtx);
      full_bgr(crop_roi).copyTo(latest_bgr);
      has_new_frame = true;
    }
    cv.notify_one();
  }
}

bool PiCameraStream::isOpened() const {
  return running.load();
}

bool PiCameraStream::read(cv::Mat& bgr_out) {
  std::unique_lock<std::mutex> lock(mtx);
  cv.wait(lock, [this] { return has_new_frame || !running; });

  if (!running && !has_new_frame) return false;

  latest_bgr.copyTo(bgr_out);
  has_new_frame = false;
  return true;
}
