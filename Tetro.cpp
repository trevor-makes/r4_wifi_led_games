#include "Tetro.h"
#include "PlayStation.h"
#include "Frame.h"
#include "Menu.h"

#include <stdint.h>

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

Shape* shapes[] = {&shape_I, &shape_Z, &shape_S, &shape_O, &shape_T, &shape_J, &shape_L};
constexpr uint8_t NUM_SHAPES = sizeof(shapes) / sizeof(shapes[0]);

class TetroField {
public:
  static constexpr uint8_t NUM_ROWS = Frame.WIDTH;
  static constexpr uint8_t NUM_COLS = Frame.HEIGHT;
private:
  uint8_t row_mask_[NUM_ROWS];
  static_assert(sizeof(row_mask_[0]) * 8 >= NUM_COLS, "bitfield size must exceed num columns");

public:
  static bool is_above(int8_t row) { return row < 0; }
  static bool is_below(int8_t row) { return row >= NUM_ROWS; }
  static bool is_above_or_below(int8_t row) { return is_above(row) || is_below(row); }

  static bool is_left_of(int8_t col, uint8_t mask) {
    return col < 0 && (mask & ((1 << -col) - 1)) != 0;
  }

  static bool is_right_of(int8_t col, uint8_t mask) {
    return mask >> (NUM_COLS - col) != 0;
  }

  void clear() {
    memset(row_mask_, 0, sizeof(row_mask_));
  }

  void plot(int8_t row, int8_t col, bool set) {
    if (row < 0 || row >= NUM_ROWS || col < 0 || col >= NUM_COLS) return;
    Frame.plot(NUM_COLS - 1 - col, row, set); // apply rotation transform
  }

  bool is_overlapping(int8_t row, uint8_t mask) const {
    if (is_above_or_below(row)) return false;
    return (row_mask_[row] & mask) != 0;
  }

  bool is_filled(int8_t row) const {
    if (is_above_or_below(row)) return false;
    return row_mask_[row] == uint8_t(~0);
  }

  bool is_empty(int8_t row) const {
    if (is_above_or_below(row)) return true;
    return row_mask_[row] == 0;
  }

  bool try_place(int8_t row, uint8_t mask) {
    if (is_above_or_below(row)) return false;
    if (is_overlapping(row, mask)) return false;
    row_mask_[row] |= mask;
    return true;
  }

  void drop_row(int8_t row) {
    for (int8_t i = row; i >= 0; --i) {
      uint8_t shifted_mask = (i > 0) ? row_mask_[i - 1] : 0;
      row_mask_[i] = shifted_mask;
      for (int8_t col = 0; col < NUM_COLS; ++col) {
        plot(i, col, shifted_mask & 1);
        shifted_mask >>= 1;
      }
    }
  }

  uint8_t try_drop() {
    uint8_t rows_cleared = 0;
    for (int8_t row = 0; row < NUM_ROWS; ++row) {
      if (is_filled(row)) {
        ++rows_cleared;
        drop_row(row);
      }
    }
    return rows_cleared;
  }
};

class Tetro {
  TetroField& field_;
  Shape* shape_ = nullptr;
  int8_t row_;
  int8_t col_;
  uint8_t rot_;

public:
  Tetro(TetroField& field): field_{field} {}

  void set_shape(Shape* shape) { shape_ = shape; }
  void set_row(int8_t row) { row_ = row; }
  void set_col(int8_t col) { col_ = col; }
  void set_rot(uint8_t rot) { rot_ = rot; }

