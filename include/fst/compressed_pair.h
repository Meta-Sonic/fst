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

namespace fst {
template <typename T1, typename T2>
class compressed_pair;

namespace detail {
  enum class compressed_pair_version { normal, first_empty, second_empty, diff_empty, same_empty, same_normal };

  template <typename T1, typename T2>
  inline constexpr compressed_pair_version get_compressed_pair_version() {
    constexpr bool same = std::is_same_v<std::remove_cv_t<T1>, std::remove_cv_t<T2>>;
    constexpr bool t1_empty = std::is_empty<T1>::value;
    constexpr bool t2_empty = std::is_empty<T2>::value;
    using version = compressed_pair_version;

    if constexpr (same) {
      return t1_empty ? version::same_empty : version::same_normal;
    }
    else if constexpr (t1_empty == t2_empty) {
      return t1_empty ? version::diff_empty : version::normal;
    }
    else {
      return t1_empty ? version::first_empty : version::second_empty;
    }
  }

  template <typename T1, typename T2, compressed_pair_version version>
  class compressed_pair_imp;

  // Derive from neither.
  template <typename T1, typename T2>
  class compressed_pair_imp<T1, T2, compressed_pair_version::normal> {
  public:
    using first_type = T1;
    using second_type = T2;
    using first_param_type = fst::call_param_type_t<first_type>;
    using second_param_type = fst::call_param_type_t<second_type>;
    using first_reference = std::remove_reference_t<first_type>&;
    using second_reference = std::remove_reference_t<second_type>&;
    using first_const_reference = const std::remove_reference_t<first_type>&;
    using second_const_reference = const std::remove_reference_t<second_type>&;

    compressed_pair_imp() = default;

    compressed_pair_imp(first_param_type x, second_param_type y)
        : _first(x)
        , _second(y) {}

    explicit compressed_pair_imp(first_param_type x)
        : _first(x) {}

    explicit compressed_pair_imp(second_param_type y)
        : _second(y) {}

    inline first_reference first() { return _first; }
    inline first_const_reference first() const { return _first; }

    inline second_reference second() { return _second; }
    inline second_const_reference second() const { return _second; }

    inline void swap(compressed_pair<T1, T2>& y) {
      std::swap(_first, y.first());
      std::swap(_second, y.second());
    }

  private:
    first_type _first;
    second_type _second;
  };

  // Derive from T1.
  template <typename T1, typename T2>
  class compressed_pair_imp<T1, T2, compressed_pair_version::first_empty> : private T1 {
  public:
    using first_type = T1;
    using second_type = T2;
    using first_param_type = fst::call_param_type_t<first_type>;
    using second_param_type = fst::call_param_type_t<second_type>;
    using first_reference = std::remove_reference_t<first_type>&;
    using second_reference = std::remove_reference_t<second_type>&;
    using first_const_reference = const std::remove_reference_t<first_type>&;
    using second_const_reference = const std::remove_reference_t<second_type>&;

    compressed_pair_imp() = default;

    compressed_pair_imp(first_param_type x, second_param_type y)
        : first_type(x)
        , _second(y) {}

    explicit compressed_pair_imp(first_param_type x)
        : first_type(x) {}

    explicit compressed_pair_imp(second_param_type y)
        : _second(y) {}

    inline first_reference first() { return *this; }
    inline first_const_reference first() const { return *this; }

    inline second_reference second() { return _second; }
    inline second_const_reference second() const { return _second; }

    inline void swap(compressed_pair<T1, T2>& y) {
      // No need to swap empty base class.
      std::swap(_second, y.second());
    }

  private:
    second_type _second;
  };

  // Derive from T2.
  template <typename T1, typename T2>
  class compressed_pair_imp<T1, T2, compressed_pair_version::second_empty> : private T2 {
  public:
    using first_type = T1;
    using second_type = T2;
    using first_param_type = fst::call_param_type_t<first_type>;
    using second_param_type = fst::call_param_type_t<second_type>;
    using first_reference = std::remove_reference_t<first_type>&;
    using second_reference = std::remove_reference_t<second_type>&;
    using first_const_reference = const std::remove_reference_t<first_type>&;
    using second_const_reference = const std::remove_reference_t<second_type>&;

    compressed_pair_imp() = default;

    compressed_pair_imp(first_param_type x, second_param_type y)
        : second_type(y)
        , _first(x) {}

    explicit compressed_pair_imp(first_param_type x)
        : _first(x) {}

    explicit compressed_pair_imp(second_param_type y)
        : second_type(y) {}

    inline first_reference first() { return _first; }
    inline first_const_reference first() const { return _first; }

    inline second_reference second() { return *this; }
    inline second_const_reference second() const { return *this; }

    inline void swap(compressed_pair<T1, T2>& y) {
      // No need to swap empty base class.
      std::swap(_first, y.first());
    }

