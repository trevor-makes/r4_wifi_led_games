#include "PlayStation.h"

PlayStation_t PlayStation;

// See https://github.com/arduino/ArduinoCore-renesas/issues/12 for SPI fix
#define SPI_HACK
#if defined(ARDUINO_MINIMA)
#define SPI_SPCMD R_SPI1->SPCMD_b[0]
#elif defined(ARDUINO_UNOWIFIR4)
#define SPI_SPCMD R_SPI0->SPCMD_b[0]
#endif

void PlayStation_t::begin(pin_size_t chip_select) {
  chip_select_ = chip_select;
  pinMode(chip_select_, OUTPUT);
  digitalWrite(chip_select_, HIGH);
  SPI.begin();
}

void PlayStation_t::update() {
  SPI.beginTransaction(settings_);
#ifdef SPI_HACK
  // Hack to set LSBFIRST and SPI_MODE3
  SPI_SPCMD.CPHA = 1;
  SPI_SPCMD.CPOL = 1;
  SPI_SPCMD.LSBF = 1;
#endif
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
#ifdef SPI_HACK
  // Hack to clear LSBFIRST and SPI_MODE3
  SPI_SPCMD.CPHA = 0;
  SPI_SPCMD.CPOL = 0;
  SPI_SPCMD.LSBF = 0;
#endif

  last_buttons_ = buttons_;
  buttons_ = ~((buttons2 << 8) | buttons1);
}