#pragma once

#include <opencv2/core.hpp>

class SkyDetector {
private:
  double min_area;
  double max_area;
  double threshold_val;

public:
  SkyDetector(double min_a = 25.0, double max_a = 15000.0, double thresh = 100.0);

  bool findTarget(const cv::Mat& frame, cv::Rect& out_bbox) const;
};
