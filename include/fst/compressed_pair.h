/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// The compressed pair class is very similar to std::pair, but if either of the
// template arguments are empty classes, then the "empty base-class optimization"
// is applied to compress the size of the pair.
//
// The design for compressed_pair here is very similar to that found in template
// metaprogramming libraries such as Boost, GCC, and Metrowerks, given that
// these libraries have established this interface as a defacto standard for
// solving this problem. Also, these are described in various books on the
// topic of template metaprogramming, such as "Modern C++ Design".
//
// template <typename T1, typename T2>
// class compressed_pair
// {
// public:
//     typedef T1                                                 first_type;
//     typedef T2                                                 second_type;
//     typedef typename call_traits<first_type>::param_type       first_param_type;
//     typedef typename call_traits<second_type>::param_type      second_param_type;
//     typedef typename call_traits<first_type>::reference        first_reference;
//     typedef typename call_traits<second_type>::reference       second_reference;
//     typedef typename call_traits<first_type>::const_reference  first_const_reference;
//     typedef typename call_traits<second_type>::const_reference second_const_reference;
//
//     compressed_pair() : base() {}
//     compressed_pair(first_param_type x, second_param_type y);
//     explicit compressed_pair(first_param_type x);
//     explicit compressed_pair(second_param_type y);
//
//     compressed_pair& operator=(const compressed_pair&);
//
//     first_reference       first();
//     first_const_reference first() const;
//
//     second_reference       second();
//     second_const_reference second() const;
//
//     void swap(compressed_pair& y);
// };
//
// The two members of the pair can be accessed using the member functions first()
// and second(). Note that not all member functions can be instantiated for all
// template parameter types. In particular compressed_pair can be instantiated for
// reference and array types, however in these cases the range of constructors that
// can be used are limited. If types T1 and T2 are the same type, then there is
// only one version of the single-argument constructor, and this constructor
// initialises both values in the pair to the passed value.
//
// Note that compressed_pair can not be instantiated if either of the template
// arguments is a union type, unless there is compiler support for is_union,
// or if is_union is specialised for the union type.
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include <fst/traits>

//#if defined(_MSC_VER) && (_MSC_VER >= 1900)  // VS2015 or later
//	EA_DISABLE_VC_WARNING(4626 5027) // warning C4626: 'eastl::compressed_pair_imp<T1,T2,0>': assignment operator was
// implicitly defined as deleted because a base class assignment operator is inaccessible or deleted #endif

namespace fst {
template <typename T1, typename T2>
class compressed_pair;

namespace detail {
  template <typename T1, typename T2>
  inline constexpr std::size_t get_compressed_pair_version() {
    constexpr bool same = std::is_same_v<std::remove_cv_t<T1>, std::remove_cv_t<T2>>;
    constexpr bool t1_empty = std::is_empty<T1>::value;
    constexpr bool t2_empty = std::is_empty<T2>::value;

    if constexpr (same) {
      return t1_empty ? 4 : 5;
    }
    else {
      return t1_empty ? (t2_empty ? 3 : 1) : (t2_empty ? 2 : 0);
    }
  }

  template <typename T1, typename T2, int version>
  class compressed_pair_imp;

  // Derive from neither.
  template <typename T1, typename T2>
  class compressed_pair_imp<T1, T2, 0> {
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

    compressed_pair_imp(first_param_type x)
        : _first(x) {}

    compressed_pair_imp(second_param_type y)
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
  class compressed_pair_imp<T1, T2, 1> : private T1 {
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

    compressed_pair_imp(first_param_type x)
        : first_type(x) {}

    compressed_pair_imp(second_param_type y)
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
  class compressed_pair_imp<T1, T2, 2> : private T2 {
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

    compressed_pair_imp(first_param_type x)
        : _first(x) {}

    compressed_pair_imp(second_param_type y)
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
  class compressed_pair_imp<T1, T2, 3> : private T1, private T2 {
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

    compressed_pair_imp(first_param_type x)
        : first_type(x) {}

    compressed_pair_imp(second_param_type y)
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
  class compressed_pair_imp<T1, T2, 4> : private T1 {
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

    compressed_pair_imp(first_param_type x)
        : first_type(x) {}

    inline first_reference first() { return *this; }
    inline first_const_reference first() const { return *this; }
    inline second_reference second() { return *this; }
    inline second_const_reference second() const { return *this; }
    inline void swap(compressed_pair<T1, T2>&) {}
  };

  // T1 == T2 and are not empty.
  template <typename T1, typename T2>
  class compressed_pair_imp<T1, T2, 5> {
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

    inline compressed_pair_imp(first_param_type x)
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

  compressed_pair() = default;

  compressed_pair(first_param_type x, second_param_type y)
      : base(x, y) {}

  explicit compressed_pair(first_param_type x)
      : base(x) {}

  explicit compressed_pair(second_param_type y)
      : base(y) {}

  inline first_reference first() { return base::first(); }
  inline first_const_reference first() const { return base::first(); }
  inline second_reference second() { return base::second(); }
  inline second_const_reference second() const { return base::second(); }
  inline void swap(compressed_pair& y) { base::swap(y); }
};

// Partial specialisation for case where T1 == T2.
template <typename T>
class compressed_pair<T, T> : private detail::compressed_pair_base<T, T> {
private:
  using base = detail::compressed_pair_base<T, T>;

public:
  using first_type = typename base::first_type;
  using second_type = typename base::second_type;
  using first_param_type = typename base::first_param_type;
  using second_param_type = typename base::second_param_type;
  using first_reference = typename base::first_reference;
  using second_reference = typename base::second_reference;
  using first_const_reference = typename base::first_const_reference;
  using second_const_reference = typename base::second_const_reference;

  compressed_pair() = default;

  compressed_pair(first_param_type x, second_param_type y)
      : base(x, y) {}

  explicit compressed_pair(first_param_type x)
      : base(x) {}

  inline first_reference first() { return base::first(); }
  inline first_const_reference first() const { return base::first(); }
  inline second_reference second() { return base::second(); }
  inline second_const_reference second() const { return base::second(); }
  inline void swap(compressed_pair<T, T>& y) { base::swap(y); }
};

template <typename T1, typename T2>
inline void swap(compressed_pair<T1, T2>& x, compressed_pair<T1, T2>& y) {
  x.swap(y);
}
} // namespace fst

//#if defined(_MSC_VER) && (_MSC_VER >= 1900)  // VS2015 or later
//	EA_RESTORE_VC_WARNING()
//#endif
