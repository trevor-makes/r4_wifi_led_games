#include "Frame.h"

Frame_t Frame;

void Frame_t::begin() {
  matrix_.begin();
  fill(false);
}

void Frame_t::fill(bool set) {
#ifdef FRAME_PACKED
  const int pattern =  set ? 0xFF : 0;
#else
  const int pattern =  set ? 1 : 0;
#endif
  memset(frame_, pattern, sizeof(frame_));
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

constexpr uint32_t DIGIT0 = 0b\
0110\
1010\
1010\
1010\
1010\
1110\
0000\
0000;

constexpr uint32_t DIGIT1 = 0b\
0110\
1010\
0010\
0010\
0010\
0010\
0000\
0000;

constexpr uint32_t DIGIT2 = 0b\
0110\
1010\
0010\
0100\
1000\
1110\
0000\
0000;

constexpr uint32_t DIGIT3 = 0b\
1100\
0010\
0110\
0010\
1010\
1110\
0000\
0000;

constexpr uint32_t DIGIT4 = 0b\
1000\
1010\
1010\
1110\
0010\
0010\
0000\
0000;

constexpr uint32_t DIGIT5 = 0b\
1110\
1000\
1100\
0010\
1010\
1100\
0000\
0000;

constexpr uint32_t DIGIT6 = 0b\
0110\
1000\
1100\
1010\
1010\
1110\
0000\
0000;

constexpr uint32_t DIGIT7 = 0b\
1110\
0010\
0100\
1000\
1000\
1000\
0000\
0000;

constexpr uint32_t DIGIT8 = 0b\
1110\
1010\
1110\
1010\
1010\
1110\
0000\
0000;

constexpr uint32_t DIGIT9 = 0b\
1110\
1010\
1110\
0010\
1010\
1110\
0000\
0000;

constexpr uint32_t DIGITA = 0b\
0110\
1010\
1010\
1110\
1010\
1010\
0000\
0000;

constexpr uint32_t DIGITB = 0b\
1100\
1010\
1010\
1100\
1010\
1110\
0000\
0000;

constexpr uint32_t DIGITC = 0b\
0110\
1010\
1000\
1000\
1010\
1110\
0000\
0000;

constexpr uint32_t DIGITS[10] = { DIGIT0, DIGIT1, DIGIT2, DIGIT3, DIGIT4, DIGIT5, DIGIT6, DIGIT7, DIGIT8, DIGIT9 };

constexpr uint8_t DIGIT_ROWS = 8;
constexpr uint8_t DIGIT_COLS = 4;

void Frame_t::plot_digit(uint8_t row, uint8_t col, uint8_t digit, bool set) {
  uint32_t digit_bits = DIGITS[digit];
  for (uint8_t drow = 0; drow < DIGIT_ROWS && row + drow < HEIGHT; ++drow) {
    for (uint8_t dcol = 0; dcol < DIGIT_COLS && col + dcol < WIDTH; ++dcol) {
      uint8_t index = drow * DIGIT_COLS + dcol;
      uint32_t field = (1u << 31) >> index;//(index % 32);
      if (digit_bits & field) plot(row + drow, col + dcol, set);
    }
  }
}