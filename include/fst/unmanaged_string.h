///
/// BSD 3-Clause License
///
/// Copyright (c) 2020, Alexandre Arsenault
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
#include <fst/ascii>
#include <fst/assert>
#include <fst/traits>
#include <fst/span>
#include <fst/util>
#include <fst/verified_value>
#include <algorithm>
#include <array>
#include <cstring>
#include <cstdio>
#include <iterator>
#include <iostream>
#include <initializer_list>
#include <limits>
#include <string>
#include <string_view>
#include <sstream>
#include <stdexcept>

namespace fst {
template <typename _CharT>
class basic_unmanaged_string {
public:
  using __self = basic_unmanaged_string;
  using buffer_type = fst::span<_CharT>;
  using view_type = std::basic_string_view<_CharT>;
  using string_type = std::basic_string<_CharT>;
  using traits_type = std::char_traits<_CharT>;
  using value_type = _CharT;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  //  static constexpr size_type maximum_size = _Size;
  static constexpr size_type npos = std::numeric_limits<size_type>::max();

  static_assert(!std::is_array<value_type>::value, "Character type of basic_unmanaged_string must not be an array.");
  static_assert(
      std::is_standard_layout<value_type>::value, "Character type of basic_unmanaged_string must be standard-layout.");
  static_assert(std::is_trivial<value_type>::value, "Character type of basic_unmanaged_string must be trivial.");
  static_assert(std::is_same<value_type, typename traits_type::char_type>::value,
      "traits_type::char_type must be the same type as value_type.");

  constexpr basic_unmanaged_string() noexcept = default;

  inline constexpr basic_unmanaged_string(buffer_type buffer) noexcept {
    _buffer = buffer;
    _size = 0;
    _buffer[_size] = 0;
  }

  inline constexpr basic_unmanaged_string(buffer_type buffer, std::size_t size) noexcept {
    _buffer = buffer;
    _size = size;
    _buffer[_size] = 0;
  }

  inline constexpr basic_unmanaged_string(buffer_type buffer, size_type count, value_type ch) noexcept {
    _buffer = buffer;
    fst_assert(count <= max_size(), "basic_unmanaged_string count must be smaller or equal to maximum_size.");
    std::fill_n(_buffer.data(), count, ch);
    _size = count;
    _buffer[_size] = 0;
  }

  basic_unmanaged_string(const basic_unmanaged_string&) = delete;

  inline constexpr basic_unmanaged_string(basic_unmanaged_string&& other) noexcept
      : _buffer(std::move(other._buffer))
      , _size(other._size) {

    other._size = 0;
  }

  basic_unmanaged_string& operator=(const basic_unmanaged_string&) = delete;

  inline constexpr basic_unmanaged_string& operator=(basic_unmanaged_string&& other) noexcept {
    _buffer = std::move(other._buffer);
    _size = other._size;
    other._size = 0;
    return *this;
  }

  // Iterators.
  inline constexpr iterator begin() noexcept { return iterator(_buffer.data()); }
  inline constexpr const_iterator begin() const noexcept { return const_iterator(_buffer.data()); }

  inline constexpr iterator end() noexcept { return iterator(_buffer.data() + _size); }
  inline constexpr const_iterator end() const noexcept { return const_iterator(_buffer.data() + _size); }

  inline constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  inline constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }

  inline constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  inline constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

  inline constexpr const_iterator cbegin() const noexcept { return begin(); }
  inline constexpr const_iterator cend() const noexcept { return end(); }

  inline constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  inline constexpr const_reverse_iterator crend() const noexcept { return rend(); }

  // Capacity.
  FST_NODISCARD inline constexpr size_type size() const noexcept { return _size; }
  FST_NODISCARD inline constexpr size_type length() const noexcept { return _size; }
  FST_NODISCARD inline constexpr size_type max_size() const noexcept { return _buffer.size(); }
  FST_NODISCARD inline constexpr size_type capacity() const noexcept { return _buffer.size(); }
  FST_NODISCARD inline constexpr bool empty() const noexcept { return _size == 0; }

