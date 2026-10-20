// src/PDController.cpp
#include "PDController.hpp"
#include <cmath>
#include <algorithm>

PDController::PDController(double p, double d, double max_spd, int deadband)
  : kp(p), kd(d), max_speed(max_spd), deadband_px(deadband) {
}

void PDController::reset() {
  prev_error = 0.0;
  first_sample = true;
}

int PDController::compute(double error, double dt) {
  if (std::abs(error) <= deadband_px) {
    prev_error = error;
    first_sample = false;
    return 0;
  }

  double derivative = 0.0;
  if (!first_sample && dt > 0.0001) {
    derivative = (error - prev_error) / dt;
  }
  first_sample = false;
  prev_error = error;

  double output = std::clamp((kp * error) + (kd * derivative), -max_speed, max_speed);
  return static_cast<int>(std::round(output));
}
