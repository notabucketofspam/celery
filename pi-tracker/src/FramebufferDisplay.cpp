// src/FramebufferDisplay.cpp
#include "FramebufferDisplay.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <opencv2/imgproc.hpp>

FramebufferDisplay::FramebufferDisplay(const std::string& dev) {
  fbfd = open(dev.c_str(), O_RDWR);
  if (fbfd == -1) {
    std::cerr << "[Display] Warning: Cannot open " << dev << ". Screen output disabled.\n";
    return;
  }

  struct fb_var_screeninfo vinfo {};
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
    std::cerr << "[Display] Error reading variable screen info.\n";
    close(fbfd);
    fbfd = -1;
    return;
  }

  width = static_cast<int>(vinfo.xres);
  height = static_cast<int>(vinfo.yres);
  bpp = static_cast<int>(vinfo.bits_per_pixel);
  screensize = static_cast<size_t>(width) * static_cast<size_t>(height) * (static_cast<size_t>(bpp) / 8);

  void* map_ptr = mmap(nullptr, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
  if (map_ptr == MAP_FAILED) {
    std::cerr << "[Display] Error: failed to map framebuffer to memory.\n";
    fbp = nullptr;
    close(fbfd);
    fbfd = -1;
  } else {
    fbp = static_cast<char*>(map_ptr);
    std::cout << "[Display] Framebuffer mapped: " << width << "x" << height << ", " << bpp << "bpp\n";
  }
}

FramebufferDisplay::~FramebufferDisplay() {
  if (fbp) {
    munmap(fbp, screensize);
  }
  if (fbfd >= 0) {
    close(fbfd);
  }
}

bool FramebufferDisplay::isOpened() const {
  return (fbp != nullptr);
}

void FramebufferDisplay::show(const cv::Mat& frame) {
  if (!fbp || frame.empty() || bpp != 16) return;

  // Fast path: skip resize if camera frame already matches screen dimensions (480x320)
  if (frame.cols == width && frame.rows == height) {
    cv::cvtColor(frame, bgr565_buf, cv::COLOR_BGR2BGR565);
  } else {
    cv::resize(frame, resized_buf, cv::Size(width, height), 0, 0, cv::INTER_NEAREST);
    cv::cvtColor(resized_buf, bgr565_buf, cv::COLOR_BGR2BGR565);
  }

  std::memcpy(fbp, bgr565_buf.data, screensize);
}
