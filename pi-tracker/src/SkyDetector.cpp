#include "SkyDetector.hpp"
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>

SkyDetector::SkyDetector(double min_a, double max_a, double thresh)
  : min_area(min_a), max_area(max_a), threshold_val(thresh) {
}

bool SkyDetector::findTarget(const cv::Mat& frame, cv::Rect& out_bbox) const {
  cv::Mat gray, binary;
  cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
  cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

  cv::threshold(gray, binary, threshold_val, 255, cv::THRESH_BINARY_INV);

  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  double best_area = 0.0;
  cv::Rect best_rect;

  for (const auto& contour : contours) {
    double area = cv::contourArea(contour);
    if (area >= min_area && area <= max_area && area > best_area) {
      best_area = area;
      best_rect = cv::boundingRect(contour);
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
