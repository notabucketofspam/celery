#include "PiCameraStream.hpp"
#include <string>
#include <opencv2/imgproc.hpp>

PiCameraStream::PiCameraStream(int w, int h, int fps) : width(w), height(h) {
  // Round width up to the nearest multiple of 64 for the Pi 4 ISP hardware stride
  raw_width = (width + 63) & ~63;
  raw_height = (height + 15) & ~15;

  yuv_frame_size = static_cast<size_t>(raw_width) * raw_height * 3 / 2;
  yuv_buffer.create(raw_height + raw_height / 2, raw_width, CV_8UC1);
  full_bgr.create(raw_height, raw_width, CV_8UC3);

  // --flush forces immediate stdout writes; --shutter 2000 prevents motion blur
  std::string cmd = "rpicam-vid -t 0"
    " --mode 1280:720:10:P"
    " --width " + std::to_string(raw_width) +
    " --height " + std::to_string(raw_height) +
    " --framerate " + std::to_string(fps) +
    " --shutter 2000"
    " --flush"
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

  cv::cvtColor(yuv_buffer, full_bgr, cv::COLOR_YUV2BGR_I420);

  // Copy only the unpadded 480x320 ROI into the reused bgr_out buffer
  full_bgr(cv::Rect(0, 0, width, height)).copyTo(bgr_out);
  return true;
}
