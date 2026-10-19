#pragma once

class PDController {
private:
  double kp;
  double kd;
  double max_speed;
  int deadband_px;
  double prev_error = 0.0;

public:
  PDController(double p, double d, double max_spd, int deadband);

  void reset();
  int compute(double error, double dt);
};
