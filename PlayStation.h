#pragma once

#include <SPI.h>

class PlayStation_t {
private:
  pin_size_t chip_select_;
  uint16_t buttons_ = 0;
  uint16_t last_buttons_ = 0;
  const SPISettings settings_{250000, LSBFIRST, SPI_MODE3};

public:
  void begin(pin_size_t chip_select = 10);

  void update();

  uint16_t get_held() { return buttons_; }
  uint16_t get_pressed() { return buttons_ & ~last_buttons_; }
  uint16_t get_released() { return ~buttons_ & last_buttons_; }

  enum {
    Select   = 0x0001,
    L3       = 0x0002,
    R3       = 0x0004,
    Start    = 0x0008,
    Up       = 0x0010,
    Right    = 0x0020,
    Down     = 0x0040,
    Left     = 0x0080,
    L2       = 0x0100,
    R2       = 0x0200,
    L1       = 0x0400,
    R1       = 0x0800,
    Triangle = 0x1000,
    Circle   = 0x2000,
    Cross    = 0x4000,
    Square   = 0x8000,
  };
};

extern PlayStation_t PlayStation;