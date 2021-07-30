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
/// https://github.com/WG21-SG14/SG14/blob/master/SG14/slot_map.h
///
#pragma once
#include <fst/assert>
#include <type_traits>
#include <utility>
#include <vector>

#if __FST_HAS_EXCEPTIONS__
#include <stdexcept>
#define FST_SLOT_MAP_THROW_OUT_OF_RANGE_EXCEPTION() throw std::out_of_range("slot_map::at")
#else
#define FST_SLOT_MAP_THROW_OUT_OF_RANGE_EXCEPTION() fst_error("slot_map::at : Out of range.")
#endif // __FST_HAS_EXCEPTIONS__.

namespace fst {
namespace slot_map_detail {
  template <size_t I>
  struct priority_tag : public priority_tag<I - 1> {};
  template <>
  struct priority_tag<0> {};

  template <class Ctr, class SizeType>
  inline auto reserve_if_possible(Ctr&, SizeType, priority_tag<0>) -> void {}

  template <class Ctr, class SizeType>
  inline auto reserve_if_possible(Ctr& ctr, SizeType n, priority_tag<1>) -> decltype(void(ctr.reserve(n))) {
    ctr.reserve(n);
  }

  template <class Ctr, class SizeType>
  inline void reserve_if_possible(Ctr& ctr, const SizeType& n) {
    slot_map_detail::reserve_if_possible(ctr, n, priority_tag<1>{});
  }
} // namespace slot_map_detail

template <typename _IndexType, typename _GenType>
struct slot_map_key {
  static_assert(std::is_integral_v<_IndexType>, "_IndexType must be an integral type.");

  using index_type = _IndexType;
  using generation_type = _GenType;
  index_type idx;
  generation_type gen;

  template <typename T, typename std::enable_if<std::is_convertible_v<T, index_type>>::type* = nullptr>
  inline void set_index(T i) {
    idx = static_cast<index_type>(i);
  }

  inline void set_generation(generation_type g) { gen = g; }
  inline index_type get_index() const { return idx; }
  inline generation_type get_generation() const { return gen; }

  inline void increment_generation() { ++gen; }
};

template <class T, class Key = slot_map_key<unsigned int, unsigned int>,
    template <class...> class Container = std::vector>
class slot_map {
  using slot_iterator = typename Container<Key>::iterator;

public:
  using key_type = Key;
  using mapped_type = T;
  using key_index_type = typename key_type::index_type;
  using key_generation_type = typename key_type::generation_type;
  using container_type = Container<mapped_type>;
  using reference = typename container_type::reference;
  using const_reference = typename container_type::const_reference;
  using pointer = typename container_type::pointer;
  using const_pointer = typename container_type::const_pointer;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;
  using reverse_iterator = typename container_type::reverse_iterator;
  using const_reverse_iterator = typename container_type::const_reverse_iterator;
  using size_type = typename container_type::size_type;
  using value_type = typename container_type::value_type;

  static_assert(std::is_same<value_type, mapped_type>::value, "Container<T>::value_type must be identical to T");

  constexpr slot_map() = default;
  constexpr slot_map(const slot_map&) = default;
  constexpr slot_map(slot_map&&) = default;
  constexpr slot_map& operator=(const slot_map&) = default;
  constexpr slot_map& operator=(slot_map&&) = default;
  ~slot_map() = default;

  // The at() functions have both generation counter checking
  // and bounds checking, and throw if either check fails.
  // O(1) time and space complexity.
  //
  constexpr reference at(const key_type& key) {
    auto value_iter = find(key);
    if (value_iter == end()) {
      FST_SLOT_MAP_THROW_OUT_OF_RANGE_EXCEPTION();
    }
    return *value_iter;
  }

  constexpr const_reference at(const key_type& key) const {
    auto value_iter = find(key);
    if (value_iter == end()) {
      FST_SLOT_MAP_THROW_OUT_OF_RANGE_EXCEPTION();
    }
    return *value_iter;
  }

  // The bracket operator[] has a generation counter check.
  // If the check fails it is undefined behavior.
  // O(1) time and space complexity.
  //
  constexpr reference operator[](const key_type& key) { return *find_unchecked(key); }
  constexpr const_reference operator[](const key_type& key) const { return *find_unchecked(key); }

  // The find() functions have generation counter checking.
  // If the check fails, the result of end() is returned.
  // O(1) time and space complexity.
  //
  constexpr iterator find(const key_type& key) {
    auto slot_index = key.get_index();
    if (slot_index >= slots_.size()) {
      return end();
    }

    auto slot_iter = std::next(slots_.begin(), slot_index);
    if (slot_iter->get_generation() != key.get_generation()) {
      return end();
    }

    return std::next(values_.begin(), slot_iter->get_index());
  }