  // Element access.
  inline constexpr reference at(size_type pos) {
    if (pos >= size()) {
      throw std::out_of_range("basic_unmanaged_string::at");
    }

    return _buffer[pos];
  }
  inline constexpr const_reference at(size_type pos) const {
    if (pos >= size()) {
      throw std::out_of_range("basic_unmanaged_string::at");
    }

    return _buffer[pos];
  }

  inline constexpr reference operator[](size_type n) noexcept {
    fst_assert(n < max_size(), "Index out of bounds");
    return _buffer[n];
  }
  inline constexpr const_reference operator[](size_type n) const noexcept {
    fst_assert(n < max_size(), "Index out of bounds");
    return _buffer[n];
  }

  inline constexpr reference front() noexcept {
    fst_assert(_size > 0, "basic_unmanaged_string::front when empty.");
    return _buffer[0];
  }
  inline constexpr const_reference front() const noexcept {
    fst_assert(_size > 0, "basic_unmanaged_string::front when empty.");
    return _buffer[0];
  }

  inline constexpr reference back() noexcept {
    fst_assert(_size > 0, "basic_unmanaged_string::back when empty.");
    return _buffer[_size - 1];
  }

  inline constexpr const_reference back() const noexcept {
    fst_assert(_size > 0, "basic_unmanaged_string::back when empty.");
    return _buffer[_size - 1];
  }

  inline constexpr pointer data() noexcept { return _buffer.data(); }
  inline constexpr const_pointer data() const noexcept { return _buffer.data(); }

  //
  // Operations.
  //

  inline constexpr void clear() noexcept {
    _buffer[0] = 0;
    _size = 0;
  }

  inline constexpr void push_back(value_type c) noexcept {
    fst_assert(
        _size + 1 <= max_size(0), "basic_unmanaged_string::push_back size would end up greather than maximum_size.");
    _buffer[_size++] = c;
    _buffer[_size] = 0;
  }

  inline constexpr void pop_back() noexcept {
    fst_assert(_size, "basic_unmanaged_string::pop_back when empty.");
    _size--;
    _buffer[_size] = 0;
  }

  //
  // Append.
  //
  inline constexpr bool is_appendable(size_type count) const noexcept { return _size + count <= max_size(); }

  inline constexpr bool is_appendable(view_type v) const noexcept { return _size + v.size() <= max_size(); }

  inline constexpr bool is_appendable(view_type v, size_type pos, size_type count = npos) const noexcept {
    return (pos <= v.size()) && (_size + fst::minimum(count, v.size() - pos) <= max_size());
  }

  inline constexpr basic_unmanaged_string& append(value_type c) noexcept {
    fst_assert(
        _size + 1 <= max_size(), "basic_unmanaged_string::push_back size would end up greather than maximum_size.");
    _buffer[_size++] = c;
    _buffer[_size] = 0;
    return *this;
  }

  inline constexpr basic_unmanaged_string& append(size_type count, value_type c) noexcept {
    fst_assert(
        _size + count <= max_size(), "basic_unmanaged_string::append size would end up greather than maximum_size.");
    std::fill_n(_buffer.data() + _size, count, c);
    _size += count;
    _buffer[_size] = 0;
    return *this;
  }

  inline constexpr basic_unmanaged_string& append(const basic_unmanaged_string& other) noexcept {
    fst_assert(_size + other.size() <= max_size(),
        "basic_unmanaged_string::append size would end up greather than maximum_size.");
    std::copy_n(other.data(), other.size(), _buffer.data() + _size);
    _size += other.size();
    _buffer[_size] = 0;
    return *this;
  }

