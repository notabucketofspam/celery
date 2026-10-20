#include <iostream>
#include <chrono>
#include <csignal>
#include <memory>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/tracking.hpp>

#include "PiCameraStream.hpp"
#include "UartController.hpp"
#include "PDController.hpp"
#include "SkyDetector.hpp"
#include "FramebufferDisplay.hpp"

volatile std::sig_atomic_t keep_running = 1;
void sigint_handler(int) {
  keep_running = 0;
}

int main() {
  std::signal(SIGINT, sigint_handler);

  const int FRAME_WIDTH = 480;
  const int FRAME_HEIGHT = 320;
  const int CENTER_X = FRAME_WIDTH / 2;
  const int CENTER_Y = FRAME_HEIGHT / 2;

  UartController ramps;
  // use "/dev/ttyACM0" if the arduino is plugged into the pi via a usb cable
  // use "/dev/ttyAMA5" if Arduino pin D19 is connected to Pi pin 32 (uart5)
  ramps.connect("/dev/ttyAMA5");

  // 30 FPS matches CSRT speed better than 60 FPS, preventing pipe buffer lag
  PiCameraStream cap(FRAME_WIDTH, FRAME_HEIGHT, 30);
  if (!cap.isOpened()) {
    std::cerr << "[Camera] Failed to open rpicam-vid pipe!\n";
    return -1;
  }

  FramebufferDisplay display("/dev/fb0");
  SkyDetector detector(150.0, 12000.0, 90.0);
  PDController pid_pan(6.0, 0.35, 3500.0, 5);
  PDController pid_tilt(6.0, 0.35, 3500.0, 5);

  // Raise CSRT's PSR confidence threshold so it drops lost targets cleanly
  cv::TrackerCSRT::Params csrt_params;
  csrt_params.psr_threshold = 0.06f;

  std::shared_ptr<cv::Tracker> tracker;
  cv::Rect bbox;
  bool is_tracking = false;
  int frame_counter = 0;

  cv::Mat frame;
  auto last_time = std::chrono::steady_clock::now();

  std::cout << "[Tracker] OpenCV " << CV_VERSION << " initialized (480x320).\n";

  while (keep_running) {
    if (!cap.read(frame) || frame.empty()) {
      std::cerr << "[Camera] Stream ended or failed to read frame.\n";
      break;
    }

    auto now = std::chrono::steady_clock::now();
    double dt = std::chrono::duration<double>(now - last_time).count();
    last_time = now;

    if (!is_tracking) {
      ramps.sendSpeeds(0, 0);
      pid_pan.reset();
      pid_tilt.reset();

      if (detector.findTarget(frame, bbox)) {
        tracker = cv::TrackerCSRT::create(csrt_params);
        tracker->init(frame, bbox);
        is_tracking = true;
        std::cout << "[Tracker] Locked onto target [" << bbox.x << "," << bbox.y
          << " " << bbox.width << "x" << bbox.height << "]\n";
      }
    } else {
      bool ok = tracker->update(frame, bbox);

      // Sanity-check bbox: drop lock if target leaves the frame or distorts wildly
      cv::Rect frame_bounds(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
      cv::Rect clamped = bbox & frame_bounds;
      if (!ok || clamped.area() < 50 || clamped.area() > 25000) {
        std::cout << "[Tracker] Target lost! Returning to search mode...\n";
        is_tracking = false;
        ramps.sendSpeeds(0, 0);
      } else {
        int target_x = bbox.x + (bbox.width / 2);
        int target_y = bbox.y + (bbox.height / 2);

        double error_x = static_cast<double>(target_x - CENTER_X);
        double error_y = static_cast<double>(target_y - CENTER_Y);

        int speed_x = pid_pan.compute(error_x, dt);
        int speed_y = pid_tilt.compute(error_y, dt);

        ramps.sendSpeeds(speed_x, speed_y);
      }
    }

    // Update SPI display every 3rd frame (~5-10 FPS) without cloning cv::Mat
    frame_counter++;
    if (display.isOpened() && (frame_counter % 3 == 0)) {
      // Draw center screen reference crosshair (blue)
      cv::drawMarker(frame, cv::Point(CENTER_X, CENTER_Y),
        cv::Scalar(255, 100, 0), cv::MARKER_CROSS, 16, 1);

      if (is_tracking) {
        cv::rectangle(frame, bbox, cv::Scalar(0, 255, 0), 2);
        cv::drawMarker(frame, cv::Point(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2),
          cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 16, 2);
      }
      display.show(frame);
    }
  }

  std::cout << "\n[Tracker] Stopping motors and exiting.\n";
  ramps.sendSpeeds(0, 0);
  return 0;
}
