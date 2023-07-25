#include "Tetro.h"

#include <stdint.h>

#include "PlayStation.h"
#include "Frame.h"

struct Shape {
  uint8_t count;
  uint16_t data[];
};

Shape shape_I = {
    .count = 2,
    .data = {
        //1248
        //.... 0
        //#### F
        //.... 0
        //.... 0
        0x0F00,
        //1248
        //.#.. 2
        //.#.. 2
        //.#.. 2
        //.#.. 2
        0x2222,
    },
};

Shape shape_Z = {
    .count = 2,
    .data = {
        //1248
        //##.. 3
        //.##. 6
        //.... 0
        //.... 0
        0x3600,
        //1248
        //.#.. 2
        //##.. 3
        //#... 1
        //.... 0
        0x2310,
    },
};

Shape shape_S = {
    .count = 2,
    .data = {
        //1248
        //.##. 6
        //##.. 3
        //.... 0
        //.... 0
        0x6300,
        //1248
        //#... 1
        //##.. 3
        //.#.. 2
        //.... 0
        0x1320,
    },
};

Shape shape_O = {
    .count = 1,
    .data = {
        //1248
        //##.. 3
        //##.. 3
        //.... 0
        //.... 0
        0x3300
    },
};

Shape shape_T = {
    .count = 4,
    .data = {
        //1248
        //.... 0
        //###. 7
        //.#.. 2
        //.... 0
        0x0720,
        //1248
        //.#.. 2
        //##.. 3
        //.#.. 2
        //.... 0
        0x2320,
        //1248
        //.#.. 2
        //###. 7
        //.... 0
        //.... 0
        0x2700,
        //1248
        //.#.. 2
        //.##. 6
        //.#.. 2
        //.... 0
        0x2620,
    },
};

Shape shape_J = {
    .count = 4,
    .data = {
        //1248
        //.#.. 2
        //.#.. 2
        //##.. 3
        //.... 0
        0x2230,
        //1248
        //#... 1
        //###. 7
        //.... 0
        //.... 0
        0x1700,
        //1248
        //.##. 6
        //.#.. 2
        //.#.. 2
        //.... 0
        0x6220,
        //1248
        //.... 0
        //###. 7
        //..#. 4
        //.... 0
        0x0740,
    },
};

Shape shape_L = {
    .count = 4,
    .data = {
        //1248
        //.#.. 2
        //.#.. 2
        //.##. 6
        //.... 0
        0x2260,
        //1248
        //.... 0
        //###. 7
        //#... 1
        //.... 0
        0x0710,
        //1248
        //##.. 3
        //.#.. 2
        //.#.. 2
        //.... 0
        0x3220,
        //1248
        //..#. 4
        //###. 7
        //.... 0
        //.... 0
        0x4700,
    },
};

Shape* shapes[7] = {&shape_I, &shape_Z, &shape_S, &shape_O, &shape_T, &shape_J, &shape_L};

constexpr uint8_t NUM_ROWS = Frame.WIDTH;
constexpr uint8_t NUM_COLS = Frame.HEIGHT;
uint8_t row_mask[NUM_ROWS];
static_assert(sizeof(row_mask[0]) * 8 >= NUM_COLS, "bitfield size must exceed num columns");

void draw_tetro_row(uint8_t row, uint8_t col, uint8_t tetro_row, bool set) {
  while (tetro_row > 0) {
    if ((tetro_row & 1) != 0) {
      // TODO add transform function or something
      Frame.plot(NUM_COLS - col, row, set); // flip row and col to rotate
    }
    ++col;
    tetro_row >>= 1;
  }
}

void draw_tetro(uint8_t row, uint8_t col, uint16_t tetro, bool set) {
  draw_tetro_row(row, col, tetro >> 12 & 0x0F, set);
  draw_tetro_row(row + 1, col, tetro >> 8 & 0x0F, set);
  draw_tetro_row(row + 2, col, tetro >> 4 & 0x0F, set);
  draw_tetro_row(row + 3, col, tetro & 0x0F, set);
}

// TODO handle shape out of bounds
bool test_tetro_row(uint8_t row, uint8_t col, uint8_t tetro_row) {
  if (tetro_row == 0) return false;
  if (row >= NUM_ROWS) return true;
  return (row_mask[row] & (tetro_row << col)) != 0;
}

bool test_tetro(uint8_t row, uint8_t col, uint16_t tetro) {
  if (test_tetro_row(row, col, tetro >> 12 & 0x0F)) return true;
  if (test_tetro_row(row + 1, col, tetro >> 8 & 0x0F)) return true;
  if (test_tetro_row(row + 2, col, tetro >> 4 & 0x0F)) return true;
  return test_tetro_row(row + 3, col, tetro & 0x0F);
}