  constexpr const_iterator find(const key_type& key) const {
    auto slot_index = key.get_index();
    if (slot_index >= slots_.size()) {
      return end();
    }

    auto slot_iter = std::next(slots_.begin(), slot_index);
    if (slot_iter->get_generation() != key.get_generation()) {
      return end();
    }

    return std::next(values_.begin(), slot_iter->get_index());
  }

  // The find_unchecked() functions perform no checks of any kind.
  // O(1) time and space complexity.
  //
  constexpr iterator find_unchecked(const key_type& key) {
    auto slot_iter = std::next(slots_.begin(), key.get_index());
    return std::next(values_.begin(), slot_iter->get_index());
  }

  constexpr const_iterator find_unchecked(const key_type& key) const {
    auto slot_iter = std::next(slots_.begin(), key.get_index());
    return std::next(values_.begin(), slot_iter->get_index());
  }

  //
  // All begin() and end() variations have O(1) time and space complexity.
  //
  constexpr iterator begin() { return values_.begin(); }
  constexpr iterator end() { return values_.end(); }
  constexpr const_iterator begin() const { return values_.begin(); }
  constexpr const_iterator end() const { return values_.end(); }
  constexpr const_iterator cbegin() const { return values_.begin(); }
  constexpr const_iterator cend() const { return values_.end(); }
  constexpr reverse_iterator rbegin() { return values_.rbegin(); }
  constexpr reverse_iterator rend() { return values_.rend(); }
  constexpr const_reverse_iterator rbegin() const { return values_.rbegin(); }
  constexpr const_reverse_iterator rend() const { return values_.rend(); }
  constexpr const_reverse_iterator crbegin() const { return values_.rbegin(); }
  constexpr const_reverse_iterator crend() const { return values_.rend(); }

  // Functions for checking the size and capacity of the adapted container
  // have the same complexity as the adapted container.
  // reserve(n) has the complexity of the adapted container, and uses
  // additional time which is linear on the increase in size.
  // This is caused by adding the new slots to the free list.
  //
  constexpr bool empty() const { return values_.size() == 0; }
  constexpr size_type size() const { return values_.size(); }
  // constexpr size_type max_size() const; TODO, NO SEMANTICS

  constexpr void reserve(size_type n) {
    slot_map_detail::reserve_if_possible(values_, n);
    slot_map_detail::reserve_if_possible(reverse_map_, n);
    reserve_slots(n);
  }

  template <class C = Container<T>, class = decltype(std::declval<const C&>().capacity())>
  constexpr size_type capacity() const {
    return values_.capacity();
  }

  // Functions for accessing and modifying the size of the slots container.
  // These are beneficial as allocating more slots than values will cause the
  // generation counter increases to be more evenly distributed across the slots.
  //
  constexpr void reserve_slots(size_type n) {
    slot_map_detail::reserve_if_possible(slots_, n);
    key_index_type original_num_slots = static_cast<key_index_type>(slots_.size());

    if (original_num_slots < n) {
      slots_.emplace_back(key_type{ next_available_slot_index_, key_generation_type{} });
      key_index_type last_new_slot = original_num_slots;
      --n;

      while (last_new_slot != n) {
        slots_.emplace_back(key_type{ last_new_slot, key_generation_type{} });
        ++last_new_slot;
      }

      next_available_slot_index_ = last_new_slot;
    }
  }

  constexpr size_type slot_count() const { return slots_.size(); }

  // These operations have O(1) time and space complexity.
  // When size() == capacity() an allocation is required
  // which has O(n) time and space complexity.
  //
  constexpr key_type insert(const mapped_type& value) { return emplace(value); }
  constexpr key_type insert(mapped_type&& value) { return emplace(std::move(value)); }

  template <class... Args>
  constexpr key_type emplace(Args&&... args) {
    auto value_pos = values_.size();
    values_.emplace_back(std::forward<Args>(args)...);
    reverse_map_.emplace_back(next_available_slot_index_);

    if (next_available_slot_index_ == slots_.size()) {
      auto idx = next_available_slot_index_;
      ++idx;

      // Make a new slot.
      slots_.emplace_back(key_type{ idx, key_generation_type{} });
      last_available_slot_index_ = idx;
    }

    auto slot_iter = std::next(slots_.begin(), next_available_slot_index_);
    if (next_available_slot_index_ == last_available_slot_index_) {
      next_available_slot_index_ = static_cast<key_index_type>(slots_.size());
      last_available_slot_index_ = next_available_slot_index_;
    }
    else {
      next_available_slot_index_ = slot_iter->get_index();
    }

    slot_iter->set_index(value_pos);
    key_type result = *slot_iter;
    result.set_index(std::distance(slots_.begin(), slot_iter));
    return result;
  }

  //
  // Each erase() version has an O(1) time complexity per value
  // and O(1) space complexity.
  //
  constexpr iterator erase(iterator pos) { return erase(const_iterator(pos)); }

