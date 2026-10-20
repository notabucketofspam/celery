// src/PDController.cpp
#include "PDController.hpp"
#include <cmath>
#include <algorithm>

PDController::PDController(double p, double d, double max_spd, int deadband, bool invert)
  : kp(p), kd(d), max_speed(max_spd), deadband_px(deadband),
  direction_sign(invert ? -1.0 : 1.0) {
}

void PDController::reset() {
  prev_error = 0.0;
  filtered_derivative = 0.0;
  first_sample = true;
}

int PDController::compute(double error, double dt) {
  if (std::abs(error) <= deadband_px) {
    prev_error = error;
    filtered_derivative = 0.0;
    first_sample = false;
    return 0;
  }

  if (!first_sample && dt > 0.0001) {
    double raw_derivative = (error - prev_error) / dt;
    // Low-pass filter (alpha = 0.4) to smooth out integer pixel step noise
    filtered_derivative = (0.4 * raw_derivative) + (0.6 * filtered_derivative);
  } else {
    filtered_derivative = 0.0;
  }

  first_sample = false;
  prev_error = error;

  double output = direction_sign * ((kp * error) + (kd * filtered_derivative));
  output = std::clamp(output, -max_speed, max_speed);
  return static_cast<int>(std::round(output));
}
