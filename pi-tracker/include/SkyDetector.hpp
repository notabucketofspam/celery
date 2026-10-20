// include/SkyDetector.hpp
#pragma once

#include <opencv2/core.hpp>

class SkyDetector {
private:
  double min_area;
  double max_area;
  double threshold_val;
  mutable cv::Mat gray;
  mutable cv::Mat binary;

public:
  SkyDetector(double min_a = 150.0, double max_a = 12000.0, double thresh = 90.0);

  bool findTarget(const cv::Mat& frame, cv::Rect& out_bbox) const;
};
