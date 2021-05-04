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
#include "fst/assert.h"
#include "fst/ascii.h"
#include "fst/traits.h"
#include "fst/span.h"
#include "fst/print.h"
#include "fst/util.h"
#include "fst/verified_value.h"

#include "fst/unmanaged_string.h"
#include "fst/detail/dragonbox/dragonbox.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <iostream>
#include <initializer_list>
#include <limits>
#include <string>
#include <string_view>
#include <sstream>
#include <stdexcept>

namespace fst::string_conv {
template <typename T, class = typename std::enable_if<std::is_arithmetic<T>::value, void>::type>
inline constexpr const char* type_to_format() {
  if constexpr (std::is_integral<T>::value) {
    if constexpr (std::is_same<T, std::size_t>::value) {
      return "%zu";
    }
    else if constexpr (std::is_same<T, std::ptrdiff_t>::value) {
      return "%td";
    }
    else if constexpr (std::is_same<T, char>::value) {
      return "%hhd";
    }
    else if constexpr (std::is_same<T, short>::value) {
      return "%hd";
    }
    else if constexpr (std::is_same<T, int>::value) {
      return "%d";
    }
    else if constexpr (std::is_same<T, long>::value) {
      return "%ld";
    }
    else if constexpr (std::is_same<T, long long>::value) {
      return "%lld";
    }
    else if constexpr (std::is_same<T, unsigned char>::value) {
      return "%hhu";
    }
    else if constexpr (std::is_same<T, unsigned short>::value) {
      return "%hu";
    }
    else if constexpr (std::is_same<T, unsigned int>::value) {
      return "%u";
    }
    else if constexpr (std::is_same<T, unsigned long>::value) {
      return "%lu";
    }
    else if constexpr (std::is_same<T, unsigned long long>::value) {
      return "%llu";
    }
    else {
      return "%d";
    }
  }
  else if constexpr (std::is_floating_point<T>::value) {
    if constexpr (std::is_same<T, float>::value) {
      return "%f";
    }
    else if constexpr (std::is_same<T, long double>::value) {
      return "%LF";
    }
    else {
      return "%lf";
    }
  }

  return "%d";
}

template <typename T, std::size_t Precision,
    class = typename std::enable_if<std::is_floating_point<T>::value, void>::type>
inline constexpr std::array<char, 8> type_to_format() {
  static_assert(Precision < 99, "");

  if constexpr (std::is_same<T, float>::value) {
    if constexpr (Precision < 10) {
      std::array<char, 8> s = { "%.0f" };
      s[2] = '0' + Precision;
      return s;
    }
    else {
      std::array<char, 8> s = { "%.00f" };
      constexpr unsigned char p0 = Precision / 10;
      constexpr unsigned char p1 = Precision - (Precision / 10) * 10;
      s[2] = '0' + p0;
      s[3] = '0' + p1;
      return s;
    }
  }
  else if constexpr (std::is_same<T, long double>::value) {
    if constexpr (Precision < 10) {
      std::array<char, 8> s = { "%.0LF" };
      s[2] = '0' + Precision;
      return s;
    }
    else {
      std::array<char, 8> s = { "%.00LF" };
      constexpr unsigned char p0 = Precision / 10;
      constexpr unsigned char p1 = Precision - (Precision / 10) * 10;
      s[2] = '0' + p0;
      s[3] = '0' + p1;
      return s;
    }
  }
  else {
    if constexpr (Precision < 10) {
      std::array<char, 8> s = { "%.0lf" };
      s[2] = '0' + Precision;
      return s;
    }
    else {
      std::array<char, 8> s = { "%.00lf" };
      constexpr unsigned char p0 = Precision / 10;
      constexpr unsigned char p1 = Precision - (Precision / 10) * 10;
      s[2] = '0' + p0;
      s[3] = '0' + p1;
      return s;
    }
  }
}

template <typename T, class = typename std::enable_if<std::is_arithmetic<T>::value, void>::type>
inline bool to_number(const char* str, const char* format, T& value) {
  return std::sscanf(str, format, &value) != EOF;
}

template <typename T, class = typename std::enable_if<std::is_arithmetic<T>::value, void>::type>
inline std::size_t from_number(T value, char* buffer, const char* format, std::size_t maximum_size) {
  int result = std::snprintf(buffer, maximum_size, format, value);
  return fst::maximum(result, 0);
}
} // namespace fst::string_conv.

