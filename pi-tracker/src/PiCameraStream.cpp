#include "PiCameraStream.hpp"
#include <string>
#include <opencv2/imgproc.hpp>

PiCameraStream::PiCameraStream(int w, int h, int fps) : width(w), height(h) {
  // The Pi 4 ISP hardware enforces a 64-byte stride. 
  // We force the internal width to 512 to prevent silent row padding.
  int raw_width = 512;
  int raw_height = 320;

  yuv_frame_size = (raw_width * raw_height * 3) / 2;
  yuv_buffer.create(raw_height + raw_height / 2, raw_width, CV_8UC1);

  std::string cmd = "rpicam-vid -t 0"
    " --mode 1280:720:10:P"
    " --width " + std::to_string(raw_width) +
    " --height " + std::to_string(raw_height) +
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

  // Read the full 512x320 padded frame from the hardware
  size_t bytes_read = fread(yuv_buffer.data, 1, yuv_frame_size, pipe);
  if (bytes_read != yuv_frame_size) {
    return false;
  }

  cv::Mat full_bgr;
  cv::cvtColor(yuv_buffer, full_bgr, cv::COLOR_YUV2BGR_I420);

  // Crop the 32 pixels of hardware padding off the right edge
  // to return the exact 480x320 frame the rest of the app expects
  bgr_out = full_bgr(cv::Rect(0, 0, width, height)).clone();

  return true;
}
