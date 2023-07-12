#include "PlayStation.h"

PlayStation_t PlayStation;

void PlayStation_t::begin(pin_size_t chip_select) {
  chip_select_ = chip_select;
  pinMode(chip_select_, OUTPUT);
  digitalWrite(chip_select_, HIGH);
  SPI.begin();
}

void PlayStation_t::update() {
  SPI.beginTransaction(settings_);
  digitalWrite(chip_select_, LOW);

  delayMicroseconds(50);
  uint8_t first = SPI.transfer(0x01);
  delayMicroseconds(20);
  uint8_t mode = SPI.transfer(0x42);
  delayMicroseconds(20);
  uint8_t third = SPI.transfer(0x00);
  delayMicroseconds(20);
  uint8_t buttons1 = SPI.transfer(0x00);
  delayMicroseconds(20);
  uint8_t buttons2 = SPI.transfer(0x00);
  delayMicroseconds(20);

  digitalWrite(chip_select_, HIGH);
  SPI.endTransaction();

  last_buttons_ = buttons_;
  buttons_ = ~((buttons2 << 8) | buttons1);
}