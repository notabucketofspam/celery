#include <iostream>
#include <chrono>
#include <csignal>
#include <memory>

#include <opencv2/core.hpp>
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
  ramps.connect("/dev/ttyACM0");

  PiCameraStream cap(FRAME_WIDTH, FRAME_HEIGHT, 60);
  if (!cap.isOpened()) {
    std::cerr << "[Camera] Failed to open rpicam-vid pipe!\n";
    return -1;
  }

  SkyDetector detector(25.0, 15000.0, 100.0);
  PDController pid_pan(6.0, 0.35, 3500.0, 5);
  PDController pid_tilt(6.0, 0.35, 3500.0, 5);

  std::shared_ptr<cv::Tracker> tracker;
  cv::Rect bbox;
  bool is_tracking = false;

  cv::Mat frame;
  auto last_time = std::chrono::steady_clock::now();

  std::cout << "[Tracker] OpenCV " << CV_VERSION << " initialized (" << FRAME_WIDTH << "x" << FRAME_HEIGHT << " @ 60fps).\n";

  FramebufferDisplay display("/dev/fb0");
  int frame_counter = 0;

  while (keep_running) {
    if (!cap.read(frame) || frame.empty()) {
      std::cerr << "[Camera] Stream ended or failed to read frame.\n";
      break;
    }

    auto now = std::chrono::steady_clock::now();
    double dt = std::chrono::duration<double>(now - last_time).count();
    last_time = now;
    double fps = (dt > 0.0) ? (1.0 / dt) : 0.0;

    if (!is_tracking) {
      ramps.sendSpeeds(0, 0);
      pid_pan.reset();
      pid_tilt.reset();

      if (detector.findTarget(frame, bbox)) {
        tracker = cv::TrackerCSRT::create();
        tracker->init(frame, bbox);
        is_tracking = true;
        std::cout << "\n[Tracker] Locked onto target [" << bbox.x << "," << bbox.y
          << " " << bbox.width << "x" << bbox.height << "]\n";
      }
    } else {
      if (tracker->update(frame, bbox)) {
        int target_x = bbox.x + (bbox.width / 2);
        int target_y = bbox.y + (bbox.height / 2);

        double error_x = static_cast<double>(target_x - CENTER_X);
        double error_y = static_cast<double>(target_y - CENTER_Y);

        int speed_x = pid_pan.compute(error_x, dt);
        int speed_y = pid_tilt.compute(error_y, dt);

        ramps.sendSpeeds(speed_x, speed_y);

        std::cout << "\r[TRACKING] FPS: " << static_cast<int>(fps)
          << " | Err: (" << error_x << ", " << error_y << ")"
          << " | Cmd: <" << speed_x << "," << speed_y << ">      " << std::flush;
      } else {
        std::cout << "\n[Tracker] Target lost! Searching...\n";
        is_tracking = false;
        ramps.sendSpeeds(0, 0);
      }
    }
    frame_counter++;
    // Only update the SPI screen every 4th frame (~15 FPS) to prevent lag
    if (display.isOpened() && (frame_counter % 4 == 0)) {
      cv::Mat display_frame;
      frame.copyTo(display_frame);

      if (is_tracking) {
        // Draw a thick green rectangle around the drone
        cv::rectangle(display_frame, bbox, cv::Scalar(0, 255, 0), 3);

        // Draw a center crosshair
        cv::drawMarker(display_frame, cv::Point(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2),
          cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 20, 2);
      }

      display.show(display_frame);
    }
  }

  std::cout << "\n[Tracker] Stopping motors and exiting.\n";
  ramps.sendSpeeds(0, 0);
  return 0;
}
