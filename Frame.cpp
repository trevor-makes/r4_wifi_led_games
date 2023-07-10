#include "Frame.h"

Frame_t Frame;

void Frame_t::begin() {
  matrix_.begin();
  clear();
}

void Frame_t::clear() {
  memset(frame_, 0, sizeof(frame_));
}

void Frame_t::plot(uint8_t row, uint8_t col, bool set) {
#ifdef FRAME_PACKED
  uint8_t index = row * 12 + col;
  uint32_t field = (1u << 31) >> (index % 32);
  if (set) {
    frame_[index / 32] |= field;
  } else {
    frame_[index / 32] &= ~field;
  }
#else
  frame_[row][col] = set;
#endif
}

void Frame_t::render() {
#ifdef FRAME_PACKED
  matrix_.loadFrame(frame_);
#else
  matrix_.renderBitmap(frame_, HEIGHT, WIDTH);
#endif
}