  inline constexpr basic_unmanaged_string& append(
      const basic_unmanaged_string& other, size_type pos, size_type count = npos) noexcept {
    fst_assert(pos <= other.size(), "basic_unmanaged_string pos must be smaller or equal to string size.");
    size_type o_size = fst::minimum(count, other.size() - pos);
    fst_assert(
        _size + o_size <= max_size(), "basic_unmanaged_string::append size would end up greather than maximum_size.");

    std::copy_n(other.data() + pos, o_size, _buffer.data() + _size);
    _size += o_size;
    _buffer[_size] = 0;
    return *this;
  }

  inline constexpr basic_unmanaged_string& append(view_type v) noexcept {
    fst_assert(
        _size + v.size() <= max_size(), "basic_unmanaged_string::append size would end up greather than maximum_size.");

    std::copy_n(v.data(), v.size(), _buffer.data() + _size);
    _size += v.size();
    _buffer[_size] = 0;
    return *this;
  }

  inline constexpr basic_unmanaged_string& append(view_type v, size_type pos, size_type count = npos) noexcept {
    fst_assert(pos <= v.size(), "basic_unmanaged_string pos must be smaller or equal to view size.");
    size_type o_size = fst::minimum(count, v.size() - pos);
    fst_assert(
        _size + o_size <= max_size(), "basic_unmanaged_string::append size would end up greather than maximum_size.");

    std::copy_n(v.data() + pos, o_size, _buffer.data() + _size);
    _size += o_size;
    _buffer[_size] = 0;
    return *this;
  }

  inline constexpr basic_unmanaged_string& append(const value_type* s) noexcept { return append(view_type(s)); }

  inline constexpr basic_unmanaged_string& append(const value_type* s, size_type pos, size_type count = npos) noexcept {
    return append(view_type(s), pos, count);
  }

  inline constexpr basic_unmanaged_string& append(const string_type& s) noexcept { return append(view_type(s)); }

  inline constexpr basic_unmanaged_string& append(
      const string_type& s, size_type pos, size_type count = npos) noexcept {
    return append(view_type(s), pos, count);
  }

  template <class InputIt>
  constexpr basic_unmanaged_string& append(InputIt first, InputIt last) noexcept {
    return append(view_type(first, last));
  }

  constexpr basic_unmanaged_string& append(std::initializer_list<value_type> ilist) noexcept {
    return append(view_type(ilist.begin(), ilist.end()));
  }

  template <class T,
      class =
          typename std::enable_if<fst::is_convertible_to_string_view<value_type, traits_type, T>::value, void>::type>
  inline constexpr basic_unmanaged_string& append(const T& t) noexcept {
    return append(view_type(t));
  }

  template <class T,
      class =
          typename std::enable_if<fst::is_convertible_to_string_view<value_type, traits_type, T>::value, void>::type>
  inline constexpr basic_unmanaged_string& append(const T& t, size_type pos, size_type count = npos) noexcept {
    return append(view_type(t), pos, count);
  }

  inline constexpr basic_unmanaged_string& operator+=(const basic_unmanaged_string& other) noexcept {
    return append(other);
  }

  inline constexpr basic_unmanaged_string& operator+=(view_type v) noexcept { return append(v); }

  inline constexpr basic_unmanaged_string& operator+=(const value_type* s) noexcept { return append(view_type(s)); }

  inline constexpr basic_unmanaged_string& operator+=(const string_type& s) noexcept { return append(view_type(s)); }

  constexpr basic_unmanaged_string& operator+=(std::initializer_list<value_type> ilist) noexcept {
    return append(view_type(ilist.begin(), ilist.end()));
  }

  template <class T,
      class =
          typename std::enable_if<fst::is_convertible_to_string_view<value_type, traits_type, T>::value, void>::type>
  inline constexpr basic_unmanaged_string& operator+=(const T& t) noexcept {
    return append(view_type(t));
  }

