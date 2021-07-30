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
#include <fst/config>
#include <fst/ui/point>
#include <fst/ui/size>
#include <fst/util>

namespace fst::ui {
template <typename _Tp>
class t_rect {
public:
  using value_type = _Tp;
  static_assert(std::is_arithmetic<value_type>::value, "value_type is not arithmetic");
  using size_type = t_size<value_type>;
  using point_type = t_point<value_type>;

#if __FST_CLANG__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#elif __FST_GCC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#elif __FST_MSVC__
#pragma warning(push)
#pragma warning(disable : 4201)
#endif

  union {
    point_type position;
    struct {
      value_type x, y;
    };
  };

  union {
    size_type size;
    struct {
      value_type width, height;
    };
  };

#if __FST_CLANG__
#pragma clang diagnostic pop
#elif __FST_GCC__
#pragma GCC diagnostic pop
#elif __FST_MSVC__
#pragma warning(pop)
#endif

  inline constexpr t_rect()
      : position()
      , size() {}

  inline constexpr t_rect(const point_type& pt_, const size_type& size_)
      : position(pt_)
      , size(size_) {}

  inline constexpr t_rect(value_type x, value_type y, value_type w, value_type h)
      : position(x, y)
      , size(w, h) {}

  inline constexpr t_rect(value_type x, value_type y, const size_type& size_)
      : position(x, y)
      , size(size_) {}

  inline constexpr t_rect(const t_rect& r)
      : position(r.position)
      , size(r.size) {}

  template <typename T>
  inline constexpr t_rect(const t_rect<T>& r)
      : position(r.position.template cast<value_type>())
      , size(r.size.template cast<value_type>()) {}

  inline constexpr t_rect& operator=(const t_rect& r) {
    position = r.position;
    size = r.size;
    return *this;
  }

  inline constexpr static t_rect from_bottom_left(value_type x, value_type y, value_type w, value_type h) {
    return t_rect(x, y - h, w, h);
  }

  template <typename P>
  inline constexpr t_rect<P> cast() const {
    return t_rect<P>(position.template cast<P>(), size.template cast<P>());
  }

  inline constexpr t_rect operator+(const t_rect& r) const {
    return t_rect(
        position.x + r.position.x, position.y + r.position.y, size.width + r.size.width, size.height + r.size.height);
  }

  inline constexpr t_rect operator-(const t_rect& r) const {
    return t_rect(
        position.x - r.position.x, position.y - r.position.y, size.width - r.size.width, size.height - r.size.height);
  }

  inline constexpr bool operator==(const t_rect& r) const { return position == r.position && size == r.size; }
  inline constexpr bool operator!=(const t_rect& r) const { return !operator==(r); }

  inline constexpr value_type left() const { return position.x; }
  inline constexpr value_type right() const { return position.x + size.width; }
  inline constexpr value_type top() const { return position.y; }

  inline constexpr value_type bottom() const { return position.y + size.height; }

  inline constexpr bool intersect(const t_rect& r) const {
    return left() > r.right() || r.left() > right() || top() < r.bottom() || r.top() < bottom();
  }

  template <typename _PointValueType>
  inline constexpr bool contains(const t_point<_PointValueType>& pos) const {
    return pos.x >= position.x && pos.x <= position.x + size.width && pos.y >= position.y
        && pos.y <= position.y + size.height;
  }

  inline constexpr t_rect get_interior_rect(const point_type& point) const {
    return t_rect(position.x + point.x, position.y + point.y, size.width - (value_type)(2.0 * point.x),
        size.height - (value_type)(2.0 * point.y));
  }

  inline constexpr t_rect get_exterior_rect(const point_type& point) const {
    return t_rect(position.x - point.x, position.y - point.y, size.width + (value_type)(2.0 * point.x),
        size.height + (value_type)(2.0 * point.y));
  }

  inline constexpr void reduce(const point_type& point) {
    position.x += point.x;
    position.y += point.y;
    size.width -= (value_type)(2.0 * point.x);
    size.height -= (value_type)(2.0 * point.y);
  }

  inline constexpr t_rect reduced(const point_type& point) const {
    return t_rect(x + point.x, y + point.y, width - (value_type)(2.0 * point.x), height - (value_type)(2.0 * point.y));
  }

  inline constexpr t_rect reduced_from_left_percent(float v) const { return reduced_from_left(width * (float)v); }
  inline constexpr t_rect reduced_from_right_percent(float v) const { return reduced_from_right(width * (float)v); }

  inline constexpr t_rect reduced_from_left(value_type x) const {
    return t_rect(position.x + x, position.y, size.width - x, size.height);
  }
  inline constexpr t_rect reduced_from_right(value_type x) const {
    return t_rect(position.x, position.y, size.width - x, size.height);
  }
  inline constexpr t_rect reduced_from_top(value_type y) const {
    return t_rect(position.x, position.y + y, size.width, size.height - y);
  }
  inline constexpr t_rect reduced_from_bottom(value_type y) const {
    return t_rect(position.x, position.y, size.width, size.height - y);
  }

  inline constexpr point_type in_middle(const size_type& s) const {
    return point_type(x + (width - s.width) * 0.5, y + (height - s.height) * 0.5);
  }

  inline constexpr point_type top_left() const { return position; }
  inline constexpr point_type top_right(value_type dx = 0, value_type dy = 0) const {
    return point_type(position.x + size.width + dx, position.y + dy);
  }
  inline constexpr point_type bottom_left() const { return point_type(position.x, position.y + size.height); }
  inline constexpr point_type bottom_right() const {
    return point_type(position.x + size.width, position.y + size.height);
  }
  inline constexpr point_type middle() const {
    return point_type(position.x + size.width * 0.5, position.y + size.height * 0.5);
  }
  inline constexpr point_type next_left(value_type delta) const { return point_type(position.x, position.y - delta); }
  inline constexpr point_type next_right(value_type delta) const {
    return point_type(position.x + size.width + delta, position.y);
  }
  inline constexpr point_type next_down(value_type delta) const {
    return point_type(position.x, position.y + size.height + delta);
  }
  inline constexpr point_type next_up(value_type delta) const { return point_type(position.x, position.y - delta); }

  inline constexpr void clip_to(const t_rect& r) {
    x = fst::maximum(r.x, x);
    y = fst::maximum(r.y, y);
    width = fst::minimum(r.width, width);
    height = fst::minimum(r.height, height);
  }

  friend std::ostream& operator<<(std::ostream& stream, const t_rect& r) {
    stream << r.position.x << ", " << r.position.y << ", " << r.size.width << ", " << r.size.height;
    return stream;
  }
};

using rect = t_rect<int>;
using frect = t_rect<float>;
} // namespace fst::ui.
