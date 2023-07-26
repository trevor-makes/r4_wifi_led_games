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

void draw_pixel(int8_t row, int8_t col, bool set) {
  if (row < 0 || row >= NUM_ROWS || col < 0 || col >= NUM_COLS) return;
  Frame.plot(NUM_COLS - 1 - col, row, set); // flip row and col to rotate
}

void draw_tetro_row(int8_t row, int8_t col, uint8_t tetro_row, bool set) {
  while (tetro_row > 0) {
    if ((tetro_row & 1) == 1) {
      draw_pixel(row, col, set);
    }
    ++col;
    tetro_row >>= 1;
  }
}

void draw_tetro(int8_t row, int8_t col, uint16_t tetro, bool set) {
  draw_tetro_row(row, col, tetro >> 12 & 0x0F, set);
  draw_tetro_row(row + 1, col, tetro >> 8 & 0x0F, set);
  draw_tetro_row(row + 2, col, tetro >> 4 & 0x0F, set);
  draw_tetro_row(row + 3, col, tetro & 0x0F, set);
}

bool test_tetro_row(int8_t row, int8_t col, uint8_t tetro_row) {
  if (tetro_row == 0) return false; // skip rows without pixels
  if (row < 0) return false; // ignore shapes falling from offscreen above
  if (row >= NUM_ROWS) return true; // block offscreen pixels below
  if (col < 0 && (tetro_row & ((1 << -col) - 1)) != 0) return true; // test left boundary
  if (tetro_row >> (NUM_COLS - col) != 0) return true; // test right boundary
  uint8_t tetro_mask = (col >= 0) ? (tetro_row << col) : (tetro_row >> -col);
  return (row_mask[row] & tetro_mask) != 0;
}

bool test_tetro(int8_t row, int8_t col, uint16_t tetro) {
  if (test_tetro_row(row + 0, col, (tetro >> 12) & 0x0F)) return true;
  if (test_tetro_row(row + 1, col, (tetro >> 8) & 0x0F)) return true;
  if (test_tetro_row(row + 2, col, (tetro >> 4) & 0x0F)) return true;
  return test_tetro_row(row + 3, col, (tetro >> 0) & 0x0F);
}

bool place_tetro_row(int8_t row, int8_t col, uint8_t tetro_row) {
  if (tetro_row == 0) return true;
  if (row < 0 || row >= NUM_ROWS) return false; // can't place offscreen
  if (col < 0 && (tetro_row & ((1 << -col) - 1)) != 0) return false; // test left boundary
  if (tetro_row >> (NUM_COLS - col) != 0) return false; // test right boundary
  uint8_t tetro_mask = (col >= 0) ? (tetro_row << col) : (tetro_row >> -col);
  if ((row_mask[row] & tetro_mask) == 0) {
    row_mask[row] |= tetro_mask;
    return true;
  } else {
    return false;
  }
}

bool place_tetro(int8_t row, int8_t col, uint16_t tetro) {
  if (!place_tetro_row(row + 0, col, (tetro >> 12) & 0x0F)) return false;
  if (!place_tetro_row(row + 1, col, (tetro >> 8) & 0x0F)) return false;
  if (!place_tetro_row(row + 2, col, (tetro >> 4) & 0x0F)) return false;
  return place_tetro_row(row + 3, col, (tetro >> 0) & 0x0F);
}

// TODO rename/put these in object/namespace; collision with row/col parameters above
Shape* shape_ptr;
int8_t row;
int8_t col;
uint8_t rot;
uint16_t cur_tetro;

void next_shape() {
  // TODO random shuffle deck of shapes or w/e rando algo
  static uint8_t num = 0;
  if (num >= 7) num = 0;

  shape_ptr = shapes[num++];
  row = -2;
  col = (NUM_COLS - 3) / 2;
  rot = 0;
  cur_tetro = shape_ptr->data[rot];
}

uint8_t clear_rows(int8_t row) {
  uint8_t rows_cleared = 0;
  int8_t end = min(row + 4, NUM_ROWS); // TODO magic number (shape height 4)
  row = max(row, 0);
  for (; row < end; ++row) {
    if (row_mask[row] == uint8_t(~0)) {
      ++rows_cleared;
      // Shift mask downwards
      for (int8_t i = row; i >= 0; --i) {
        uint8_t shifted_mask = (i > 0) ? row_mask[i - 1] : 0;
        row_mask[i] = shifted_mask;
        for (int8_t col = 0; col < NUM_COLS; ++col) {
          draw_pixel(i, col, shifted_mask & 1);
          shifted_mask >>= 1;
        }
      }
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
    memset(row_mask, 0, sizeof(row_mask));
    is_game_over = false;
    next_shape();
    first_call = false;
  }

  // TODO use state machine, add end screen, back to menu, etc
  if (is_game_over) return;

  // Clear shape before moving with controller
  draw_tetro(row, col, cur_tetro, false);

  PlayStation.update();

  // Move shape left/right
  uint16_t pressed = PlayStation.get_pressed();
  if ((pressed & (PlayStation.Left | PlayStation.Right)) != 0) {
    int8_t next_col = (pressed & PlayStation.Left) ? (col - 1) : (col + 1);
    if (test_tetro(row, next_col, cur_tetro) == false) {
      col = next_col;
    }
  }

  // Rotate shape
  if ((pressed & (PlayStation.L1 | PlayStation.R1)) != 0) {
    uint8_t next_rot = ((pressed & PlayStation.L1) ? (shape_ptr->count + rot - 1) : (rot + 1)) % shape_ptr->count;
    uint16_t next_tetro = shape_ptr->data[next_rot];
    if (test_tetro(row, col, next_tetro) == false) {
      rot = next_rot;
      cur_tetro = next_tetro;
    }
  }

  const bool drop_now = (pressed & PlayStation.Cross);

  // Wait for next timer tick
  if (drop_now || timer.did_tick()) {
    // Try to move down...
    int8_t next_row = row + 1;
    if (test_tetro(next_row, col, cur_tetro) == false) {
      row = next_row;
    } else {
      // ... blocked below, so place shape here
      draw_tetro(row, col, cur_tetro, true);
      if (place_tetro(row, col, cur_tetro) == false) {
        // TODO add game over state transition
        is_game_over = true;
      }
      // Did placing the shape clear any rows?
      if (clear_rows(row) > 0) {
        // TODO scroll/redraw rows above
        // TODO increment score based on num rows cleared
        // TODO Uno R4 can just use float
        auto period = timer.get_period();
        period = (period >> 4) + (period >> 3) + (period >> 2) + (period >> 1);
        timer.set_period(period);
      }
      // Cycle to next shape
      next_shape();
    }
  }

  // Redraw shape after moving
  draw_tetro(row, col, cur_tetro, true);

  Frame.render();
}