  //
  // Insert.
  //
  inline constexpr basic_unmanaged_string& insert(size_type index, size_type count, value_type c) {
    fst_assert(index <= _size, "basic_unmanaged_string::insert index out of bounds.");
    fst_assert(
        count + _size <= max_size(), "basic_unmanaged_string::insert size would end up greather than maximum_size.");
    size_type delta = _size - index;
    std::memmove(
        (void*)(_buffer.data() + index + count), (const void*)(_buffer.data() + index), delta * sizeof(value_type));
    std::fill_n(_buffer.data() + index, count, c);
    _size += count;
    _buffer[_size] = 0;
    return *this;
  }

  inline constexpr basic_unmanaged_string& insert(size_type index, view_type v) {
    fst_assert(index <= _size, "basic_unmanaged_string::insert index out of bounds.");
    fst_assert(
        v.size() + _size <= max_size(), "basic_unmanaged_string::insert size would end up greather than maximum_size.");
    size_type delta = _size - index;
    std::memmove(
        (void*)(_buffer.data() + index + v.size()), (const void*)(_buffer.data() + index), delta * sizeof(value_type));
    std::copy_n(v.data(), v.size(), _buffer.data() + index);
    _size += v.size();
    _buffer[_size] = 0;
    return *this;
  }

  inline constexpr basic_unmanaged_string& insert(size_type index, view_type v, size_type count) {
    fst_assert(index <= _size, "basic_unmanaged_string::insert index out of bounds.");
    fst_assert(count <= v.size(), "basic_unmanaged_string::insert count out of bounds.");
    fst_assert(
        count + _size <= max_size(), "basic_unmanaged_string::insert size would end up greather than maximum_size.");
    size_type delta = _size - index;
    std::memmove(
        (void*)(_buffer.data() + index + count), (const void*)(_buffer.data() + index), delta * sizeof(value_type));
    std::copy_n(v.data(), count, _buffer.data() + index);
    _size += count;
    _buffer[_size] = 0;
    return *this;
  }

  inline constexpr basic_unmanaged_string& insert(size_type index, view_type v, size_type index_str, size_type count) {
    fst_assert(index <= _size, "basic_unmanaged_string::insert index out of bounds.");
    fst_assert(index_str <= v.size(), "basic_unmanaged_string::insert index_str out of bounds.");
    size_type s_size = fst::minimum(count, v.size() - index_str);
    fst_assert(
        s_size + _size <= max_size(), "basic_unmanaged_string::insert size would end up greather than maximum_size.");

    size_type delta = _size - index;
    std::memmove(
        (void*)(_buffer.data() + index + s_size), (const void*)(_buffer.data() + index), delta * sizeof(value_type));
    std::copy_n(v.data() + index_str, s_size, _buffer.data() + index);
    _size += s_size;
    _buffer[_size] = 0;
    return *this;
  }

  inline constexpr basic_unmanaged_string& insert(size_type index, const value_type* s) {
    return insert(index, view_type(s));
  }

  inline constexpr basic_unmanaged_string& insert(size_type index, const value_type* s, size_type count) {
    return insert(index, view_type(s), count);
  }

  inline constexpr basic_unmanaged_string& insert(size_type index, const basic_unmanaged_string& str) {
    fst_assert(index <= _size, "basic_unmanaged_string::insert index out of bounds.");
    fst_assert(str.size() + _size <= max_size(),
        "basic_unmanaged_string::insert size would end up greather than maximum_size.");
    size_type delta = _size - index;
    std::memmove((void*)(_buffer.data() + index + str.size()), (const void*)(_buffer.data() + index),
        delta * sizeof(value_type));
    std::copy_n(str.data(), str.size(), _buffer.data() + index);
    _size += str.size();
    _buffer[_size] = 0;
    return *this;
  }