  uint8_t get_row() const { return row_; }
  uint8_t get_col() const { return col_; }
  uint8_t get_rot() const { return rot_; }

private:
  void draw_row(int8_t row, int8_t col, uint8_t tetro_row, bool set) {
    while (tetro_row > 0) {
      if ((tetro_row & 1) == 1) {
        field_.plot(row, col, set);
      }
      ++col;
      tetro_row >>= 1;
    }
  }

public:
  void draw(bool set) {
    uint16_t tetro = shape_->data[rot_];
    draw_row(row_ + 0, col_, (tetro >> 12) & 0x0F, set);
    draw_row(row_ + 1, col_, (tetro >> 8) & 0x0F, set);
    draw_row(row_ + 2, col_, (tetro >> 4) & 0x0F, set);
    draw_row(row_ + 3, col_, (tetro >> 0) & 0x0F, set);
  }

private:
  bool is_valid_move_row(int8_t row, int8_t col, uint8_t tetro_row) const {
    if (tetro_row == 0) return true; // skip rows without pixels
    if (field_.is_below(row)) return false; // block offscreen pixels below
    if (field_.is_left_of(col, tetro_row)) return false; // test left boundary
    if (field_.is_right_of(col, tetro_row)) return false; // test right boundary
    if (field_.is_above(row)) return true; // allow shapes falling from offscreen above
    uint8_t tetro_mask = (col >= 0) ? (tetro_row << col) : (tetro_row >> -col);
    return !field_.is_overlapping(row, tetro_mask);
  }

  bool is_valid_move(int8_t row, int8_t col, uint8_t rot) const {
    uint16_t tetro = shape_->data[rot];
    if (!is_valid_move_row(row + 0, col, (tetro >> 12) & 0x0F)) return false;
    if (!is_valid_move_row(row + 1, col, (tetro >> 8) & 0x0F)) return false;
    if (!is_valid_move_row(row + 2, col, (tetro >> 4) & 0x0F)) return false;
    return is_valid_move_row(row + 3, col, (tetro >> 0) & 0x0F);
  }

  bool try_place_row(int8_t row, int8_t col, uint8_t tetro_row) {
    if (tetro_row == 0) return true;
    if (field_.is_above_or_below(row)) return false; // can't place offscreen
    if (field_.is_left_of(col, tetro_row)) return false; // test left boundary
    if (field_.is_right_of(col, tetro_row)) return false; // test right boundary
    uint8_t tetro_mask = (col >= 0) ? (tetro_row << col) : (tetro_row >> -col);
    return field_.try_place(row, tetro_mask);
  }

public:
  bool try_place() {
    uint16_t tetro = shape_->data[rot_];
    if (!try_place_row(row_ + 0, col_, (tetro >> 12) & 0x0F)) return false;
    if (!try_place_row(row_ + 1, col_, (tetro >> 8) & 0x0F)) return false;
    if (!try_place_row(row_ + 2, col_, (tetro >> 4) & 0x0F)) return false;
    return try_place_row(row_ + 3, col_, (tetro >> 0) & 0x0F);
  }

  bool try_move(int8_t drow, int8_t dcol) {
    int8_t next_row = row_ + drow;
    int8_t next_col = col_ + dcol;
    if (is_valid_move(next_row, next_col, rot_)) {
      row_ = next_row;
      col_ = next_col;
      return true;
    }
    return false;
  }

  bool try_rotate(int8_t drot) {
    uint8_t next_rot = (shape_->count + rot_ + drot) % shape_->count;
    if (is_valid_move(row_, col_, next_rot)) {
      rot_ = next_rot;
      return true;
    }
    return false;
  }
};

static TetroField field;
static Tetro tetro(field);
static uint16_t score;
static unsigned long period;
static Timer timer;

constexpr unsigned long INIT_PERIOD = 1000;
constexpr unsigned long MIN_PERIOD = 75;

void tetro_menu_setup(StateMachine&);
void tetro_menu_loop(StateMachine&);

const State tetro_menu_state = {
  .setup = tetro_menu_setup,
  .loop = tetro_menu_loop,
};

void tetro_game_setup(StateMachine&);
void tetro_game_loop(StateMachine&);

const State tetro_game_state = {
  .setup = tetro_game_setup,
  .loop = tetro_game_loop,
};

void tetro_fail_setup(StateMachine&);
void tetro_fail_loop(StateMachine&);

const State tetro_fail_state = {
  .setup = tetro_fail_setup,
  .loop = tetro_fail_loop,
};

void tetro_score_loop(StateMachine&);

const State tetro_score_state = {
  .setup = nullptr,
  .loop = tetro_score_loop,
};

void tetro_menu_setup(StateMachine& state) {
  field.clear();
  tetro.set_shape(&shape_L);
  tetro.set_rot(0);
  timer.set_period(300);
  timer.reset();
}

