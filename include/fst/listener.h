///
/// BSD 3-Clause License
///
/// Copyright (c) 2021, Alexandre Arsenault
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// * Redistributions of source code must retain the above copyright notice, this
///   list of conditions and the following disclaimer.
///
/// * Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
/// * Neither the name of the copyright holder nor the names of its
///   contributors may be used to endorse or promote products derived from
///   this software without specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
/// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
/// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
/// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
/// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
/// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
/// POSSIBILITY OF SUCH DAMAGE.
///

#pragma once
#include <fst/pointer>
#include <mutex>
#include <vector>

namespace fst {
template <typename _Listener, template <class...> class _VectorType = std::vector>
class listener_manager {
public:
  using listener = _Listener;
  using vector_type = _VectorType<listener*>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;
  using size_type = typename vector_type::size_type;

  inline void add(fst::not_null<listener*> new_listener) {
    for (listener* l : _listeners) {
      if (l == new_listener) {
        return;
      }
    }

    _listeners.push_back(new_listener);
  }

  inline void remove(fst::not_null<listener*> old_listener) {
    for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
      if ((*it) == old_listener) {
        _listeners.erase(it);
        return;
      }
    }
  }

  inline vector_type& get() { return _listeners; }
  inline const vector_type& get() const { return _listeners; }

  inline size_type size() const noexcept { return _listeners.size(); }
  inline bool empty() const noexcept { return _listeners.empty(); }

  inline iterator begin() { return _listeners.begin(); }
  inline const_iterator begin() const { return _listeners.begin(); }
  inline iterator end() { return _listeners.end(); }
  inline const_iterator end() const { return _listeners.end(); }

private:
  vector_type _listeners;
};

namespace mt {
  template <typename _Listener, typename _Mutex = std::recursive_mutex,
      template <class...> class _VectorType = std::vector>
  class listener_manager {
  public:
    using listener = _Listener;
    using vector_type = _VectorType<listener*>;
    using iterator = typename vector_type::iterator;
    using const_iterator = typename vector_type::const_iterator;
    using size_type = typename vector_type::size_type;
    using mutex_type = _Mutex;
    using lock_guard_type = std::lock_guard<mutex_type>;

    inline void add(fst::not_null<listener*> nl) {
      lock_guard_type lk(_mutex);

      for (listener* l : _listeners) {
        if (l == nl) {
          return;
        }
      }

      _listeners.push_back(nl);
    }

    inline void remove(fst::not_null<listener*> ol) {
      lock_guard_type lk(_mutex);

      for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
        if ((*it) == ol) {
          _listeners.erase(it);
          return;
        }
      }
    }

    inline vector_type& get() { return _listeners; }
    inline const vector_type& get() const { return _listeners; }

    inline size_type size() const noexcept { return _listeners.size(); }
    inline bool empty() const noexcept { return _listeners.empty(); }
    inline void lock() { _mutex.lock(); }
    inline void unlock() { _mutex.unlock(); }

    template <auto Fct, typename... Args>
    inline void notify(Args&&... args) {
      if (_listeners.empty()) {
        return;
      }

      lock();
      for (auto& l : _listeners) {
        (l->*Fct)(std::forward<Args>(args)...);
      }
      unlock();
    }

    inline void clear() {
      lock();
      _listeners.clear();
      unlock();
    }

    inline void reset() {
      lock();
      vector_type().swap(_listeners);
      unlock();
    }

  private:
    vector_type _listeners;
    mutex_type _mutex;
  };
} // namespace mt.
} // namespace fst.