  inline constexpr basic_unmanaged_string& insert(
      size_type index, const basic_unmanaged_string& str, size_type index_str, size_type count = npos) {
    fst_assert(index <= _size, "basic_unmanaged_string::insert index out of bounds.");
    fst_assert(index_str <= str.size(), "basic_unmanaged_string::insert index_str out of bounds.");
    size_type s_size = fst::minimum(count, str.size() - index_str);
    fst_assert(
        s_size + _size <= max_size(), "basic_unmanaged_string::insert size would end up greather than maximum_size.");

    size_type delta = _size - index;
    std::memmove(
        (void*)(_buffer.data() + index + s_size), (const void*)(_buffer.data() + index), delta * sizeof(value_type));
    std::copy_n(str.data() + index_str, s_size, _buffer.data() + index);
    _size += s_size;
    _buffer[_size] = 0;
    return *this;
  }

  //  inline constexpr iterator insert(const_iterator pos, CharT ch ) {
  //
  //  }
  // constexpr iterator insert( const_iterator pos, size_type count, CharT ch );
  // template< class InputIt >
  // constexpr iterator insert( const_iterator pos, InputIt first, InputIt last );

  // constexpr iterator insert( const_iterator pos, std::initializer_list<CharT> ilist );

  template <class T,
      class =
          typename std::enable_if<fst::is_convertible_to_string_view<value_type, traits_type, T>::value, void>::type>
  inline constexpr basic_unmanaged_string& insert(size_type index, const T& t) {
    return insert(index, view_type(t));
  }

  template <class T,
      class =
          typename std::enable_if<fst::is_convertible_to_string_view<value_type, traits_type, T>::value, void>::type>
  constexpr basic_unmanaged_string& insert(size_type index, const T& t, size_type index_str, size_type count = npos) {
    return insert(index, view_type(t), index_str, count);
  }

  //
  //
  //
  inline constexpr basic_unmanaged_string& erase(size_type index = 0, size_type count = npos) {
    fst_assert(index <= _size, "basic_unmanaged_string::insert index out of bounds.");
    size_type s_size = fst::minimum(count, _size - index);
    size_type delta = _size - s_size;
    std::memmove(
        (void*)(_buffer.data() + index), (const void*)(_buffer.data() + index + s_size), delta * sizeof(value_type));
    _size -= s_size;
    _buffer[_size] = 0;
    return *this;
  }

  //  constexpr iterator erase( const_iterator position );
  //  constexpr iterator erase( const_iterator first, const_iterator last );

  //
  // Resize.
  //
  inline constexpr void resize(size_type count, value_type c = value_type()) {
    fst_assert(count <= max_size(), "basic_unmanaged_string::resize count must be smaller or equal to maximum_size.");
    if (count < _size) {
      _size = count;
      _buffer[_size] = 0;
    }
    else if (count > _size) {
      std::fill_n(_buffer.data() + _size, count - _size, c);
      _size = count;
      _buffer[_size] = 0;
    }
  }

  //
  //
  //
  inline constexpr void to_upper_case() {
    for (std::size_t i = 0; i < _size; i++) {
      _buffer[i] = fst::to_upper_case(_buffer[i]);
    }
  }

  inline constexpr void to_lower_case() {
    for (std::size_t i = 0; i < _size; i++) {
      _buffer[i] = fst::to_lower_case(_buffer[i]);
    }
  }

  //
  // Convert.
  //
  inline string_type to_string() const { return string_type(data(), size()); }
  inline operator string_type() const { return to_string(); }

  inline constexpr view_type to_view() const { return view_type(data(), size()); }
  inline constexpr operator view_type() const { return to_view(); }

