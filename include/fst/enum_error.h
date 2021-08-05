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

namespace fst {
template <typename _Enum, _Enum _ValidResult = (_Enum)0>
class enum_error {
public:
  using enum_type = _Enum;
  static constexpr _Enum valid_result = _ValidResult;

  enum_error() noexcept = default;
  enum_error(const enum_error&) noexcept = default;
  enum_error(enum_error&&) noexcept = default;

  inline enum_error(enum_type res) noexcept
      : _result(res) {}

  ~enum_error() noexcept = default;

  enum_error& operator=(const enum_error&) noexcept = default;
  enum_error& operator=(enum_error&&) noexcept = default;

  inline enum_type get() const noexcept { return _result; }
  inline bool is_valid() const noexcept { return _result == valid_result; }

  /// Returns true on error.
  inline explicit operator bool() const noexcept { return !is_valid(); }
  inline bool operator!() const noexcept { return is_valid(); }

  inline bool operator==(bool b) const noexcept { return is_valid() != b; }
  inline bool operator!=(bool b) const noexcept { return is_valid() == b; }

  inline operator enum_type() const noexcept { return _result; }

private:
  enum_type _result = valid_result;
};
} // namespace fst.
