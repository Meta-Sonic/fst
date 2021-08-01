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
#include <cstddef>
#include <type_traits>
#include <iterator>
#include <complex>
#include <string_view>
#include <utility>
#include <tuple>

namespace fst {
#if __FST_CPP_20__
template <typename T>
using remove_cvref = std::remove_cvref<T>;

template <typename T>
using remove_cvref_t = std::remove_cvref_t<T>;
#else
template <class T>
struct remove_cvref {
  using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;
#endif // __FST_CPP_20__.

template <typename _A, typename _B>
using is_not_same = typename std::conditional<!std::is_same<_A, _B>::value, std::true_type, std::false_type>::type;

template <typename _Iterator>
using is_random_access_iterator
    = std::is_same<typename std::iterator_traits<_Iterator>::iterator_category, std::random_access_iterator_tag>;

//
// call_param_type.
//
template <typename _Tp, std::size_t _MinimumSize = sizeof(void*)>
struct call_param_type {
  using type = std::conditional_t<std::is_pointer_v<_Tp>, _Tp const,
      std::conditional_t<sizeof(_Tp) <= _MinimumSize, const _Tp, const _Tp&>>;
};

template <typename _Tp, std::size_t _MinimumSize>
struct call_param_type<_Tp&, _MinimumSize> {
  using type = _Tp&;
};

template <typename _Tp, std::size_t _MinimumSize = sizeof(void*)>
using call_param_type_t = typename call_param_type<_Tp, _MinimumSize>::type;

//
// Call traits.
//
template <typename _Tp>
struct call_traits {
  using value_type = _Tp;
  using reference = _Tp&;
  using const_reference = const _Tp&;
  using param_type = call_param_type_t<_Tp>;
};

template <typename _Tp>
struct call_traits<_Tp&> {
  using value_type = _Tp&;
  using reference = _Tp&;
  using const_reference = const _Tp&;
  using param_type = _Tp&;
};

template <typename _Tp, std::size_t N>
struct call_traits<_Tp[N]> {
private:
  using array_type = _Tp[N];

public:
  using value_type = const _Tp*;
  using reference = array_type&;
  using const_reference = const array_type&;
  using param_type = const _Tp* const;
};

template <typename _Tp, std::size_t N>
struct call_traits<const _Tp[N]> {
private:
  using array_type = const _Tp[N];

public:
  using value_type = const _Tp*;
  using reference = array_type&;
  using const_reference = const array_type&;
  using param_type = const _Tp* const;
};

template <class _Tp>
struct type_identity {
  using type = _Tp;
};

template <class _Tp, bool>
struct dependent_type : public _Tp {};

template <bool _Dummy, class D>
using dependent_type_condition = typename dependent_type<type_identity<D>, _Dummy>::type;

template <bool _Dummy, class _D>
using enable_if_same = typename std::enable_if<std::is_same<_D, std::true_type>::value>::type;

template <bool _Dummy, class _D>
using enable_if_different = typename std::enable_if<std::is_same<_D, std::false_type>::value>::type;

template <typename T, typename R = void>
struct enabled_if_or_void {
  using type = R;
};

template <typename T, typename R = void>
using enabled_if_t_or_void = typename enabled_if_or_void<T, R>::type;

namespace detector_detail {
  struct nonesuch {
    ~nonesuch() = delete;
    nonesuch(nonesuch const&) = delete;
    void operator=(nonesuch const&) = delete;
  };

  template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
  struct detector {
    using value_t = std::false_type;
    using type = Default;
  };

  template <class Default, template <class...> class Op, class... Args>
  struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
  };
} // namespace detector_detail

template <template <class...> class Op, class... Args>
using is_detected = typename detector_detail::detector<detector_detail::nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
using is_detected_t = typename detector_detail::detector<detector_detail::nonesuch, void, Op, Args...>::type;

template <template <class...> class _Op, typename K>
using type_exist = is_detected<_Op, K>;

template <class, class = void>
struct is_defined : std::false_type {};

template <class T>
struct is_defined<T, std::enable_if_t<std::is_object<T>::value && !std::is_pointer<T>::value && (sizeof(T) > 0)>>
    : std::true_type {};

template <typename... _InputT>
using tuple_cat_t = decltype(std::tuple_cat(std::declval<_InputT>()...));

// Pair.
namespace pair_detail {
  template <typename T>
  using has_first = decltype(T::first);

  template <typename T>
  using has_second = decltype(T::second);
} // namespace pair_detail.

namespace range_detail {
  template <typename T>
  using has_min = decltype(T::min);

  template <typename T>
  using has_max = decltype(T::max);
} // namespace range_detail.

template <typename T>
using is_pair = std::conjunction<type_exist<pair_detail::has_first, T>, type_exist<pair_detail::has_second, T>>;

template <typename T>
using is_range = std::conjunction<type_exist<range_detail::has_min, T>, type_exist<range_detail::has_max, T>>;

template <typename>
struct is_tuple : std::false_type {};

template <typename... T>
struct is_tuple<std::tuple<T...>> : std::true_type {};

template <typename>
struct is_complex : std::false_type {};

template <typename... T>
struct is_complex<std::complex<T...>> : std::true_type {};

template <class _CharT, class _Traits, class _Tp>
struct is_convertible_to_string_view
    : public std::bool_constant<std::is_convertible<const _Tp&, std::basic_string_view<_CharT, _Traits>>::value
          && !std::is_convertible<const _Tp&, const _CharT*>::value> {};

namespace ostream_detail {
  template <typename = void, typename... Args>
  struct has_ostream : std::false_type {};

