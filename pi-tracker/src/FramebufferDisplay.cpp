#include "FramebufferDisplay.hpp"
#include <iostream>
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

  struct fb_var_screeninfo vinfo;
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
    std::cerr << "[Display] Error reading variable information.\n";
    close(fbfd);
    fbfd = -1;
    return;
  }

  width = vinfo.xres;
  height = vinfo.yres;
  bpp = vinfo.bits_per_pixel;
  screensize = (long)width * height * (bpp / 8);

  fbp = (char*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
  if (reinterpret_cast<long>(fbp) == -1) {
    std::cerr << "[Display] Error: failed to map framebuffer device to memory.\n";
    fbp = nullptr;
    close(fbfd);
    fbfd = -1;
  } else {
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
  if (!fbp || frame.empty()) return;

  cv::Mat resized, bgr565;

  // Resize to fit the physical screen exactly
  cv::resize(frame, resized, cv::Size(width, height));

  // Convert OpenCV's 24-bit BGR to the screen's required 16-bit RGB565 layout
  cv::cvtColor(resized, bgr565, cv::COLOR_BGR2BGR565);

  // Blast the pixels directly into the kernel's hardware memory space
  std::memcpy(fbp, bgr565.data, screensize);
}
