// src/main.cpp
#include <iostream>
#include <string>
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

  PiCameraStream cap(FRAME_WIDTH, FRAME_HEIGHT, 30);
  if (!cap.isOpened()) {
    std::cerr << "[Camera] Failed to open rpicam-vid pipe!\n";
    return -1;
  }

  FramebufferDisplay display("/dev/fb0");
  SkyDetector detector(150.0, 8000.0, 45.0);

  // Flip the last boolean to 'true' if either motor turns away from the target
  PDController pid_pan(6.0, 0.35, 3500.0, 5, false);
  PDController pid_tilt(6.0, 0.35, 3500.0, 5, false);

  cv::TrackerCSRT::Params csrt_params;
  csrt_params.psr_threshold = 0.06f;
  csrt_params.use_color_names = false;
  csrt_params.use_gray = true;
  csrt_params.use_hog = true;
  csrt_params.template_size = 100;
  csrt_params.number_of_scales = 17;

  std::shared_ptr<cv::Tracker> tracker;
  cv::Rect bbox;
  bool is_tracking = false;
  int cooldown_frames = 0;
  int frame_counter = 0;

  int last_speed_x = 0;
  int last_speed_y = 0;
  double smoothed_fps = 30.0;

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
    if (dt > 0.0001) {
      smoothed_fps = (0.9 * smoothed_fps) + (0.1 * (1.0 / dt));
    }

    if (!is_tracking) {
      last_speed_x = 0;
      last_speed_y = 0;
      ramps.sendSpeeds(0, 0);
      pid_pan.reset();
      pid_tilt.reset();

      if (cooldown_frames > 0) {
        // Wait for physical turret to stop moving before scanning sky
        cooldown_frames--;
      } else if (detector.findTarget(frame, bbox)) {
        tracker = cv::TrackerCSRT::create(csrt_params);
        tracker->init(frame, bbox);
        is_tracking = true;
        std::cout << "[Tracker] Locked onto target [" << bbox.x << "," << bbox.y
          << " " << bbox.width << "x" << bbox.height << "]\n";
      }
    } else {
      bool ok = tracker->update(frame, bbox);

      cv::Rect frame_bounds(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
      cv::Rect clamped = bbox & frame_bounds;

      if (!ok || clamped.area() < 50 || clamped.area() > 20000) {
        std::cout << "[Tracker] Target lost! Settling turret...\n";
        is_tracking = false;
        cooldown_frames = 10; // ~300ms mechanical settling pause
        last_speed_x = 0;
        last_speed_y = 0;
        ramps.sendSpeeds(0, 0);
      } else {
        double target_x = bbox.x + (bbox.width * 0.5);
        double target_y = bbox.y + (bbox.height * 0.5);

        double error_x = target_x - CENTER_X;
        double error_y = target_y - CENTER_Y;

        last_speed_x = pid_pan.compute(error_x, dt);
        last_speed_y = pid_tilt.compute(error_y, dt);

        ramps.sendSpeeds(last_speed_x, last_speed_y);
      }
    }

    // Update SPI display every 3rd frame with field diagnostics HUD
    frame_counter++;
    if (display.isOpened() && (frame_counter % 3 == 0)) {
      cv::drawMarker(frame, cv::Point(CENTER_X, CENTER_Y),
        cv::Scalar(255, 100, 0), cv::MARKER_CROSS, 16, 1);

      if (is_tracking) {
        cv::rectangle(frame, bbox, cv::Scalar(0, 255, 0), 2);
        cv::drawMarker(frame, cv::Point(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2),
          cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 16, 2);
      }

      // Draw dark header bar for text readability against bright sky
      cv::rectangle(frame, cv::Point(0, 0), cv::Point(FRAME_WIDTH, 24),
        cv::Scalar(0, 0, 0), cv::FILLED);

      std::string state_str = is_tracking ? "LOCKED" : (cooldown_frames > 0 ? "SETTLING" : "SEARCH");
      cv::Scalar state_color = is_tracking ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 200, 255);

      std::string hud = state_str +
        " | FPS: " + std::to_string(static_cast<int>(smoothed_fps)) +
        " | SPD: <" + std::to_string(last_speed_x) + "," + std::to_string(last_speed_y) + ">";

      cv::putText(frame, hud, cv::Point(8, 17), cv::FONT_HERSHEY_SIMPLEX,
        0.45, state_color, 1, cv::LINE_AA);

      display.show(frame);
    }
  }

  std::cout << "[Tracker] Stopping motors and exiting.\n";
  ramps.sendSpeeds(0, 0);
  return 0;
}