void tetro_menu_loop(StateMachine& state) {
  if (menu_selection<tetro_left_state, tetro_right_state, tetro_game_state>(state)) return;

  if (timer.did_tick() == false) return;

  Frame.clear();
  switch (tetro.get_rot()) {
    case 0:
      tetro.set_row(4+1);
      tetro.set_col(2-1);
      break;
    case 1:
      tetro.set_row(4-1);
      tetro.set_col(2+0);
      break;
    case 2:
      tetro.set_row(4+0);
      tetro.set_col(2+2);
      break;
    case 3:
      tetro.set_row(4+2);
      tetro.set_col(2+1);
      break;
  }
  tetro.draw(true);
  tetro.try_rotate(1);
  Frame.render();
}

void tetro_score_loop(StateMachine& state) {
  PlayStation.update();
  const auto pressed = PlayStation.get_pressed();
  if (pressed & PlayStation.Select) {
    state.exit(); // go back to the parent menu
    return;
  } else if (pressed & PlayStation.Start) {
    state.next(tetro_game_state);
    return;
  }

  if (timer.did_tick() == false) return;

  Frame.clear();
  Frame.plot_digit(0, 0, (score / 100) % 10, true);
  Frame.plot_digit(0, 4, (score / 10) % 10, true);
  Frame.plot_digit(0, 8, (score / 1) % 10, true);
  Frame.render();
}

void tetro_fail_loop(StateMachine& state) {
  // Exit animation early
  PlayStation.update();
  const auto pressed = PlayStation.get_pressed();
  if (pressed & PlayStation.Select) {
    state.next(tetro_score_state);
    return;
  }

  if (timer.did_tick() == false) return;

  field.drop_row(field.NUM_ROWS - 1);
  Frame.render();

  if (field.is_empty(field.NUM_ROWS - 1)) {
    state.next(tetro_score_state);
  }
}

void tetro_fail_setup(StateMachine& state) {
  timer.set_period(150);
}

void next_shape() {
  // TODO shuffle a deck or ?? to make pieces easier to work with
  uint8_t num = random(NUM_SHAPES);
  tetro.set_shape(shapes[num]);
  tetro.set_row(-2);
  tetro.set_col((field.NUM_COLS - 3) / 2);
  tetro.set_rot(0);
}

void tetro_game_loop(StateMachine& state) {
  // Clear shape before moving with controller
  tetro.draw(false);

  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();

  // Exit game early
  if (pressed & PlayStation.Select) {
    state.next(tetro_fail_state);
    return;
  }

  // Move/rotate shape
  if ((pressed & (PlayStation.Left | PlayStation.Right)) != 0) {
    tetro.try_move(0, (pressed & PlayStation.Left) ? -1 : 1);
  }
  if ((pressed & (PlayStation.L1 | PlayStation.R1)) != 0) {
    tetro.try_rotate((pressed & PlayStation.L1) ? -1 : 1);
  }

  const bool drop_now = (pressed & PlayStation.Cross);
  timer.set_period((PlayStation.get_held() & PlayStation.Cross) ? MIN_PERIOD : period);

  // Wait for next timer tick
  if (drop_now || timer.did_tick()) {
    // Try to move down...
    if (tetro.try_move(1, 0) == false) {
      // ...but blocked below, so try to place shape here
      tetro.draw(true);
      if (tetro.try_place() == false) {
        state.next(tetro_fail_state);
        return;
      }
      // Did placing the shape clear any rows?
      const uint8_t cleared = field.try_drop();
      if (cleared > 0) {
        score += cleared * 2 - 1;
        // TODO Uno R4 can just use float multiplication
        if (period > MIN_PERIOD) {
          period = (period >> 4) + (period >> 3) + (period >> 2) + (period >> 1);
          period = max(period, MIN_PERIOD);
        }
      }
      // Cycle to next shape
      next_shape();
    }
  }

  // Redraw shape after moving
  tetro.draw(true);

  Frame.render();
}

void tetro_game_setup(StateMachine& state) {
  Frame.clear();
  field.clear();
  period = INIT_PERIOD;
  score = 0;
  randomSeed(micros());
  next_shape();
}