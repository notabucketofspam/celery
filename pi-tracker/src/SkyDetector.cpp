// src/SkyDetector.cpp
#include "SkyDetector.hpp"
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>

SkyDetector::SkyDetector(double min_a, double max_a, double thresh_offset)
  : min_area(min_a), max_area(max_a), threshold_val(thresh_offset) {
}

bool SkyDetector::findTarget(const cv::Mat& frame, cv::Rect& out_bbox) const {
  cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
  cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

  // Calculate average background brightness and threshold strictly darker objects
  double mean_brightness = cv::mean(gray)[0];
  double dynamic_thresh = std::max(20.0, mean_brightness - threshold_val);

  cv::threshold(gray, binary, dynamic_thresh, 255, cv::THRESH_BINARY_INV);

  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  double best_area = 0.0;
  cv::Rect best_rect;
  const int margin = 15;

  for (const auto& contour : contours) {
    double area = cv::contourArea(contour);
    if (area >= min_area && area <= max_area) {
      cv::Rect rect = cv::boundingRect(contour);

      // Reject elongated horizon lines (aspect ratio > 4:1) and frame edges
      double aspect = static_cast<double>(std::max(rect.width, rect.height)) /
        std::max(1, std::min(rect.width, rect.height));
      if (aspect > 4.0) continue;

      if (rect.x <= margin || rect.y <= margin ||
        rect.x + rect.width >= frame.cols - margin ||
        rect.y + rect.height >= frame.rows - margin) {
        continue;
      }

      if (area > best_area) {
        best_area = area;
        best_rect = rect;
      }
    }
  }

  if (best_area > 0.0) {
    int pad = 6;
    best_rect.x = std::max(0, best_rect.x - pad);
    best_rect.y = std::max(0, best_rect.y - pad);
    best_rect.width = std::min(frame.cols - best_rect.x, best_rect.width + pad * 2);
    best_rect.height = std::min(frame.rows - best_rect.y, best_rect.height + pad * 2);
    out_bbox = best_rect;
    return true;
  }
  return false;
}
