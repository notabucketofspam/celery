#pragma once

#include <string>

class UartController {
private:
  int serial_fd = -1;

public:
  UartController() = default;
  ~UartController();

  UartController(const UartController&) = delete;
  UartController& operator=(const UartController&) = delete;

  bool connect(const std::string& port_name);
  void sendSpeeds(int speed_x, int speed_y);
};
