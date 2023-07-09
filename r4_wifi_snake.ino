#include "Frame.h"
#include "PlayStation.h"

uint8_t row, col;

void setup() {
  PlayStation.begin();
  Frame.begin();
  row = Frame.HEIGHT / 2;
  col = Frame.WIDTH / 2;
}

void loop() {
  Frame.plot(row, col, false);
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  if ((pressed & PlayStation.Left) && col > 0) --col;
  if ((pressed & PlayStation.Right) && col < Frame.WIDTH - 1) ++col;
  if ((pressed & PlayStation.Up) && row > 0) --row;
  if ((pressed & PlayStation.Down) && row < Frame.HEIGHT - 1) ++row;
  Frame.plot(row, col, true);
  Frame.render();
}