namespace fst::string_conv_v2 {
template <typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>, T>>
inline fst::verified_value<T> to_number(std::string_view str);

template <typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>, T>>
inline std::string_view to_string(fst::span<char> buffer, T value);

template <std::size_t _Precision, typename T, typename = typename std::enable_if_t<std::is_floating_point_v<T>, T>>
inline std::string_view to_string(fst::span<char> buffer, T value);

template <typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>, T>>
inline std::string to_string(T value);

template <std::size_t _Precision, typename T, typename = typename std::enable_if_t<std::is_floating_point_v<T>, T>>
inline std::string to_string(T value);

namespace detail {
  //
  // String to number.
  //
  inline std::size_t get_first_digit_index(std::string_view str) {
    for (std::size_t i = 0; i < str.size(); i++) {
      if (fst::is_digit(str[i])) {
        return i;
      }
    }

    return str.size();
  }

  inline std::size_t get_dot_or_space_index(std::size_t begin_index, std::string_view str) {
    for (std::size_t i = begin_index; i < str.size(); i++) {
      if (!fst::is_digit(str[i])) {
        return i;
      }
    }

    return str.size();
  }

  inline std::size_t get_first_not_digit_index(std::size_t begin_index, std::string_view str) {
    for (std::size_t i = begin_index; i < str.size(); i++) {
      if (!fst::is_digit(str[i])) {
        return i;
      }
    }

    return str.size();
  }

  template <typename T>
  struct integer_mult_values {};

  template <>
  struct integer_mult_values<char> {
    static constexpr const char values[] = { 1, 10, 100 };
  };

  template <>
  struct integer_mult_values<unsigned char> {
    static constexpr const unsigned char values[] = { 1, 10, 100 };
  };

  template <>
  struct integer_mult_values<short> {
    static constexpr const short values[] = { 1, 10, 100, 1000, 10000 };
  };

  template <>
  struct integer_mult_values<unsigned short> {
    static constexpr const unsigned short values[] = { 1, 10, 100, 1000, 10000 };
  };

  template <>
  struct integer_mult_values<int> {
    static constexpr const int values[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
  };

  template <>
  struct integer_mult_values<unsigned int> {
    static constexpr const unsigned int values[]
        = { 1u, 10u, 100u, 1000u, 10000u, 100000u, 1000000u, 10000000u, 100000000u, 1000000000u };
  };

  template <>
  struct integer_mult_values<long> {
    static constexpr const long values[] = { 1L, 10L, 100L, 1000L, 10000L, 100000L, 1000000L, 10000000L, 100000000L,
      1000000000L, 10000000000L, 100000000000L, 1000000000000L, 10000000000000L, 100000000000000L, 1000000000000000L,
      10000000000000000L, 100000000000000000L, 1000000000000000000L };
  };

  template <>
  struct integer_mult_values<unsigned long> {
    static constexpr const unsigned long values[]
        = { 1UL, 10UL, 100UL, 1000UL, 10000UL, 100000UL, 1000000UL, 10000000UL, 100000000UL, 1000000000UL,
            10000000000UL, 100000000000UL, 1000000000000UL, 10000000000000UL, 100000000000000UL, 1000000000000000UL,
            10000000000000000UL, 100000000000000000UL, 1000000000000000000UL, 10000000000000000000UL };
  };

  template <>
  struct integer_mult_values<long long> {
    static constexpr const long long values[] = { 1LL, 10LL, 100LL, 1000LL, 10000LL, 100000LL, 1000000LL, 10000000LL,
      100000000LL, 1000000000LL, 10000000000LL, 100000000000LL, 1000000000000LL, 10000000000000LL, 100000000000000LL,
      1000000000000000LL, 10000000000000000LL, 100000000000000000LL, 1000000000000000000LL };
  };

  template <>
  struct integer_mult_values<unsigned long long> {
    static constexpr const unsigned long long values[] = { 1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL,
      1000000ULL, 10000000ULL, 100000000ULL, 1000000000ULL, 10000000000ULL, 100000000000ULL, 1000000000000ULL,
      10000000000000ULL, 100000000000000ULL, 1000000000000000ULL, 10000000000000000ULL, 100000000000000000ULL,
      1000000000000000000ULL, 10000000000000000000ULL };
  };