  constexpr iterator erase(iterator first, iterator last) { return erase(const_iterator(first), const_iterator(last)); }

  constexpr iterator erase(const_iterator pos) { return erase_slot_iter(slot_iter_from_value_iter(pos)); }

  constexpr iterator erase(const_iterator first, const_iterator last) {
    // Must use indexes, not iterators, because Container iterators might be invalidated by pop_back.
    auto first_index = std::distance(cbegin(), first);
    auto last_index = std::distance(cbegin(), last);

    while (last_index != first_index) {
      --last_index;
      erase(std::next(cbegin(), last_index));
    }
    return std::next(begin(), first_index);
  }

  constexpr size_type erase(const key_type& key) {
    auto iter = find(key);
    if (iter == end()) {
      return 0;
    }
    erase(iter);
    return 1;
  }

  /// clear() has O(n) time complexity and O(1) space complexity.
  /// It also has semantics differing from erase(begin(), end())
  /// in that it also resets the generation counter of every slot
  /// and rebuilds the free list.
  constexpr void clear() {
    // This resets the generation counters, which "undefined-behavior-izes" at() and find() for the old keys.
    slots_.clear();
    values_.clear();
    reverse_map_.clear();
    next_available_slot_index_ = key_index_type{};
    last_available_slot_index_ = key_index_type{};
  }

  /// swap is not mentioned in P0661r1 but it should be.
  constexpr void swap(slot_map& rhs) {
    std::swap(slots_, rhs.slots_);
    std::swap(values_, rhs.values_);
    std::swap(reverse_map_, rhs.reverse_map_);
    std::swap(next_available_slot_index_, rhs.next_available_slot_index_);
    std::swap(last_available_slot_index_, rhs.last_available_slot_index_);
  }

protected:
  /// These accessors are not part of P0661R2 but are "modernized" versions
  /// of the protected interface of std::priority_queue, std::stack, etc.
  constexpr Container<mapped_type>& c() & noexcept { return values_; }
  constexpr const Container<mapped_type>& c() const& noexcept { return values_; }
  constexpr Container<mapped_type>&& c() && noexcept { return std::move(values_); }
  constexpr const Container<mapped_type>&& c() const&& noexcept { return std::move(values_); }

private:
  constexpr slot_iterator slot_iter_from_value_iter(const_iterator value_iter) {
    auto value_index = std::distance(const_iterator(values_.begin()), value_iter);
    auto slot_index = *std::next(reverse_map_.begin(), value_index);
    return std::next(slots_.begin(), slot_index);
  }

  constexpr iterator erase_slot_iter(slot_iterator slot_iter) {
    auto slot_index = std::distance(slots_.begin(), slot_iter);
    auto value_index = slot_iter->get_index();
    auto value_iter = std::next(values_.begin(), value_index);
    auto value_back_iter = std::prev(values_.end());

    if (value_iter != value_back_iter) {
      auto slot_back_iter = slot_iter_from_value_iter(value_back_iter);
      *value_iter = std::move(*value_back_iter);
      slot_back_iter->set_index(value_index);
      auto reverse_map_iter = std::next(reverse_map_.begin(), value_index);
      *reverse_map_iter = static_cast<key_index_type>(std::distance(slots_.begin(), slot_back_iter));
    }

    values_.pop_back();
    reverse_map_.pop_back();

    // Expire this key.
    if (next_available_slot_index_ == slots_.size()) {
      next_available_slot_index_ = static_cast<key_index_type>(slot_index);
      last_available_slot_index_ = static_cast<key_index_type>(slot_index);
    }
    else {
      auto last_slot_iter = std::next(slots_.begin(), last_available_slot_index_);
      last_slot_iter->set_index(slot_index);
      last_available_slot_index_ = static_cast<key_index_type>(slot_index);
    }

    slot_iter->increment_generation();
    return std::next(values_.begin(), value_index);
  }

  // high_water_mark() entries.
  Container<key_type> slots_;

  // exactly size() entries.
  Container<key_index_type> reverse_map_;

  // exactly size() entries.
  Container<mapped_type> values_;

  key_index_type next_available_slot_index_{};
  key_index_type last_available_slot_index_{};

  // Class invariant:
  // Either next_available_slot_index_ == last_available_slot_index_ == slots_.size(),
  // or else 0 <= next_available_slot_index_ < slots_.size() and the "key" of that slot
  // entry points to the subsequent available slot, and so on, until reaching
  // last_available_slot_index_ (which might equal next_available_slot_index_ if there
  // is only one available slot at the moment).
};

template <class T, class Key, template <class...> class Container>
constexpr void swap(slot_map<T, Key, Container>& lhs, slot_map<T, Key, Container>& rhs) {
  lhs.swap(rhs);
}
} // namespace fst

#undef FST_SLOT_MAP_THROW_OUT_OF_RANGE_EXCEPTION