  template <typename... Args>
  struct has_ostream<std::void_t<decltype(std::declval<std::ostream>().operator<<(std::declval<Args>()...))>, Args...>
      : std::true_type {};

  template <typename = void, typename... Args>
  struct has_global_ostream : std::false_type {};

  template <typename... Args>
  struct has_global_ostream<decltype(operator<<(std::declval<std::ostream&>(), std::declval<const Args&>()...)),
      Args...> : std::true_type {};
} // namespace ostream_detail.

template <typename... Args>
using has_ostream = std::disjunction<ostream_detail::has_ostream<void, Args...>,
    ostream_detail::has_global_ostream<std::ostream&, Args...>>;

namespace iterable_detail {
  template <typename T>
  auto is_iterable_impl(int)
      -> decltype(std::begin(std::declval<T&>()) != std::end(std::declval<T&>()), // begin/end and operator !=
          void(), // Handle evil operator ,
          ++std::declval<decltype(std::begin(std::declval<T&>()))&>(), // operator ++
          void(*std::begin(std::declval<T&>())), // operator*
          std::true_type{});

  template <typename T>
  std::false_type is_iterable_impl(...);

  template <typename T>
  auto is_const_iterable_impl(int) -> decltype(
      std::cbegin(std::declval<const T&>()) != std::cend(std::declval<const T&>()), // begin/end and operator !=
      void(), // Handle evil operator ,
      ++std::declval<const decltype(std::cbegin(std::declval<const T&>()))&>(), // operator ++
      void(*std::cbegin(std::declval<const T&>())), // operator*
      std::true_type{});

  template <typename T>
  std::false_type is_const_iterable_impl(...);
} // namespace iterable_detail

// Has begin() and end().
template <typename T>
using is_iterable = decltype(iterable_detail::is_iterable_impl<T>(0));

// Has cbegin() and cend().
template <typename T>
using is_const_iterable = decltype(iterable_detail::is_const_iterable_impl<T>(0));

template <template <typename...> class base, typename derived>
struct is_base_of_template_impl {
  template <typename... Ts>
  static constexpr std::true_type test(const base<Ts...>*);
  static constexpr std::false_type test(...);
  using type = decltype(test(std::declval<derived*>()));
};

template <template <typename...> class base, typename derived>
using is_base_of_template = typename is_base_of_template_impl<base, derived>::type;

template <typename T>
constexpr typename std::underlying_type<T>::type integral(T value) {
  return static_cast<typename std::underlying_type<T>::type>(value);
}

/// Check if a container can use memcpy when copying a buffer of T.
template <typename T>
struct is_memcopyable {
  static constexpr bool value = std::is_trivially_copy_constructible_v<T> && std::is_trivially_destructible_v<T>;
};

struct faster_without_const_reference_tag {};

namespace detail {
  template <class T>
  using is_faster_without_const_reference_t = typename T::is_faster_without_const_reference;

  template <typename T>
  using has_is_faster_without_const_reference_defined = fst::is_detected<is_faster_without_const_reference_t, T>;

  template <typename T>
  inline constexpr bool get_is_faster_without_const_reference() {
    if constexpr (detail::has_is_faster_without_const_reference_defined<T>::value) {
      constexpr bool value
          = std::is_same<typename T::is_faster_without_const_reference, faster_without_const_reference_tag>::value;
      static_assert(value,
          "T has implemented is_faster_without_const_reference with the wrong tag. Use 'using "
          "is_faster_without_const_reference = fst::dtraits::faster_without_const_reference_tag;");
      return value;
    }
    else {
      return std::is_fundamental_v<
                 T> || (std::is_trivially_copy_constructible_v<T> && sizeof(T) <= fst::config::bitness_byte_size);
    }
  }
} // namespace detail.

template <typename T>
struct is_integer_convertible {
  static constexpr bool value = std::is_unsigned_v<T> || std::is_signed_v<T>;
};

template <typename T>
inline constexpr bool is_integer_convertible_v = is_integer_convertible<T>::value;

template <typename T>
struct is_integer_convertiable_but_not_bool {
  static constexpr bool value = fst::is_integer_convertible_v<T> && !std::is_same_v<T, bool>;
};

template <typename T>
inline constexpr bool is_integer_convertiable_but_not_bool_v = is_integer_convertiable_but_not_bool<T>::value;

template <typename T>
struct is_faster_without_const_reference {
  static constexpr bool value = detail::get_is_faster_without_const_reference<T>();
};

///
/// Fast vector resize.
///
/// Ensures your class is optimized to be stored in a vector.
/// Checks whether it is trivially destructible (skips destructor call on resize)
/// and trivially copy constructible (use memcpy on resive).
/// If not, falls back to ensure your class is noexcept move constructible.
///
/// In vector::resize(size), if T's move constructor is not noexcept and T is not
/// CopyInsertable into *this, vector will use the throwing move constructor. If it throws,
/// the guarantee is waived and the effects are unspecified.
///
template <typename T>
struct is_fast_vector_resize {
  static constexpr bool value = is_memcopyable<T>::value || std::is_nothrow_move_constructible_v<T>;
};
} // namespace fst.
