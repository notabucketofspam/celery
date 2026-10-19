#include "PiCameraStream.hpp"
#include <string>
#include <opencv2/imgproc.hpp>

PiCameraStream::PiCameraStream(int w, int h, int fps) : width(w), height(h) {
  yuv_frame_size = (width * height * 3) / 2;
  yuv_buffer.create(height + height / 2, width, CV_8UC1);

  std::string cmd = "rpicam-vid -t 0"
    " --mode 1280:720:10:P"
    " --width " + std::to_string(width) +
    " --height " + std::to_string(height) +
    " --framerate " + std::to_string(fps) +
    " --codec yuv420 --denoise cdn_off --nopreview -o - 2>/dev/null";

  pipe = popen(cmd.c_str(), "r");
}

PiCameraStream::~PiCameraStream() {
  if (pipe) {
    pclose(pipe);
  }
}

bool PiCameraStream::isOpened() const {
  return (pipe != nullptr);
}

bool PiCameraStream::read(cv::Mat& bgr_out) {
  if (!pipe) return false;

  size_t bytes_read = fread(yuv_buffer.data, 1, yuv_frame_size, pipe);
  if (bytes_read != yuv_frame_size) {
    return false;
  }

  cv::cvtColor(yuv_buffer, bgr_out, cv::COLOR_YUV2BGR_I420);
  return true;
}
