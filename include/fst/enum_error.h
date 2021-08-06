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
#include <fst/traits>
#include <string_view>

namespace fst {

namespace enum_error_detail {
  template <typename _ToStringEnumArray>
  struct to_string_array_impl {
    using array_type = decltype(_ToStringEnumArray::array);
    using enum_type = typename array_type::enum_type;
    using value_type = typename array_type::value_type;
    static constexpr const array_type& string_array = _ToStringEnumArray::array;

    static constexpr bool is_convertible_to_string_view
        = std::is_convertible_v<const value_type&,
              std::basic_string_view<char,
                  std::char_traits<char>>> || std::is_convertible_v<const value_type&, const char*>;

    static_assert(is_convertible_to_string_view, "To string array::value_type must be convertible to string_view.");

    inline static constexpr std::string_view to_string(enum_type e) { return string_array[e]; }
  };

  struct empty_to_string_array {};

  template <typename _ToStringEnumArray>
  using to_string_array_base = std::conditional_t<std::is_same_v<_ToStringEnumArray, void>, empty_to_string_array,
      to_string_array_impl<_ToStringEnumArray>>;

} // namespace enum_error_detail.

template <typename _Enum, _Enum _ValidResult = (_Enum)0, typename _ToStringEnumArray = void>
class enum_error : private enum_error_detail::to_string_array_base<_ToStringEnumArray> {
public:
  using enum_type = _Enum;
  static constexpr _Enum valid_result = _ValidResult;
  using to_string_base = enum_error_detail::to_string_array_base<_ToStringEnumArray>;

  using has_base = std::bool_constant<!std::is_same_v<_ToStringEnumArray, void>>;

  constexpr enum_error() noexcept = default;
  constexpr enum_error(const enum_error&) noexcept = default;
  constexpr enum_error(enum_error&&) noexcept = default;

  inline constexpr enum_error(enum_type res) noexcept
      : _result(res) {}

  constexpr ~enum_error() noexcept = default;

  constexpr enum_error& operator=(const enum_error&) noexcept = default;
  constexpr enum_error& operator=(enum_error&&) noexcept = default;

  inline constexpr enum_type get() const noexcept { return _result; }
  inline constexpr bool is_valid() const noexcept { return _result == valid_result; }

  /// Returns true on error.
  inline constexpr explicit operator bool() const noexcept { return !is_valid(); }
  inline constexpr bool operator!() const noexcept { return is_valid(); }

  inline constexpr bool operator==(bool b) const noexcept { return is_valid() != b; }
  inline constexpr bool operator!=(bool b) const noexcept { return is_valid() == b; }

  inline constexpr operator enum_type() const noexcept { return _result; }

  template <bool _Dummy, class _D = dependent_type_condition<_Dummy, has_base>>
  using enable_if_has_base = enable_if_same<_Dummy, _D>;

  template <bool _Dummy = true, class = enable_if_has_base<_Dummy>>
  inline constexpr std::string_view to_string() const {
    return to_string_base::to_string(_result);
  }

  template <bool _Dummy = true, class = enable_if_has_base<_Dummy>>
  inline constexpr operator std::string_view() const noexcept {
    return to_string();
  }

private:
  enum_type _result = valid_result;
};
} // namespace fst.
