// include/PDController.hpp
#pragma once

class PDController {
private:
  double kp;
  double kd;
  double max_speed;
  int deadband_px;
  double direction_sign;
  double prev_error = 0.0;
  double filtered_derivative = 0.0;
  bool first_sample = true;

public:
  PDController(double p, double d, double max_spd, int deadband, bool invert = false);

  void reset();
  int compute(double error, double dt);
};