  private:
    first_type _first;
  };

  // Derive from T1 and T2.
  template <typename T1, typename T2>
  class compressed_pair_imp<T1, T2, compressed_pair_version::diff_empty> : private T1, private T2 {
  public:
    using first_type = T1;
    using second_type = T2;
    using first_param_type = fst::call_param_type_t<first_type>;
    using second_param_type = fst::call_param_type_t<second_type>;
    using first_reference = std::remove_reference_t<first_type>&;
    using second_reference = std::remove_reference_t<second_type>&;
    using first_const_reference = const std::remove_reference_t<first_type>&;
    using second_const_reference = const std::remove_reference_t<second_type>&;

    compressed_pair_imp() = default;

    compressed_pair_imp(first_param_type x, second_param_type y)
        : first_type(x)
        , second_type(y) {}

    explicit compressed_pair_imp(first_param_type x)
        : first_type(x) {}

    explicit compressed_pair_imp(second_param_type y)
        : second_type(y) {}

    inline first_reference first() { return *this; }
    inline first_const_reference first() const { return *this; }
    inline second_reference second() { return *this; }
    inline second_const_reference second() const { return *this; }
    inline void swap(compressed_pair<T1, T2>&) {}
  };

  /// T1 == T2, T1 and T2 are both empty.
  /// Note does not actually store an instance of T2 at all;
  /// but reuses T1 base class for both first() and second().
  template <typename T1, typename T2>
  class compressed_pair_imp<T1, T2, compressed_pair_version::same_empty> : private T1 {
  public:
    using first_type = T1;
    using second_type = T2;
    using first_param_type = fst::call_param_type_t<first_type>;
    using second_param_type = fst::call_param_type_t<second_type>;
    using first_reference = std::remove_reference_t<first_type>&;
    using second_reference = std::remove_reference_t<second_type>&;
    using first_const_reference = const std::remove_reference_t<first_type>&;
    using second_const_reference = const std::remove_reference_t<second_type>&;

    compressed_pair_imp() = default;

    compressed_pair_imp(first_param_type x, second_param_type)
        : first_type(x) {}

    explicit compressed_pair_imp(first_param_type x)
        : first_type(x) {}

    inline first_reference first() { return *this; }
    inline first_const_reference first() const { return *this; }
    inline second_reference second() { return *this; }
    inline second_const_reference second() const { return *this; }
    inline void swap(compressed_pair<T1, T2>&) {}
  };

  // T1 == T2 and are not empty.
  template <typename T1, typename T2>
  class compressed_pair_imp<T1, T2, compressed_pair_version::same_normal> {
  public:
    using first_type = T1;
    using second_type = T2;
    using first_param_type = fst::call_param_type_t<first_type>;
    using second_param_type = fst::call_param_type_t<second_type>;
    using first_reference = std::remove_reference_t<first_type>&;
    using second_reference = std::remove_reference_t<second_type>&;
    using first_const_reference = const std::remove_reference_t<first_type>&;
    using second_const_reference = const std::remove_reference_t<second_type>&;

    compressed_pair_imp() = default;

    inline compressed_pair_imp(first_param_type x, second_param_type y)
        : _first(x)
        , _second(y) {}

    inline explicit compressed_pair_imp(first_param_type x)
        : _first(x)
        , _second(x) {}

    inline first_reference first() { return _first; }
    inline first_const_reference first() const { return _first; }
    inline second_reference second() { return _second; }
    inline second_const_reference second() const { return _second; }

    void swap(compressed_pair<T1, T2>& y) {
      std::swap(_first, y.first());
      std::swap(_second, y.second());
    }

  private:
    first_type _first;
    second_type _second;
  };

  template <typename T1, typename T2>
  using compressed_pair_base = compressed_pair_imp<T1, T2, get_compressed_pair_version<T1, T2>()>;
} // namespace detail.

template <typename T1, typename T2>
class compressed_pair : private detail::compressed_pair_base<T1, T2> {
private:
  using base = detail::compressed_pair_base<T1, T2>;

public:
  using first_type = typename base::first_type;
  using second_type = typename base::second_type;
  using first_param_type = typename base::first_param_type;
  using second_param_type = typename base::second_param_type;
  using first_reference = typename base::first_reference;
  using second_reference = typename base::second_reference;
  using first_const_reference = typename base::first_const_reference;
  using second_const_reference = typename base::second_const_reference;

  using base::base;

  inline first_reference first() { return base::first(); }
  inline first_const_reference first() const { return base::first(); }
  inline second_reference second() { return base::second(); }
  inline second_const_reference second() const { return base::second(); }
  inline void swap(compressed_pair& y) { base::swap(y); }
};

template <typename T1, typename T2>
inline void swap(compressed_pair<T1, T2>& x, compressed_pair<T1, T2>& y) {
  x.swap(y);
}
} // namespace fst
