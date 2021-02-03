#pragma once

#include "fst/assert.h"
#include "fst/traits.h"

#include <cstddef>
#include <memory>
#include <utility>
#include <iterator>
#include <algorithm>

namespace fst {
template <typename _Tp, std::size_t _Size, bool _IsHeapBuffer = false>
class buffer {
public:
  using value_type = _Tp;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const pointer;
  using size_type = std::size_t;

  using byte_type = std::byte;

  static constexpr size_type maximum_size = _Size;
  static_assert(maximum_size > 0, "buffer size must be greater than 0");

  inline constexpr reference operator[](size_type n) { return data()[n]; }
  inline constexpr const_reference operator[](size_type n) const { return data()[n]; }

  inline pointer data() { return static_cast<pointer>(static_cast<void*>(_data)); }
  inline const_pointer data() const { return (const_pointer)((const void*)(_data)); }

private:
  byte_type _data[maximum_size * sizeof(value_type)];
};

template <typename _Tp, std::size_t _Size>
class buffer<_Tp, _Size, true> {
public:
  using value_type = _Tp;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const pointer;
  using size_type = std::size_t;

  using byte_type = std::byte;

  static constexpr size_type maximum_size = _Size;
  static_assert(maximum_size > 0, "buffer size must be greater than 0");

  inline constexpr reference operator[](size_type n) { return data()[n]; }
  inline constexpr const_reference operator[](size_type n) const { return data()[n]; }

  inline pointer data() { return static_cast<pointer>(static_cast<void*>(_data.get())); }
  inline const_pointer data() const { return static_cast<const_pointer>(static_cast<void*>(_data.get())); }

private:
  std::unique_ptr<byte_type[]> _data
      = { std::unique_ptr<byte_type[]>(new byte_type[maximum_size * sizeof(value_type)]) };
};

template <typename _Tp, std::size_t _Size, bool _IsHeapBuffer = false>
class fixed_vector {
public:
  using value_type = _Tp;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const pointer;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  static constexpr size_type maximum_size = _Size;
  static_assert(maximum_size > 0, "buffer size must be greater than 0");

  static constexpr bool is_heap_buffer = _IsHeapBuffer;
  using buffer_type = buffer<value_type, maximum_size, is_heap_buffer>;

  static constexpr bool is_trivial = std::is_trivial<value_type>::value;

private:
  using is_default_constructible = traits::bool_constant<std::is_default_constructible<value_type>::value>;
  using is_copy_constructible = traits::bool_constant<std::is_copy_constructible<value_type>::value>;
  using is_not_copy_constructible = traits::bool_constant<!std::is_copy_constructible<value_type>::value>;
  using is_move_constructible = traits::bool_constant<std::is_move_constructible<value_type>::value>;
  using is_heap_buffer_condition = traits::bool_constant<is_heap_buffer>;
  using is_not_heap_buffer_condition = traits::bool_constant<!is_heap_buffer>;

  template <bool _Dummy, class _D = traits::dependent_type_condition<_Dummy, is_default_constructible>>
  using enable_if_is_default_constructible = traits::enable_if_same<_Dummy, _D>;

  template <bool _Dummy, class _D = traits::dependent_type_condition<_Dummy, is_copy_constructible>>
  using enable_if_is_copy_constructible = traits::enable_if_same<_Dummy, _D>;

  template <bool _Dummy, class _D = traits::dependent_type_condition<_Dummy, is_not_copy_constructible>>
  using enable_if_is_not_copy_constructible = traits::enable_if_same<_Dummy, _D>;

  template <bool _Dummy, class _D = traits::dependent_type_condition<_Dummy, is_move_constructible>>
  using enable_if_is_move_constructible = traits::enable_if_same<_Dummy, _D>;

  template <bool _Dummy, class _D = traits::dependent_type_condition<_Dummy, is_heap_buffer_condition>>
  using enable_if_is_heap_buffer = traits::enable_if_same<_Dummy, _D>;

  template <bool _Dummy, class _D = traits::dependent_type_condition<_Dummy, is_not_heap_buffer_condition>>
  using enable_if_is_not_heap_buffer = traits::enable_if_same<_Dummy, _D>;

public:
  fixed_vector() = default;

  fixed_vector(size_type size) { resize(size); }

  fixed_vector(size_type size, const_reference value) { resize(size, value); }

  fixed_vector(const fixed_vector& fv) {
    static_assert(std::is_copy_constructible<value_type>::value, "value_type is not copy constructible");
    for (size_type i = 0; i < fv.size(); i++) {
      push_back(fv[i]);
    }
  }

  fixed_vector(fixed_vector&& fv) {
    static_assert(is_heap_buffer || is_trivial || is_move_constructible::value || is_copy_constructible::value,
        "value_type is not move constructible");

    if constexpr (is_heap_buffer) {
      _data = std::move(fv._data);
      _size = fv._size;
      fv._size = 0;
    }
    else if constexpr (is_trivial) {
      _size = fv.size();
      for (size_type i = 0; i < _size; i++) {
        _data[i] = fv[i];
      }
      fv._size = 0;
    }
    else if constexpr (is_move_constructible::value) {
      for (size_type i = 0; i < fv.size(); i++) {
        push_back(std::move(fv[i]));
      }
      fv._size = 0;
    }
    else if constexpr (is_copy_constructible::value) {
      for (size_type i = 0; i < fv.size(); i++) {
        push_back(fv[i]);
      }
      fv._size = 0;
    }
  }