  friend std::ostream& operator<<(std::ostream& stream, const basic_unmanaged_string& bss) {
    return stream << bss.to_view();
  }

private:
  buffer_type _buffer;
  std::size_t _size = 0;
};
//
////
//// Operator ==
////
// template <class _CharT, std::size_t _Size>
// inline bool operator==(
//    const basic_unmanaged_string<_CharT, _Size>& __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  using view_type = typename basic_unmanaged_string<_CharT, _Size>::view_type;
//  return (__lhs.size() == __rhs.size()) && (view_type(__lhs) == view_type(__rhs));
//}
//
// template <class _CharT, std::size_t _Size>
// inline bool operator==(const basic_unmanaged_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
//  using view_type = typename basic_unmanaged_string<_CharT, _Size>::view_type;
//  return view_type(__lhs) == view_type(__rhs);
//}
//
// template <class _CharT, std::size_t _Size>
// inline bool operator==(const _CharT* __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  using view_type = typename basic_unmanaged_string<_CharT, _Size>::view_type;
//  return view_type(__lhs) == view_type(__rhs);
//}
//
////
//// Operator !=
////
// template <class _CharT, std::size_t _Size>
// inline bool operator!=(
//    const basic_unmanaged_string<_CharT, _Size>& __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  return !(__lhs == __rhs);
//}
//
// template <class _CharT, std::size_t _Size>
// inline bool operator!=(const basic_unmanaged_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
//  return !(__lhs == __rhs);
//}
//
// template <class _CharT, std::size_t _Size>
// inline bool operator!=(const _CharT* __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  return !(__lhs == __rhs);
//}
//
////
//// Operator <
////
// template <class _CharT, std::size_t _Size>
// inline bool operator<(
//    const basic_unmanaged_string<_CharT, _Size>& __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  using view_type = typename basic_unmanaged_string<_CharT, _Size>::view_type;
//  return view_type(__lhs).compare(view_type(__rhs)) < 0;
//}
//
// template <class _CharT, std::size_t _Size>
// inline bool operator<(const basic_unmanaged_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
//  using view_type = typename basic_unmanaged_string<_CharT, _Size>::view_type;
//  return view_type(__lhs).compare(view_type(__rhs)) < 0;
//}
//
// template <class _CharT, std::size_t _Size>
// inline bool operator<(const _CharT* __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  using view_type = typename basic_unmanaged_string<_CharT, _Size>::view_type;
//  return view_type(__lhs).compare(view_type(__rhs)) < 0;
//}
//
////
//// Operator >
////
// template <class _CharT, std::size_t _Size>
// inline bool operator>(
//    const basic_unmanaged_string<_CharT, _Size>& __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  return __rhs < __lhs;
//}
//
// template <class _CharT, std::size_t _Size>
// inline bool operator>(const basic_unmanaged_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
//  return __rhs < __lhs;
//}
//
// template <class _CharT, std::size_t _Size>
// inline bool operator>(const _CharT* __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  return __rhs < __lhs;
//}
//
////
//// Operator <=
////
// template <class _CharT, std::size_t _Size>
// inline bool operator<=(
//    const basic_unmanaged_string<_CharT, _Size>& __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  return !(__rhs < __lhs);
//}
//
// template <class _CharT, std::size_t _Size>
// inline bool operator<=(const basic_unmanaged_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
//  return !(__rhs < __lhs);
//}
//
// template <class _CharT, std::size_t _Size>
// inline bool operator<=(const _CharT* __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  return !(__rhs < __lhs);
//}
//
////
//// Operator >=
////
// template <class _CharT, std::size_t _Size>
// inline bool operator>=(
//    const basic_unmanaged_string<_CharT, _Size>& __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  return !(__lhs < __rhs);
//}
//
// template <class _CharT, std::size_t _Size>
// inline bool operator>=(const basic_unmanaged_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
//  return !(__lhs < __rhs);
//}
//
// template <class _CharT, std::size_t _Size>
// inline bool operator>=(const _CharT* __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  return !(__lhs < __rhs);
//}
//
//// operator +
// template <class _CharT, std::size_t _Size>
// inline basic_unmanaged_string<_CharT, _Size> operator+(
//    const basic_unmanaged_string<_CharT, _Size>& __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  basic_unmanaged_string<_CharT, _Size> s = __lhs;
//  s += __rhs;
//  return s;
//}
//
// template <class _CharT, std::size_t _Size>
// inline basic_unmanaged_string<_CharT, _Size> operator+(_CharT __lhs, const basic_unmanaged_string<_CharT, _Size>&
// __rhs) {
//  basic_unmanaged_string<_CharT, _Size> s;
//  s.push_back(__lhs);
//  s.append(__rhs);
//  return s;
//}
//
// template <class _CharT, std::size_t _Size>
// inline basic_unmanaged_string<_CharT, _Size> operator+(
//    const basic_unmanaged_string<_CharT, _Size>& __lhs, _CharT __rhs) noexcept {
//  basic_unmanaged_string<_CharT, _Size> s = __lhs;
//  s.push_back(__rhs);
//  return s;
//}
//
// template <class _CharT, std::size_t _Size>
// inline basic_unmanaged_string<_CharT, _Size> operator+(
//    const _CharT* __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  basic_unmanaged_string<_CharT, _Size> s = __lhs;
//  s += __rhs;
//  return s;
//}
//
// template <class _CharT, std::size_t _Size>
// inline basic_unmanaged_string<_CharT, _Size> operator+(
//    const basic_unmanaged_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
//  basic_unmanaged_string<_CharT, _Size> s = __lhs;
//  s += __rhs;
//  return s;
//}
//
// template <class _CharT, std::size_t _Size>
// inline basic_unmanaged_string<_CharT, _Size> operator+(
//    basic_unmanaged_string<_CharT, _Size>&& __lhs, const basic_unmanaged_string<_CharT, _Size>& __rhs) noexcept {
//  return std::move(__lhs.append(__rhs));
//}
//
// template <class _CharT, std::size_t _Size>
// inline basic_unmanaged_string<_CharT, _Size> operator+(
//    const basic_unmanaged_string<_CharT, _Size>& __lhs, basic_unmanaged_string<_CharT, _Size>&& __rhs) noexcept {
//  return std::move(__rhs.insert(0, __lhs));
//}
//
// template <class _CharT, std::size_t _Size>
// inline basic_unmanaged_string<_CharT, _Size> operator+(
//    basic_unmanaged_string<_CharT, _Size>&& __lhs, basic_unmanaged_string<_CharT, _Size>&& __rhs) noexcept {
//  return std::move(__lhs.append(__rhs));
//}
//
// template <class _CharT, std::size_t _Size>
// inline basic_unmanaged_string<_CharT, _Size> operator+(
//    basic_unmanaged_string<_CharT, _Size>&& __lhs, const _CharT* __rhs) noexcept {
//  return std::move(__lhs.append(__rhs));
//}
//
// template <class _CharT, std::size_t _Size>
// inline basic_unmanaged_string<_CharT, _Size> operator+(
//    const _CharT* __lhs, basic_unmanaged_string<_CharT, _Size>&& __rhs) noexcept {
//  return std::move(__rhs.insert(0, __lhs));
//}
//
// template <class _CharT, std::size_t _Size>
// inline basic_unmanaged_string<_CharT, _Size> operator+(_CharT __lhs, basic_unmanaged_string<_CharT, _Size>&& __rhs)
// noexcept {
//  __rhs.insert(__rhs.begin(), __lhs);
//  return std::move(__rhs);
//}
//
// template <class _CharT, std::size_t _Size>
// inline basic_unmanaged_string<_CharT, _Size> operator+(basic_unmanaged_string<_CharT, _Size>&& __lhs, _CharT __rhs)
// noexcept {
//  __lhs.push_back(__rhs);
//  return std::move(__lhs);
//}

// template <std::size_t N>
using unmanaged_string = basic_unmanaged_string<char>;

// template <typename T, std::size_t _Size>
// inline std::pair<bool, T> to_number(std::string_view str) {
//  T value;
//
//  small_stringstream<_Size> stream;
//  stream << str;
//  stream >> value;
//  return std::pair<bool, T>{ !stream.fail(), value };
//}
} // namespace fst.
