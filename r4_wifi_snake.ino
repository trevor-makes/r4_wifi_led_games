#include <Arduino_LED_Matrix.h>

#include "PlayStation.h"

constexpr size_t WIDTH = 12;
constexpr size_t HEIGHT = 8;

class {
private:
  ArduinoLEDMatrix matrix_;
  //byte frame_[HEIGHT][WIDTH];
  uint32_t frame_[3];
public:
  void begin() {
    matrix_.begin();
    memset(frame_, 0, sizeof(frame_));
  }
  void plot(uint8_t row, uint8_t col, bool set) {
    //frame_[row][col] = set;
    uint8_t index = row * 12 + col;
    uint32_t field = (1u << 31) >> (index % 32);
    if (set) {
      frame_[index / 32] |= field;
    } else {
      frame_[index / 32] &= ~field;
    }
  }
  void render() {
    //matrix_.renderBitmap(frame_, HEIGHT, WIDTH);
    matrix_.loadFrame(frame_);
  }
} frame;

uint8_t row, col;

void setup() {
  PlayStation.begin();
  frame.begin();
  row = HEIGHT / 2;
  col = WIDTH / 2;
}

void loop() {
  frame.plot(row, col, false);
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  if ((pressed & PlayStation.Left) && col > 0) --col;
  if ((pressed & PlayStation.Right) && col < WIDTH - 1) ++col;
  if ((pressed & PlayStation.Up) && row > 0) --row;
  if ((pressed & PlayStation.Down) && row < HEIGHT - 1) ++row;
  frame.plot(row, col, true);
  frame.render();
}