  ~fixed_vector() {
    if constexpr (!is_trivial) {
      for (size_type i = 0; i < _size; i++) {
        _data[i].~value_type();
      }
    }

    _size = 0;
  }

  fixed_vector& operator=(const fixed_vector& fv) {
    static_assert(std::is_copy_constructible<value_type>::value, "value_type is not copy constructible");

    if constexpr (is_trivial) {
      for (size_type i = 0; i < fv.size(); i++) {
        _data[i] = fv[i];
      }
      _size = fv.size();
    }
    else {
      for (size_type i = 0; i < size(); i++) {
        _data[i].~value_type();
      }

      for (size_type i = 0; i < fv.size(); i++) {
        new (&_data[i]) value_type(fv[i]);
      }
      _size = fv.size();
    }

    return *this;
  }

  // Iterators.
  inline iterator begin() noexcept { return iterator(_data.data()); }
  inline const_iterator begin() const noexcept { return const_iterator(_data.data()); }

  inline iterator end() noexcept { return iterator(_data.data() + _size); }
  inline const_iterator end() const noexcept { return const_iterator(_data.data() + _size); }

  inline reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  inline const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }

  inline reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  inline const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

  inline const_iterator cbegin() const noexcept { return begin(); }
  inline const_iterator cend() const noexcept { return end(); }

  inline const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  inline const_reverse_iterator crend() const noexcept { return rend(); }

  // Capacity.
  [[nodiscard]] inline size_type size() const noexcept { return _size; }
  [[nodiscard]] inline constexpr size_type capacity() const noexcept { return maximum_size; }
  [[nodiscard]] inline bool empty() const noexcept { return _size == 0; }

  // Element access.
  inline reference operator[](size_type n) {
    fst_assert(n < maximum_size, "Index out of bounds");
    return _data[n];
  }
  inline const_reference operator[](size_type n) const {
    fst_assert(n < maximum_size, "Index out of bounds");
    return _data[n];
  }

  inline reference at(size_type n) {
    fst_assert(n < maximum_size, "Index out of bounds");
    return _data[n];
  }

  inline const_reference at(size_type n) const {
    fst_assert(n < maximum_size, "Index out of bounds");
    return _data[n];
  }

  inline reference front() { return _data[0]; }
  inline const_reference front() const { return _data[0]; }

  inline reference back() { return _data[_size - 1]; }
  inline const_reference back() const { return _data[_size - 1]; }

  inline pointer data() noexcept { return _data.data(); }
  inline const_pointer data() const noexcept { return _data.data(); }

  template <bool _Dummy = true, class = enable_if_is_copy_constructible<_Dummy>>
  void push_back(const_reference value) {
    fst_assert(_size < maximum_size, "Out of bounds push_back");

    if constexpr (is_trivial) {
      _data[_size++] = value;
    }
    else {
      new (&_data[_size++]) value_type(value);
    }
  }

  template <bool _Dummy = true, class = enable_if_is_move_constructible<_Dummy>>
  void push_back(value_type&& value) {
    fst_assert(_size < maximum_size, "Out of bounds push_back");

    if constexpr (is_trivial) {
      _data[_size++] = std::move(value);
    }
    else {
      new (&_data[_size++]) value_type(std::move(value));
    }
  }

  template <typename... Args>
  void emplace_back(Args&&... args) {
    fst_assert(_size < maximum_size, "Out of bounds emplace_back");
    if constexpr (is_trivial) {
      _data[_size++] = value_type(std::forward<Args>(args)...);
    }
    else {
      new (&_data[_size++]) value_type(std::forward<Args>(args)...);
    }
  }

  inline void pop_back() {
    fst_assert(_size, "pop_back when empty");
    if constexpr (!is_trivial) {
      _data[_size - 1].~value_type();
    }
    _size--;
  }

  template <bool _Dummy = true, class = enable_if_is_default_constructible<_Dummy>>
  void resize(size_type size) {
    fst_assert(size <= maximum_size, "Out of bounds resize size");

    if (size == _size) {
      return;
    }

    if (size < _size) {
      if constexpr (!is_trivial) {
        for (size_type i = size; i < _size; i++) {
          _data[i].~value_type();
        }
      }

      _size = size;
      return;
    }

    if constexpr (!is_trivial) {
      for (size_type i = _size; i < size; i++) {
        new (&_data[i]) value_type();
      }
    }

    _size = size;
  }

  template <bool _Dummy = true, class = enable_if_is_copy_constructible<_Dummy>>
  void resize(size_type size, const_reference value) {
    fst_assert(size <= maximum_size, "Out of bounds resize size");

    if (size == _size) {
      return;
    }

    if (size < _size) {
      if constexpr (!is_trivial) {
        for (size_type i = size; i < _size; i++) {
          _data[i].~value_type();
        }
      }

      _size = size;
      return;
    }

    if constexpr (!is_trivial) {
      for (size_type i = _size; i < size; i++) {
        new (&_data[i]) value_type(value);
      }
    }

    _size = size;
  }

  void erase(size_type index) {
    if (index >= _size) {
      return;
    }

    if (_size == 0) {
      return;
    }

    for (size_type i = index; i < _size - 1; i++) {
      _data[i] = std::move(_data[i + 1]);
    }

    if constexpr (!is_trivial) {
      _data[_size - 1].~value_type();
    }

    _size--;
  }

  void erase(iterator it) { erase((size_type)std::distance(begin(), it)); }

  void clear() { resize(0); }

private:
  buffer_type _data;
  size_type _size = 0;
};
} // namespace fst.