void place_tetro_row(uint8_t row, uint8_t col, uint8_t tetro_row) {
  if (tetro_row == 0) return;
  if (row >= NUM_ROWS) return;
  row_mask[row] |= tetro_row << col;
  while (tetro_row > 0) {
    if ((tetro_row & 1) != 0) {
      // TODO add transform function or something
      Frame.plot(NUM_COLS - col, row, true); // flip row and col to rotate
    }
    ++col;
    tetro_row >>= 1;
  }
}

void place_tetro(uint8_t row, uint8_t col, uint16_t tetro) {
  place_tetro_row(row + 0, col, (tetro >> 12) & 0x0F);
  place_tetro_row(row + 1, col, (tetro >> 8) & 0x0F);
  place_tetro_row(row + 2, col, (tetro >> 4) & 0x0F);
  place_tetro_row(row + 3, col, (tetro >> 0) & 0x0F);
}

// TODO rename/put these in object/namespace; collision with row/col parameters above
Shape* shape_ptr;
uint8_t row;
uint8_t col;
uint8_t rot;
uint16_t cur_tetro;

bool next_shape() {
  // TODO random shuffle deck of shapes or w/e rando algo
  static uint8_t num = 0;
  if (num >= 7) num = 0;

  shape_ptr = shapes[num++];
  row = 0;
  col = NUM_COLS / 2;
  rot = 0;
  cur_tetro = shape_ptr->data[rot];

  if (test_tetro(row, col, cur_tetro)) {
    return false;
  }

  draw_tetro(row, col, cur_tetro, true);
  return true;
}

uint8_t clear_rows(uint8_t row) {
  uint8_t rows_cleared = 0;
  uint8_t end = row + 4; // TODO magic number (shape height)
  for (; row < end; ++row) {
    if (row_mask[row] == uint8_t(~0)) {
      for (uint8_t col = 0; col < NUM_COLS; ++col) {
        // TODO add transform function or something
        Frame.plot(NUM_COLS - col, row, false); // flip row and col to rotate
      }
      ++rows_cleared;
      for (uint8_t i = row; i > 0; --i) {
        row_mask[i] = row_mask[i - 1];
      }
      row_mask[0] = 0;
    }
  }
  return rows_cleared;
}

void tetro_loop(Timer& timer) {
  // TODO add game over state
  static bool is_game_over = false;

  // TODO add an init call/state to do the following
  static bool first_call = true;
  if (first_call) {
    is_game_over = !next_shape();
    first_call = false;
  }

  // TODO use state machine, add end screen, back to menu, etc
  if (is_game_over) return;

  // Clear shape before moving with controller
  draw_tetro(row, col, cur_tetro, false);

  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  if (pressed & PlayStation.Left) {
    if (!test_tetro(row, col - 1, cur_tetro)) {
      --col;
    }
  }
  if (pressed & PlayStation.Right) {
    if (!test_tetro(row, col + 1, cur_tetro)) {
      ++col;
    }
  }
  if (pressed & PlayStation.Up) {
    uint8_t next_rot = (shape_ptr->count + rot - 1) % shape_ptr->count;
    uint16_t next_tetro = shape_ptr->data[next_rot];
    if (!test_tetro(row, col, next_tetro)) {
      rot = next_rot;
      cur_tetro = next_tetro;
    }
  }
  if (pressed & PlayStation.Down) {
    // Rotate shape forward
    uint8_t next_rot = (rot + 1) % shape_ptr->count;
    uint16_t next_tetro = shape_ptr->data[next_rot];
    if (!test_tetro(row, col, next_tetro)) {
      rot = next_rot;
      cur_tetro = next_tetro;
    }
  }

  // Wait for next timer tick
  const bool drop_now = (pressed & PlayStation.Cross);
  if (drop_now || timer.did_tick()) {
    if (test_tetro(row + 1, col, cur_tetro)) {
      place_tetro(row, col, cur_tetro);
      draw_tetro(row, col, cur_tetro, true);
      if (clear_rows(row) > 0) {
        // TODO scroll/redraw rows above
        // TODO increment score based on num rows cleared
        // TODO Uno R4 can just use float
        auto period = timer.get_period();
        period = (period >> 4) + (period >> 3) + (period >> 2) + (period >> 1);
        timer.set_period(period);
      }
      if (next_shape() == false) {
        // TODO add game over state transition
        is_game_over = true;
      }
    } else {
      // Move shape down
      ++row;
    }
  }

  // Redraw shape after moving
  draw_tetro(row, col, cur_tetro, true);

  Frame.render();
}