#pragma once
#include <atomic>
#include <chrono>
#include <thread>

namespace fst {
template <typename _Object, auto _Callback>
class steady_timer {
public:
  using object_type = _Object;

  steady_timer() = delete;
  steady_timer(const steady_timer&) = delete;
  steady_timer(steady_timer&&) = delete;

  inline steady_timer(object_type& obj)
      : _object(obj) {}

  inline ~steady_timer() { stop(); }

  steady_timer& operator=(const steady_timer&) = delete;
  steady_timer& operator=(steady_timer&&) = delete;

  template <class _Rep, class _Period>
  inline void start(const std::chrono::duration<_Rep, _Period>& delta_time) {
    stop();

    _delta_ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta_time);
    _stopper.start();

    // Start timer thread.
    _thread = std::thread(&steady_timer::timer_thread, std::ref(*this));
  }

  inline void stop() {
    _stopper.stop();

    if (_thread.joinable()) {
      _thread.join();
    }
  }

  inline void suspend() { _is_suspended = true; }
  inline void resume() { _is_suspended = false; }
  inline bool is_suspended() const { return _is_suspended; }

  inline std::chrono::seconds get_delta_time() const {
    return std::chrono::duration_cast<std::chrono::seconds>(_delta_ms);
  }
  inline std::chrono::milliseconds get_delta_ms() const { return _delta_ms; }

private:
  class timer_stopper {
  public:
    inline timer_stopper() noexcept { _stopper.test_and_set(); }
    inline ~timer_stopper() { stop(); }

    timer_stopper(const timer_stopper&) = delete;
    timer_stopper(timer_stopper&&) = delete;
    timer_stopper& operator=(const timer_stopper&) = delete;
    timer_stopper& operator=(timer_stopper&&) = delete;

    inline explicit operator bool() {
      // Atomically changes the state of a std::atomic_flag to set(true)
      // and returns the value it held before.
      return _stopper.test_and_set();
    }

    inline void start() { _stopper.test_and_set(); }
    inline void stop() { _stopper.clear(); }

  private:
    std::atomic_flag _stopper = ATOMIC_FLAG_INIT;
  };

  object_type& _object;
  timer_stopper _stopper;
  std::atomic<bool> _is_suspended = false;
  std::thread _thread;
  std::chrono::milliseconds _delta_ms;

  inline static int timer_thread(steady_timer& tm) {
    while (tm._stopper) {
      if (!tm._is_suspended) {
        (tm._object.*_Callback)();
      }
      std::this_thread::sleep_for(tm._delta_ms);
    }

    return 0;
  }
};
} // namespace fst.
