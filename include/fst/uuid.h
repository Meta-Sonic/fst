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
#include <fst/assert>
#include <fst/ascii>
#include <fst/span>
#include <algorithm>
#include <array>
#include <iostream>
#include <random>

namespace fst {
// https://github.com/mariusbancila/stduuid
class uuid {
public:
  using value_type = std::uint8_t;

  uuid() noexcept = default;
  ~uuid() noexcept = default;

  uuid(value_type (&arr)[16]) noexcept {
    std::copy(std::cbegin(arr), std::cend(arr), std::begin(_data));
    _is_valid = true;
  }

  uuid(const std::array<value_type, 16>& data) noexcept
      : _data(data)
      , _is_valid(true) {}

  explicit uuid(fst::span<value_type> bytes) noexcept {
    fst_assert(bytes.size() == 16, "bytes size must be 16.");
    if (bytes.size() != 16) {
      _is_valid = false;
      _data.fill(0);
      return;
    }

    std::copy(std::cbegin(bytes), std::cend(bytes), std::begin(_data));
    _is_valid = true;
  }

  template <typename _ForwardIterator>
  explicit uuid(_ForwardIterator first, _ForwardIterator last) noexcept {
    if (std::distance(first, last) != 16) {
      _is_valid = false;
      _data.fill(0);
      return;
    }

    std::copy(first, last, std::begin(_data));
    _is_valid = true;
  }

  uuid& operator=(const uuid& rhs) noexcept = default;
  uuid& operator=(uuid&& rhs) noexcept = default;

  inline static uuid create() {
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    std::uniform_int_distribution<std::uint32_t> distribution;

    std::uint8_t bytes[16];
    for (int i = 0; i < 16; i += 4) {
      *reinterpret_cast<std::uint32_t*>(bytes + i) = distribution(generator);
    }

    // Variant must be 10xxxxxx.
    bytes[8] &= 0xBF;
    bytes[8] |= 0x80;

    // Version must be 0100xxxx.
    bytes[6] &= 0x4F;
    bytes[6] |= 0x40;

    return uuid(std::begin(bytes), std::end(bytes));
  }

  inline bool is_valid() const { return _is_valid; }

  static bool is_valid(const char* str) noexcept {
    bool firstDigit = true;
    int hasBraces = 0;
    size_t index = 0;
    size_t size = strlen(str);

    if (str == nullptr || size == 0) {
      return false;
    }

    if (str[0] == '{') {
      hasBraces = 1;
    }

    if (hasBraces && str[size - 1] != '}') {
      return false;
    }

    for (size_t i = hasBraces; i < size - hasBraces; ++i) {
      if (str[i] == '-') {
        continue;
      }

      if (index >= 16 || !fst::is_hex(str[i])) {
        return false;
      }

      if (firstDigit) {
        firstDigit = false;
      }
      else {
        index++;
        firstDigit = true;
      }
    }

    return index >= 16;
  }

  static uuid from_string(const char* str) noexcept {
    char digit = 0;
    bool firstDigit = true;
    int hasBraces = 0;
    size_t index = 0;
    size_t size = strlen(str);

    std::array<uint8_t, 16> data{ { 0 } };

    if (str == nullptr || size == 0) {
      return {};
    }

    if (str[0] == '{') {
      hasBraces = 1;
    }
    if (hasBraces && str[size - 1] != '}') {
      return {};
    }

    for (size_t i = hasBraces; i < size - hasBraces; ++i) {
      if (str[i] == '-') {
        continue;
      }

      if (index >= 16 || !fst::is_hex(str[i])) {
        return {};
      }

      if (firstDigit) {
        digit = str[i];
        firstDigit = false;
      }
      else {
        data[index++] = (fst::hex_to_char(digit) << 4) | fst::hex_to_char(str[i]);
        firstDigit = true;
      }
    }

    if (index < 16) {
      return {};
    }

    return uuid{ std::cbegin(data), std::cend(data) };
  }

  inline value_type* data() { return _data.data(); }
  inline const value_type* data() const { return _data.data(); }

  template <class CharT = char, class Traits = std::char_traits<CharT>, class Allocator = std::allocator<CharT>>
  inline std::basic_string<CharT, Traits, Allocator> to_string() const {
    std::basic_stringstream<CharT, Traits, Allocator> sstr;
    sstr << *this;
    return sstr.str();
  }

private:
  std::array<value_type, 16> _data{ { 0 } };
  bool _is_valid = false;

  friend bool operator==(const uuid& lhs, const uuid& rhs) noexcept;
  friend bool operator<(const uuid& lhs, const uuid& rhs) noexcept;

  template <class Elem, class Traits>
  friend std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& s, const uuid& id);
};

inline bool operator==(const uuid& lhs, const uuid& rhs) noexcept { return lhs._data == rhs._data; }
inline bool operator!=(const uuid& lhs, const uuid& rhs) noexcept { return !(lhs == rhs); }
inline bool operator<(const uuid& lhs, const uuid& rhs) noexcept { return lhs._data < rhs._data; }

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& s, const uuid& id) {
  // Save current flags.
  std::ios_base::fmtflags f(s.flags());

  s << std::hex << std::setfill(static_cast<Elem>('0')) << std::setw(2) << (int)id._data[0] << std::setw(2)
    << (int)id._data[1] << std::setw(2) << (int)id._data[2] << std::setw(2) << (int)id._data[3] << '-' << std::setw(2)
    << (int)id._data[4] << std::setw(2) << (int)id._data[5] << '-' << std::setw(2) << (int)id._data[6] << std::setw(2)
    << (int)id._data[7] << '-' << std::setw(2) << (int)id._data[8] << std::setw(2) << (int)id._data[9] << '-'
    << std::setw(2) << (int)id._data[10] << std::setw(2) << (int)id._data[11] << std::setw(2) << (int)id._data[12]
    << std::setw(2) << (int)id._data[13] << std::setw(2) << (int)id._data[14] << std::setw(2) << (int)id._data[15];

  // Restore original flags.
  s.flags(f);

  return s;
}

} // namespace fst.
