#include "PDController.hpp"
#include <cmath>
#include <algorithm>

PDController::PDController(double p, double d, double max_spd, int deadband)
  : kp(p), kd(d), max_speed(max_spd), deadband_px(deadband) {
}

void PDController::reset() {
  prev_error = 0.0;
}

int PDController::compute(double error, double dt) {
  if (std::abs(error) <= deadband_px) {
    prev_error = error;
    return 0;
  }

  double derivative = (dt > 0.0001) ? ((error - prev_error) / dt) : 0.0;
  prev_error = error;

  double output = std::clamp((kp * error) + (kd * derivative), -max_speed, max_speed);
  return static_cast<int>(std::round(output));
}
