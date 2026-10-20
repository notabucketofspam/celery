// src/UartController.cpp
#include "UartController.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

UartController::~UartController() {
  if (serial_fd >= 0) {
    sendSpeeds(0, 0);
    close(serial_fd);
  }
}

bool UartController::connect(const std::string& port_name) {
  serial_fd = open(port_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
  if (serial_fd < 0) {
    std::cerr << "[UART] Note: " << port_name << " not connected ("
      << strerror(errno) << "). Running in dry-run mode.\n";
    return false;
  }

  struct termios tty {};
  if (tcgetattr(serial_fd, &tty) != 0) {
    std::cerr << "[UART] tcgetattr failed\n";
    return false;
  }

  cfsetospeed(&tty, B115200);
  cfsetispeed(&tty, B115200);

  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  tty.c_cflag &= ~CRTSCTS;
  tty.c_cflag |= CREAD | CLOCAL;

  tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  tty.c_iflag &= ~(IXON | IXOFF | IXANY | IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
  tty.c_oflag &= ~OPOST;

  tcsetattr(serial_fd, TCSANOW, &tty);

  std::cout << "[UART] Connected to " << port_name << ". Waiting 2s for RAMPS boot...\n";
  std::this_thread::sleep_for(std::chrono::seconds(2));
  tcflush(serial_fd, TCIOFLUSH);
  return true;
}

void UartController::sendSpeeds(int speed_x, int speed_y) {
  if (serial_fd < 0) return;
  std::string packet = "<" + std::to_string(speed_x) + "," + std::to_string(speed_y) + ">";
  ssize_t written = write(serial_fd, packet.c_str(), packet.size());
  (void)written;
}
