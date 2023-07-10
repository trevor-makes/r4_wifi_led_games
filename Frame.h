#pragma once

#include <Arduino_LED_Matrix.h>

#define FRAME_PACKED

class Frame_t {
public:
  static constexpr size_t WIDTH = 12;
  static constexpr size_t HEIGHT = 8;
private:
  ArduinoLEDMatrix matrix_;
#ifdef FRAME_PACKED
  uint32_t frame_[3];
#else
  uint8_t frame_[HEIGHT][WIDTH];
#endif
public:
  void begin();
  void clear();
  void plot(uint8_t row, uint8_t col, bool set);
  void render();
};

extern Frame_t Frame;