  template <typename T>
  inline T to_signed_integer(std::string_view str) {
    std::size_t begin_index = detail::get_first_digit_index(str);
    std::size_t dot_or_space_index = detail::get_dot_or_space_index(begin_index, str);

    T value = 0;
    std::size_t mul_index = dot_or_space_index - begin_index - 1;
    for (std::size_t i = begin_index; i < dot_or_space_index; i++) {
      value += (str[i] - '0') * integer_mult_values<T>::values[mul_index--];
    }

    if (begin_index == 0) {
      return value;
    }

    return str[begin_index - 1] == '-' ? -value : value;
  }

  template <typename T>
  inline T to_unsigned_integer(std::string_view str) {
    std::size_t begin_index = detail::get_first_digit_index(str);
    std::size_t dot_or_space_index = detail::get_dot_or_space_index(begin_index, str);

    T value = 0;
    std::size_t mul_index = dot_or_space_index - begin_index - 1;
    for (std::size_t i = begin_index; i < dot_or_space_index; i++) {
      value += (str[i] - '0') * integer_mult_values<T>::values[mul_index--];
    }

    return value;
  }

  inline constexpr std::size_t c_to_n_size = '9' + 1;
  inline constexpr std::array<double, c_to_n_size> get_char_to_number_array() {
    return { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  }

  inline constexpr std::array<char, 10> get_number_to_char_array() {
    return { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
  }

  template <typename T>
  inline T to_real(std::string_view str) {
    std::size_t begin_index = detail::get_first_digit_index(str);
    std::size_t dot_or_space_index = detail::get_dot_or_space_index(begin_index, str);
    const double sign = begin_index == 0 ? 1 : str[begin_index - 1] == '-' ? -1 : 1;

    // clang-format off
    static constexpr const double values[] = {
      1000000000000000000.0L,
      100000000000000000.0L,
      10000000000000000.0L,
      1000000000000000.0L,
      100000000000000.0L,
      10000000000000.0L,
      1000000000000.0L,
      100000000000.0L,
      10000000000.0L,
      1000000000.0L,
      100000000.0L,
      10000000.0L,
      1000000.0L,
      100000.0L,
      10000.0L,
      1000.0L,
      100.0L,
      10.0L,
      1.0L
    };
    static constexpr std::size_t v_size = sizeof(values) / sizeof(double);
    // clang-format on

    double value = 0;
    static constexpr auto c_to_n = get_char_to_number_array();
    for (std::size_t i = begin_index, k = v_size - dot_or_space_index + begin_index; i < dot_or_space_index; i++, k++) {
      value += c_to_n[str[i]] * values[k];
    }

    // Is actually an integer.
    const bool is_dot = str[dot_or_space_index] == '.';
    if (dot_or_space_index >= str.size() - 1 || (is_dot && !fst::is_digit(str[dot_or_space_index + 1]))) {
      return sign * value;
    }

    // clang-format off
    static constexpr const double inv_mults[] = {
      1.0L / 10.0L,
      1.0L / 100.0L,
      1.0L / 1000.0L,
      1.0L / 10000.0L,
      1.0L / 100000.0L,
      1.0L / 1000000.0L,
      1.0L / 10000000.0L,
      1.0L / 100000000.0L,
      1.0L / 1000000000.0L,
      1.0L / 10000000000.0L,
      1.0L / 100000000000.0L,
      1.0L / 1000000000000.0L,
    };
    // clang-format on

    const std::size_t end_index = detail::get_first_not_digit_index(dot_or_space_index + 1, str);
    for (std::size_t i = dot_or_space_index + 1, k = 0; i < end_index; i++, k++) {
      value += c_to_n[str[i]] * inv_mults[k];
    }

    // TODO: Handle exponent.

    return sign * value;
  }

  //
  // Number to string.
  //

//  template <typename T>
//  inline std::string_view signed_to_string(fst::span<char> buffer, T value) {
//
//    static constexpr const char digit_pairs[] = { "00010203040506070809"
//                                                  "10111213141516171819"
//                                                  "20212223242526272829"
//                                                  "30313233343536373839"
//                                                  "40414243444546474849"
//                                                  "50515253545556575859"
//                                                  "60616263646566676869"
//                                                  "70717273747576777879"
//                                                  "80818283848586878889"
//                                                  "90919293949596979899" };
//
//    if (value == 0) {
//      buffer[0] = '0';
//      return std::string_view(buffer.data(), 1);
//    }
//
//    const T sign = -(value < 0);
//    T val = (value ^ sign) - sign;
//
//    T size = 1 - sign;
//    for (T mul = 10; val >= mul; mul *= 10) {
//      size++;
//    }
//    //--------------
//
//    char* c = buffer.data();
//    *c = '-';
//    c += size - 1;
//
////    while (val > 9) {
////      *(short*)(c - 1) = *(const short*)(digit_pairs + 2 * (val % 100));
////      val /= 100;
////      c -= 2;
////    }
//
////    int div = val / 100;
////    while (val > 9) {
////      *(short*)(c - 1) = *(const short*)(digit_pairs + 2 * (val - div * 100));
////      val = div;
////      div /= 100;
////      c -= 2;
////    }
//
////    int div = val / 100;
//    while (val > 9) {
//      T v100 = val / 100;
//      *(short*)(c - 1) = *(const short*)(digit_pairs + 2 * (val - v100 * 100));
//      val = v100;
////      div /= 100;
//      c -= 2;
//    }
//
//    if (val) {
//      *c = '0' + val;
//    }
//
//    return std::string_view(buffer.data(), size);
//  }

  template <typename T>
  inline std::string_view signed_to_string(fst::span<char> buffer, T value) {

    static constexpr const char digit_pairs[] = { "00010203040506070809"
                                                  "10111213141516171819"
                                                  "20212223242526272829"
                                                  "30313233343536373839"
                                                  "40414243444546474849"
                                                  "50515253545556575859"
                                                  "60616263646566676869"
                                                  "70717273747576777879"
                                                  "80818283848586878889"
                                                  "90919293949596979899" };

    if (value == 0) {
      buffer[0] = '0';
      return std::string_view(buffer.data(), 1);
    }

    const T sign = -(value < 0);
    T val = (value ^ sign) - sign;

    T size = 1 - sign;
    for (T mul = 10; val >= mul; mul *= 10) {
      size++;
    }

    buffer[0] = '-';

    short* it = (short*)(buffer.data() + size - 2);
    const short* pairs = (const short*)digit_pairs;
    while (val > 9) {
      T v100 = val / 100;
      *it = pairs[val - v100 * 100];
      val = v100;
      it--;
    }

    if(val) {
      *(((char*)it) + 1) = '0' + val;
    }

    return std::string_view(buffer.data(), size);
  }
  
  template <typename T>
  inline std::string_view unsigned_to_string(fst::span<char> buffer, T val) {

    static constexpr const char digit_pairs[] = { "00010203040506070809"
                                                  "10111213141516171819"
                                                  "20212223242526272829"
                                                  "30313233343536373839"
                                                  "40414243444546474849"
                                                  "50515253545556575859"
                                                  "60616263646566676869"
                                                  "70717273747576777879"
                                                  "80818283848586878889"
                                                  "90919293949596979899" };

    if (val == 0) {
      buffer[0] = '0';
      return std::string_view(buffer.data(), 1);
    }

    T size = 1;
    for (T mul = 10; val >= mul; mul *= 10) {
      size++;
    }

    char* c = buffer.data();
    c += size - 1;

    while (val > 9) {
      *(short*)(c - 1) = *(const short*)(digit_pairs + 2 * (val % 100));
      val /= 100;
      c -= 2;
    }

    if (val) {
      *c = '0' + val;
    }

    return std::string_view(buffer.data(), size);
  }

  template <std::size_t _Precision>
  inline constexpr long get_precision_mul() {
    long v = 1;
    for (std::size_t i = 0; i < _Precision; i++) {
      v *= 10;
    }
    return v;
  }

  template <std::size_t _Precision, typename T>
  T round_to_precision(T value) {
    if constexpr (_Precision == 0) {
      return std::floor(value);
    }
    else {
      constexpr long mult = get_precision_mul<_Precision>();
      if (value < 0) {
        value = (long)(-value * mult + (T)0.5);
        return -((T)value / mult);
      }
      value = (long)(value * mult + (T)0.5);
      return (T)value / mult;
    }
  }

  template <typename T>
  inline std::string_view real_to_string(fst::span<char> buffer, T value) {
    auto dec = fst::dragonbox::to_decimal(value);
    std::string_view str = to_string(buffer, dec.significand);

    if (dec.exponent == 0) {
      if (dec.is_negative) {
        fst::unmanaged_string u_str(buffer, str.size());
        u_str.insert(0, 1, '-');
        return std::string_view(u_str.data(), u_str.size());
      }

      return str;
    }

    if (dec.significand == 0) {
      return str;
    }

    fst::unmanaged_string u_str(buffer, str.size());

    if (dec.exponent > 0) {
      u_str.append(dec.exponent, '0');
    }
    else {
      int exp = -dec.exponent;
      if (exp >= str.size()) {
        std::size_t s_size = str.size();
        u_str.insert(0, "0.");
        u_str.insert(2, exp - s_size, '0');
      }
      else {
        u_str.insert(str.size() - exp, 1, '.');
      }
    }

    if (dec.is_negative) {
      u_str.insert(0, 1, '-');
    }

    return std::string_view(u_str.data(), u_str.size());
  }

  template <std::size_t _Precision, typename T>
  inline std::string_view real_to_string(fst::span<char> buffer, T value) {

    if constexpr (_Precision == 0) {
      return to_string(buffer, (long)std::round(value));
    }
    else {
      value = round_to_precision<_Precision>(value);
      auto dec = fst::dragonbox::to_decimal(value);
      std::string_view str = to_string(buffer, dec.significand);

      if (dec.exponent == 0) {
        if (dec.is_negative) {
          fst::unmanaged_string u_str(buffer, str.size());
          u_str.insert(0, 1, '-');
          return std::string_view(u_str.data(), u_str.size());
        }

        return str;
      }

      if (dec.significand == 0) {
        return str;
      }

      fst::unmanaged_string u_str(buffer, str.size());

      if (dec.exponent > 0) {
        u_str.append(dec.exponent, '0');

        u_str.append('.');
        u_str.append(_Precision, '0');
      }
      else {
        int exp = -dec.exponent;
        if (exp >= str.size()) {
          std::size_t s_size = str.size();
          u_str.insert(0, "0.");
          u_str.insert(2, exp - s_size, '0');
        }
        else {

          u_str.insert(str.size() - exp, 1, '.');

          if (exp < _Precision) {
            u_str.append(_Precision - exp, '0');
          }
        }
      }

      if (dec.is_negative) {
        u_str.insert(0, 1, '-');
      }

      return std::string_view(u_str.data(), u_str.size());
    }
  }
} // namespace detail.

template <typename T, typename>
inline fst::verified_value<T> to_number(std::string_view str) {
  if constexpr (std::is_floating_point_v<T>) {
    return detail::to_real<T>(str);
  }
  else if constexpr (std::is_signed_v<T>) {
    return detail::to_signed_integer<T>(str);
  }
  else if constexpr (std::is_unsigned_v<T>) {
    return detail::to_unsigned_integer<T>(str);
  }
  else {
    return fst::verified_value<T>::invalid();
  }
}

template <typename T, typename>
inline std::string_view to_string(fst::span<char> buffer, T value) {
  if constexpr (std::is_floating_point_v<T>) {
    return detail::real_to_string<T>(buffer, value);
  }
  else if constexpr (std::is_signed_v<T>) {
    return detail::signed_to_string<T>(buffer, value);
  }
  else if constexpr (std::is_unsigned_v<T>) {
    return detail::unsigned_to_string<T>(buffer, value);
  }
}

template <std::size_t _Precision, typename T, typename>
inline std::string_view to_string(fst::span<char> buffer, T value) {
  return detail::real_to_string<_Precision, T>(buffer, value);
}

template <typename T, typename>
inline std::string to_string(T value) {
  std::array<char, 32> buffer;
  return std::string(to_string<T>(buffer, value));
}

template <std::size_t _Precision, typename T, typename>
inline std::string to_string(T value) {
  std::array<char, 32> buffer;
  return std::string(to_string<_Precision, T>(buffer, value));
}
} // namespace fst::string_conv_v2.
