#pragma once

#include <Arduino.h>

#include <vector>

class Timer {
  unsigned long period_;
  unsigned long prev_;

public:
  void reset() { prev_ = millis(); }

  bool did_tick() {
    // Wait for next move
    unsigned long now = millis();
    if ((now - prev_) < period_) return false;
    prev_ = now;
    return true;
  }

  unsigned long get_period() const { return period_; }
  void set_period(unsigned long period) { period_ = period; }
};

class StateMachine;

struct State {
  using Fn = void(StateMachine&, Timer&);
  Fn *setup;
  Fn *loop;
};

class StateMachine {
private:
  Timer timer_;
  std::vector<State> states_;
  bool needs_setup_ = true;

public:
  void enter(const State& state) {
    states_.push_back(state);
    needs_setup_ = true;
  }

  void back() {
    if (!states_.empty()) {
      states_.pop_back();
    }
    needs_setup_ = true;
  }

  void next(const State& state) {
    back(); // lose the current state
    enter(state);
  }

  void update() {
    if (!states_.empty()) {
      auto& state = states_.back();
      // NOTE setup will never be called if update/loop is never called
      if (needs_setup_) {
        if (state.setup) state.setup(*this, timer_);
        needs_setup_ = false;
      }
      if (state.loop) state.loop(*this, timer_);
    }
  }
};