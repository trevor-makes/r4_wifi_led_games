#include "Arduino_LED_Matrix.h"
#include "PlayStation.h"

ArduinoLEDMatrix matrix;

constexpr size_t WIDTH = 12;
constexpr size_t HEIGHT = 8;
byte frame[HEIGHT][WIDTH];
uint8_t row, col;

void setup() {
  PlayStation.begin();
  matrix.begin();
  memset(frame, 0, sizeof(frame));
  row = HEIGHT / 2;
  col = WIDTH / 2;
}

void loop() {
  frame[row][col] = 0;
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  if ((pressed & PlayStation.Left) && col > 0) --col;
  if ((pressed & PlayStation.Right) && col < WIDTH - 1) ++col;
  if ((pressed & PlayStation.Up) && row > 0) --row;
  if ((pressed & PlayStation.Down) && row < HEIGHT - 1) ++row;
  frame[row][col] = 1;
  matrix.renderBitmap(frame, HEIGHT, WIDTH);
}
