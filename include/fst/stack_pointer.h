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
#include <fst/config>
#include <fst/print>
#include <cstdint>
#include <string>
#include <new>
#include <memory>
#include <unordered_map>

namespace fst {
using stack_variable_unique_id = std::size_t;

template <typename _Tp>
class stack_variable;

template <typename _Tp>
class weak_stack_variable;

class stack_variable_manager {
public:
  static void print() {
    fst::print("stack_variable_manager");
    auto& spm = get();
    for (const auto& n : spm._map) {
      fst::print(n);
    }
  }

  static std::size_t size() { return get()._map.size(); }

  static std::size_t ref_count(stack_variable_unique_id uid) {
    auto& spm = get();
    auto it = spm._map.find(uid);
    return it == spm._map.end() ? 0 : it->second;
  }

private:
  inline static stack_variable_manager& get() {
    static stack_variable_manager spm;
    return spm;
  }

  inline static bool find(stack_variable_unique_id uid) {
    auto& spm = get();
    auto it = spm._map.find(uid);
    return it != spm._map.end();
  }

  inline static void add(stack_variable_unique_id uid, std::size_t ref_count) {
    auto& spm = get();
    spm._map.emplace(uid, ref_count);
  }

  inline static void remove(stack_variable_unique_id uid) {
    auto& spm = get();
    auto it = spm._map.find(uid);

    if (it == spm._map.end()) {
      return;
    }

    if (it->second == 1) {
      spm._map.erase(it);
    }
    else {
      it->second--;
    }
  }

  inline static bool incr_ref(stack_variable_unique_id uid) {
    auto& spm = get();
    auto it = spm._map.find(uid);

    if (it == spm._map.end()) {
      return false;
    }

    it->second++;
    return true;
  }

  stack_variable_manager() = default;

  std::unordered_map<stack_variable_unique_id, std::size_t> _map;

  template <typename T>
  friend class stack_variable;

  template <typename T>
  friend class weak_stack_variable;

  inline static stack_variable_unique_id generate_unique_id() {
    static stack_variable_unique_id uid = 0;
    return ++uid;
  }
};

template <typename _Tp>
class stack_variable {
public:
  using value_type = _Tp;
  using pointer = _Tp*;
  using const_pointer = const _Tp*;
  constexpr static std::size_t value_type_alignement = alignof(value_type);

  stack_variable() = delete;
  stack_variable(stack_variable&&) = delete;

  template <class... Args>
  stack_variable(Args&&... args) {
    using t_args = std::tuple<Args...>;

    // Copy construct.
    if constexpr (std::tuple_size_v<t_args> == 1
        && (std::is_same_v<Args..., stack_variable&> || std::is_same_v<Args..., const stack_variable&>)) {
      fst::print("FSLKJJKSF");

      [this](const auto& v) { (*get()) = *v.get(); }(std::forward<Args>(args)...);
    }
    else {
      new (&_data) value_type{ std::forward<Args>(args)... };
    }

    _ref_count = 0;
    _uid = stack_variable_manager::generate_unique_id();
  }

  stack_variable& operator=(const stack_variable&) = delete;
  stack_variable& operator=(stack_variable&&) = delete;

  inline ~stack_variable() {
    if (_ref_count) {
      stack_variable_manager::add(_uid, _ref_count);
    }

    _uid = 0;
    get()->~value_type();
  }

  inline pointer operator->() { return get(); }
  inline const_pointer operator->() const { return get(); }
  //  inline stack_variable_unique_id get_id() const { return _uid; }
  inline std::size_t ref_count() const { return _ref_count; }

private:
  template <typename T>
  friend class weak_stack_variable;

  using aligned_type = typename std::aligned_storage<sizeof(value_type), value_type_alignement>::type;
  aligned_type _data;
  stack_variable_unique_id _uid = 0;
  mutable std::size_t _ref_count = 0;

  inline pointer get() { return reinterpret_cast<pointer>(&_data); }
  inline const_pointer get() const { return reinterpret_cast<const_pointer>(&_data); }
  inline void incr_ref() { _ref_count++; }
  inline void decr_ref() { _ref_count--; }
};

template <typename _Tp>
class weak_stack_variable {
public:
  using value_type = _Tp;
  using pointer = _Tp*;
  using const_pointer = const _Tp*;

  weak_stack_variable() = default;

  weak_stack_variable(const weak_stack_variable& w) {
    if (w.get()) {
      _sp = w._sp;
      _ptr = w._ptr;
      _uid = w._uid;
      _sp->incr_ref();
    }
    else if (stack_variable_manager::incr_ref(w._uid)) {
      _sp = nullptr;
      _ptr = nullptr;
      _uid = w._uid;
    }
    else {
      _sp = nullptr;
      _ptr = nullptr;
      _uid = 0;
    }
  }

  weak_stack_variable(weak_stack_variable&& w)
      : _sp(w._sp)
      , _ptr(w._ptr)
      , _uid(w._uid) {
    w._sp = nullptr;
    w._ptr = nullptr;
    w._uid = 0;
  }

  weak_stack_variable(stack_variable<value_type>& sp)
      : _sp(&sp)
      , _ptr(sp.get())
      , _uid(sp._uid) {

    _sp->incr_ref();
  }

  inline ~weak_stack_variable() {
    if (get()) {
      _sp->decr_ref();
      return;
    }

    stack_variable_manager::remove(_uid);
  }

  weak_stack_variable& operator=(const weak_stack_variable& w) {
    close();

    if (w.get()) {
      _sp = w._sp;
      _ptr = w._ptr;
      _uid = w._uid;
      _sp->incr_ref();
    }
    else if (stack_variable_manager::incr_ref(w._uid)) {
      _sp = nullptr;
      _ptr = nullptr;
      _uid = w._uid;
    }
    else {
      _sp = nullptr;
      _ptr = nullptr;
      _uid = 0;
    }
    return *this;
  }

  weak_stack_variable& operator=(weak_stack_variable&& w) {
    close();
    _sp = w._sp;
    _ptr = w._ptr;
    _uid = w._uid;
    w._sp = nullptr;
    w._ptr = nullptr;
    w._uid = 0;
    return *this;
  }

  weak_stack_variable& operator=(stack_variable<value_type>& sp) {
    close();
    _sp = &sp;
    _ptr = sp.get();
    _uid = sp._uid;
    _sp->incr_ref();
    return *this;
  }

  inline pointer operator->() { return get(); }
  inline const_pointer operator->() const { return get(); }

  inline pointer get() { return (_uid == 0 || stack_variable_manager::find(_uid)) ? nullptr : _ptr; }

  inline const_pointer get() const { return (_uid == 0 || stack_variable_manager::find(_uid)) ? nullptr : _ptr; }

  //  inline stack_variable_unique_id get_id() const { return _uid; }

private:
  stack_variable<value_type>* _sp = nullptr;
  pointer _ptr = nullptr;
  stack_variable_unique_id _uid = 0;

  inline void close() {
    if (_uid == 0) {
      return;
    }

    if (!stack_variable_manager::find(_uid)) {
      _sp->decr_ref();
      return;
    }

    stack_variable_manager::remove(_uid);
  }
};

} // namespace fst.
