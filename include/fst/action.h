#pragma once
#include "fst/inplace_function.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <future>
#include <vector>

namespace fst {
/// Encapsulate an action without allocations.
class action {
public:
  static constexpr std::size_t inplace_function_default_capacity = 32;
  using fct_type = fst::inplace_function<void(), inplace_function_default_capacity>;

  action() = default;
  action(const action&) = default;
  action(action&&) = default;
  inline action(fct_type fct) { _action = fct; }

  /// The lamda function passed as argument needs to be fairly small
  /// to prevent allocation (see inplace_function_default_capacity).
  /// Refer to fst/inplace_function.h for more info.
  template <typename _Fct>
  inline action(_Fct fct) {
    _action = fct_type(fct);
  }

  ~action() = default;
  action& operator=(const action&) = default;
  action& operator=(action&&) = default;

  inline void call() {
    if (_action) {
      _action();
    }
  }

private:
  fct_type _action;
};

template <typename _ResultType>
class async_action {
public:
  using result_type = _ResultType;

  async_action() = delete;
  async_action(const async_action&) = delete;
  async_action(async_action&&) = delete;

  template <typename _Fct>
  inline async_action(_Fct fct) {
    _result_future = _result_promise.get_future();
    _action = [this, fct]() { _result_promise.set_value(fct()); };
  }

  ~async_action() = default;
  async_action& operator=(const async_action&) = delete;
  async_action& operator=(async_action&&) = delete;

  inline operator action &&() { return std::move(_action); }

  /// Waits until the future has a valid result and retrieves it.
  /// It effectively calls wait() in order to wait for the result.
  /// @warning The behavior is undefined if is_valid() returns false before the call to this function.
  inline result_type get() { return _result_future.get(); }

  inline bool is_valid() const noexcept { return _result_future.valid(); }

  inline void wait() { _result_future.wait(); }

  /// Waits for the result to become available.
  /// Blocks until specified timeout_duration has elapsed or the result becomes available, whichever comes first.
  /// Returns value identifies the state of the result.
  /// @warning The behavior is undefined if is_valid() returns false before the call to this function.
  template <class Rep, class Period>
  std::future_status wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const {
    return _result_future.wait_for(timeout_duration);
  }

  /// Waits for a result to become available.
  /// Blocks until specified timeout_time has been reached or the result becomes available, whichever comes first.
  /// The return value indicates why wait_until returned.
  /// @warning The behavior is undefined if is_valid() returns false before the call to this function.
  template <class Clock, class Duration>
  std::future_status wait_until(const std::chrono::time_point<Clock, Duration>& timeout_time) const {
    return _result_future.wait_until(timeout_time);
  }

private:
  std::promise<result_type> _result_promise;
  std::future<result_type> _result_future;
  action _action;
};

/// Thread aware container to add and execute actions.
/// Only class excutor as template argument will be able to call the execute, clear and reset functions.
template <typename _Executor, typename _Mutex = std::mutex, template <class...> class _VectorType = std::vector>
class action_manager {
public:
  using executor = _Executor;
  using mutex_type = _Mutex;
  using vector_type = _VectorType<action>;

  action_manager() = default;
  action_manager(const action_manager&) = delete;
  action_manager(action_manager&&) = delete;

  inline ~action_manager() {
    _action_buffer[0].clear();
    _action_buffer[1].clear();
  }

  action_manager& operator=(const action_manager&) = delete;
  action_manager& operator=(action_manager&&) = delete;

  inline std::size_t size() const {
    std::lock_guard<mutex_type> lock(_mutex);
    return _actions_to_add->size();
  }

  inline bool empty() const {
    std::lock_guard<mutex_type> lock(_mutex);
    return _actions_to_add->empty();
  }

  /// Any call to add inside the execute loop will call the action right away
  /// unless 'call_inplace_if_possible' is false.
  /// Returns true if action was executed immediately, false if not.
  inline bool add(fst::action&& act, bool call_inplace_if_possible = true) {
    if (call_inplace_if_possible && std::this_thread::get_id() == _execute_thread_id) {
      act.call();
      return true;
    }

    _mutex.lock();
    _actions_to_add->push_back(std::move(act));
    _mutex.unlock();
    return false;
  }

  /// This will add the action to the queue and wait for the action to be executed before returning.
  template <typename _Fct>
  inline auto add_and_wait(_Fct fct) {
    fst::async_action<decltype(fct())> act(fct);
    add(act);
    return act.get();
  }

  /// This will add the action to the queue and wait for the action to be executed before returning.
  template <typename _ReturnType, typename _Fct>
  inline _ReturnType add_and_wait(_Fct fct) {
    fst::async_action<_ReturnType> act(fct);
    add(act);
    return act.get();
  }

private:
  vector_type _action_buffer[2];
  vector_type* _actions_to_add = &_action_buffer[0];
  vector_type* _actions_to_execute = &_action_buffer[1];
  mutable mutex_type _mutex;
  std::atomic<std::thread::id> _execute_thread_id;

  friend executor;

  /// Execute all actions.
  inline void execute() {
    _mutex.lock();
    _execute_thread_id = std::this_thread::get_id();
    std::swap(_actions_to_add, _actions_to_execute);
    _actions_to_add->clear();
    _mutex.unlock();

    for (auto& act : *_actions_to_execute) {
      act.call();
    }

    _execute_thread_id = std::thread::id();
  }

  /// Erase all actions.
  inline void clear() {
    _mutex.lock();
    _action_buffer[0].clear();
    _action_buffer[1].clear();
    _mutex.unlock();
  }

  /// Erase all actions and deallocates heap memory.
  inline void reset() {
    _mutex.lock();
    _action_buffer[0] = vector_type();
    _action_buffer[1] = vector_type();
    _mutex.unlock();
  }
};

} // namespace fst.
