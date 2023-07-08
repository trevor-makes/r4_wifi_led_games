#include "PlayStation.h"

void setup() {
  PlayStation.begin();

  Serial.begin(9600);
  while (!Serial) {}
}

void loop() {
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  if (pressed & PlayStation.Left) {
    Serial.println("Left");
  }
  if (pressed & PlayStation.Right) {
    Serial.println("Right");
  }
  if (pressed & PlayStation.Up) {
    Serial.println("Up");
  }
  if (pressed & PlayStation.Down) {
    Serial.println("Down");
  }
  delay(10);
}
