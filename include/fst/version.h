// https://semver.org/
// https://github.com/Neargye/semver
#pragma once
#include "fst/config.h"
#include "fst/assert.h"
#include "fst/ascii.h"

#include <cstdint>
#include <iosfwd>
#include <limits>
#include <optional>
#include <string>
#include <array>

#if __has_include(<charconv>)
#include <charconv>
#else
#include <system_error>
#endif

#if __FST_HAS_EXCEPTIONS__
#include <stdexcept>
#define FST_VERSION_THROW_INVALID_ARG(msg) throw std::invalid_argument(msg)
#else
#define FST_VERSION_THROW_INVALID_ARG(msg) fst_error(msg)
#endif // __FST_HAS_EXCEPTIONS__

#if __FST_CLANG__
// Ignore warning: suggest braces around initialization of subobject 'return {first, std::errc::invalid_argument};'.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"
#endif

namespace fst {
enum struct version_tag : std::uint8_t { alpha, beta, rc, oem, none };

// clang-format off
inline constexpr std::array version_tag_strings = {
  std::string_view("alpha"),
  std::string_view("beta"),
  std::string_view("rc"),
  std::string_view("oem")
};
// clang-format on

inline constexpr version_tag to_version_tag(std::size_t index) {
  return index <= (std::size_t)version_tag::none ? (version_tag)index : version_tag::none;
}

#if __has_include(<charconv>)
struct from_chars_result : std::from_chars_result {
  FST_NODISCARD constexpr operator bool() const noexcept { return ec == std::errc{}; }
};

struct to_chars_result : std::to_chars_result {
  FST_NODISCARD constexpr operator bool() const noexcept { return ec == std::errc{}; }
};

#else
struct from_chars_result {
  const char* ptr;
  std::errc ec;
  FST_NODISCARD constexpr operator bool() const noexcept { return ec == std::errc{}; }
};

struct to_chars_result {
  char* ptr;
  std::errc ec;
  FST_NODISCARD constexpr operator bool() const noexcept { return ec == std::errc{}; }
};
#endif

struct version {
  std::uint8_t major = 0;
  std::uint8_t minor = 1;
  std::uint8_t patch = 0;
  version_tag tag = version_tag::none;
  std::uint8_t tag_number = 0;

  constexpr version(std::uint8_t mj, std::uint8_t mn, std::uint8_t pt, version_tag prt = version_tag::none,
      std::uint8_t prn = 0) noexcept;

  explicit constexpr version(std::string_view str);

  // https://semver.org/#how-should-i-deal-with-revisions-in-the-0yz-initial-development-phase
  constexpr version() = default;
  constexpr version(const version&) = default;
  constexpr version(version&&) = default;
  ~version() = default;

  version& operator=(const version&) = default;
  version& operator=(version&&) = default;

  FST_NODISCARD constexpr from_chars_result from_chars(const char* first, const char* last) noexcept;
  FST_NODISCARD constexpr to_chars_result to_chars(char* first, char* last) const noexcept;
  FST_NODISCARD constexpr bool from_string_noexcept(std::string_view str) noexcept;
  constexpr version& from_string(std::string_view str);

  FST_NODISCARD inline operator std::string() const;
  FST_NODISCARD inline std::string to_string() const;
  FST_NODISCARD constexpr std::uint8_t string_length() const noexcept;
  FST_NODISCARD constexpr int compare(const version& other) const noexcept;
};

// Max version string length = 3(<major>) + 1(.) + 3(<minor>) + 1(.) + 3(<patch>) + 1(-) + 5(<prerelease>) + 1(.) +
// 3(<prereleaseversion>) = 21.
inline constexpr auto max_version_string_length = std::size_t{ 21 };

namespace detail {
  // Min version string length = 1(<major>) + 1(.) + 1(<minor>) + 1(.) + 1(<patch>) = 5.
  inline constexpr auto min_version_string_length = 5;
  constexpr std::uint8_t length(std::uint8_t x) noexcept { return x < 10 ? 1 : (x < 100 ? 2 : 3); }

  constexpr std::uint8_t length(version_tag t) noexcept {
    return t == version_tag::none ? 0 : (std::uint8_t)version_tag_strings[(std::size_t)t].length();
  }

  constexpr bool equals(const char* first, const char* last, std::string_view str) noexcept {
    for (std::size_t i = 0; first != last && i < str.length(); ++i, ++first) {
      if (to_lower(*first) != to_lower(str[i])) {
        return false;
      }
    }

    return true;
  }

  constexpr char* to_chars(char* str, std::uint8_t x, bool dot = true) noexcept {
    do {
      *(--str) = static_cast<char>('0' + (x % 10));
      x /= 10;
    } while (x != 0);

    if (dot) {
      *(--str) = '.';
    }

    return str;
  }

  constexpr char* to_chars(char* str, version_tag t) noexcept {
    if (t == version_tag::none) {
      return str;
    }

    const std::string_view& p = version_tag_strings[(std::size_t)t];
    for (auto it = p.rbegin(); it != p.rend(); ++it) {
      *(--str) = *it;
    }
    *(--str) = '-';

    return str;
  }

  constexpr const char* from_chars(const char* first, const char* last, std::uint8_t& d) noexcept {
    if (first != last && is_digit(*first)) {
      std::int32_t t = 0;
      for (; first != last && is_digit(*first); ++first) {
        t = t * 10 + to_digit(*first);
      }
      if (t <= (std::numeric_limits<std::uint8_t>::max)()) {
        d = static_cast<std::uint8_t>(t);
        return first;
      }
    }

    return nullptr;
  }

  constexpr const char* from_chars(const char* first, const char* last, version_tag& p) noexcept {
    if (is_hyphen(*first)) {
      ++first;
    }

    for (std::size_t i = 0; i < version_tag_strings.size(); i++) {
      if (equals(first, last, version_tag_strings[i])) {
        p = to_version_tag(i);
        return first + version_tag_strings[i].length();
      }
    }

    return nullptr;
  }

  constexpr bool check_delimiter(const char* first, const char* last, char d) noexcept {
    return first != last && first != nullptr && *first == d;
  }

} // namespace detail

constexpr version::version(
    std::uint8_t mj, std::uint8_t mn, std::uint8_t pt, version_tag prt, std::uint8_t prn) noexcept
    : major{ mj }
    , minor{ mn }
    , patch{ pt }
    , tag{ prt }
    , tag_number{ prt == version_tag::none ? static_cast<std::uint8_t>(0) : prn } {}

constexpr version::version(std::string_view str)
    : version(0, 0, 0, version_tag::none, 0) {
  from_string(str);
}

FST_NODISCARD constexpr from_chars_result version::from_chars(const char* first, const char* last) noexcept {
  if (first == nullptr || last == nullptr || (last - first) < detail::min_version_string_length) {
    return { first, std::errc::invalid_argument };
  }

  auto next = first;
  if (next = detail::from_chars(next, last, major); detail::check_delimiter(next, last, '.')) {
    if (next = detail::from_chars(++next, last, minor); detail::check_delimiter(next, last, '.')) {
      if (next = detail::from_chars(++next, last, patch); next == last) {
        tag = version_tag::none;
        tag_number = 0;
        return { next, std::errc{} };
      }
      else if (detail::check_delimiter(next, last, '-')) {
        if (next = detail::from_chars(next, last, tag); next == last) {
          tag_number = 0;
          return { next, std::errc{} };
        }
        else if (detail::check_delimiter(next, last, '.')) {
          if (next = detail::from_chars(++next, last, tag_number); next == last) {
            return { next, std::errc{} };
          }
        }
      }
    }
  }

  return { first, std::errc::invalid_argument };
}

FST_NODISCARD constexpr to_chars_result version::to_chars(char* first, char* last) const noexcept {
  const auto length = string_length();
  if (first == nullptr || last == nullptr || (last - first) < length) {
    return { last, std::errc::value_too_large };
  }

  auto next = first + length;
  if (tag != version_tag::none) {
    if (tag_number != 0) {
      next = detail::to_chars(next, tag_number);
    }
    next = detail::to_chars(next, tag);
  }
  next = detail::to_chars(next, patch);
  next = detail::to_chars(next, minor);
  next = detail::to_chars(next, major, false);

  return { first + length, std::errc{} };
}

FST_NODISCARD constexpr bool version::from_string_noexcept(std::string_view str) noexcept {
  return from_chars(str.data(), str.data() + str.length());
}

constexpr version& version::from_string(std::string_view str) {
  if (!from_string_noexcept(str)) {
    FST_VERSION_THROW_INVALID_ARG("fst::version::from_string invalid version.");
  }

  return *this;
}

FST_NODISCARD inline version::operator std::string() const { return to_string(); }

FST_NODISCARD inline std::string version::to_string() const {
  auto str = std::string(string_length(), '\0');
  if (!to_chars(str.data(), str.data() + str.length())) {
    FST_VERSION_THROW_INVALID_ARG("fst::version::to_string invalid version.");
  }

  return str;
}

FST_NODISCARD constexpr std::uint8_t version::string_length() const noexcept {
  // (<major>) + 1(.) + (<minor>) + 1(.) + (<patch>)
  auto length = detail::length(major) + detail::length(minor) + detail::length(patch) + 2;
  if (tag != version_tag::none) {
    // + 1(-) + (<prerelease>)
    length += detail::length(tag) + 1;
    if (tag_number != 0) {
      // + 1(.) + (<prereleaseversion>)
      length += detail::length(tag_number) + 1;
    }
  }

  return static_cast<std::uint8_t>(length);
}

FST_NODISCARD constexpr int version::compare(const version& other) const noexcept {
  if (major != other.major) {
    return major - other.major;
  }

  if (minor != other.minor) {
    return minor - other.minor;
  }

  if (patch != other.patch) {
    return patch - other.patch;
  }

  if (tag != other.tag) {
    return static_cast<std::uint8_t>(tag) - static_cast<std::uint8_t>(other.tag);
  }

  if (tag_number != other.tag_number) {
    return tag_number - other.tag_number;
  }

  return 0;
}

FST_NODISCARD constexpr bool operator==(const version& lhs, const version& rhs) noexcept {
  return lhs.compare(rhs) == 0;
}

FST_NODISCARD constexpr bool operator!=(const version& lhs, const version& rhs) noexcept {
  return lhs.compare(rhs) != 0;
}

FST_NODISCARD constexpr bool operator>(const version& lhs, const version& rhs) noexcept { return lhs.compare(rhs) > 0; }

FST_NODISCARD constexpr bool operator>=(const version& lhs, const version& rhs) noexcept {
  return lhs.compare(rhs) >= 0;
}

FST_NODISCARD constexpr bool operator<(const version& lhs, const version& rhs) noexcept { return lhs.compare(rhs) < 0; }

FST_NODISCARD constexpr bool operator<=(const version& lhs, const version& rhs) noexcept {
  return lhs.compare(rhs) <= 0;
}

FST_NODISCARD constexpr version operator""_version(const char* str, std::size_t length) {
  return version{ std::string_view{ str, length } };
}

FST_NODISCARD constexpr bool valid(std::string_view str) noexcept { return version{}.from_string_noexcept(str); }

FST_NODISCARD constexpr from_chars_result from_chars(const char* first, const char* last, version& v) noexcept {
  return v.from_chars(first, last);
}

FST_NODISCARD constexpr to_chars_result to_chars(char* first, char* last, const version& v) noexcept {
  return v.to_chars(first, last);
}

FST_NODISCARD constexpr std::optional<version> from_string_noexcept(std::string_view str) noexcept {
  if (version v{}; v.from_string_noexcept(str)) {
    return v;
  }

  return std::nullopt;
}

FST_NODISCARD constexpr version from_string(std::string_view str) { return version{ str }; }

FST_NODISCARD inline std::string to_string(const version& v) { return v.to_string(); }

template <typename Char, typename Traits>
inline std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os, const version& v) {
  for (const auto c : v.to_string()) {
    os.put(c);
  }

  return os;
}

inline namespace comparators {

  enum struct comparators_option : std::uint8_t { exclude_prerelease, include_prerelease };

  FST_NODISCARD constexpr int compare(const version& lhs, const version& rhs,
      comparators_option option = comparators_option::include_prerelease) noexcept {
    if (option == comparators_option::exclude_prerelease) {
      return version{ lhs.major, lhs.minor, lhs.patch }.compare(version{ rhs.major, rhs.minor, rhs.patch });
    }
    return lhs.compare(rhs);
  }

  FST_NODISCARD constexpr bool equal_to(const version& lhs, const version& rhs,
      comparators_option option = comparators_option::include_prerelease) noexcept {
    return compare(lhs, rhs, option) == 0;
  }

  FST_NODISCARD constexpr bool not_equal_to(const version& lhs, const version& rhs,
      comparators_option option = comparators_option::include_prerelease) noexcept {
    return compare(lhs, rhs, option) != 0;
  }

  FST_NODISCARD constexpr bool greater(const version& lhs, const version& rhs,
      comparators_option option = comparators_option::include_prerelease) noexcept {
    return compare(lhs, rhs, option) > 0;
  }

  FST_NODISCARD constexpr bool greater_equal(const version& lhs, const version& rhs,
      comparators_option option = comparators_option::include_prerelease) noexcept {
    return compare(lhs, rhs, option) >= 0;
  }

  FST_NODISCARD constexpr bool less(const version& lhs, const version& rhs,
      comparators_option option = comparators_option::include_prerelease) noexcept {
    return compare(lhs, rhs, option) < 0;
  }

  FST_NODISCARD constexpr bool less_equal(const version& lhs, const version& rhs,
      comparators_option option = comparators_option::include_prerelease) noexcept {
    return compare(lhs, rhs, option) <= 0;
  }

} // namespace comparators

namespace version_range {
  namespace detail {
    using namespace fst::detail;

    class range {
    public:
      constexpr explicit range(std::string_view str) noexcept
          : str_{ str } {}

      constexpr bool satisfies(const version& ver, bool include_prerelease) const {
        range_parser parser{ str_ };

        auto is_logical_or = [&parser]() constexpr noexcept->bool {
          return parser.current_token.type == range_token_type::logical_or;
        };

        auto is_operator = [&parser]() constexpr noexcept->bool {
          return parser.current_token.type == range_token_type::range_operator;
        };

        auto is_number = [&parser]() constexpr noexcept->bool {
          return parser.current_token.type == range_token_type::number;
        };

        const bool has_prerelease = ver.tag != version_tag::none;

        do {
          if (is_logical_or()) {
            parser.advance_token(range_token_type::logical_or);
          }

          bool contains = true;
          bool allow_compare = include_prerelease;

          while (is_operator() || is_number()) {
            const auto range = parser.parse_range();
            const bool equal_without_tags = equal_to(range.ver, ver, comparators_option::exclude_prerelease);

            if (has_prerelease && equal_without_tags) {
              allow_compare = true;
            }

            if (!range.satisfies(ver)) {
              contains = false;
              break;
            }
          }

          if (has_prerelease) {
            if (allow_compare && contains) {
              return true;
            }
          }
          else if (contains) {
            return true;
          }

        } while (is_logical_or());

        return false;
      }

    private:
      enum struct range_operator : std::uint8_t { less, less_or_equal, greater, greater_or_equal, equal };

      struct range_comparator {
        range_operator op;
        version ver;

        constexpr bool satisfies(const version& version) const {
          switch (op) {
          case range_operator::equal:
            return version == ver;
          case range_operator::greater:
            return version > ver;
          case range_operator::greater_or_equal:
            return version >= ver;
          case range_operator::less:
            return version < ver;
          case range_operator::less_or_equal:
            return version <= ver;
          default:
            FST_VERSION_THROW_INVALID_ARG("fst::range unexpected operator.");
          }
        }
      };

      enum struct range_token_type : std::uint8_t {
        none,
        number,
        range_operator,
        dot,
        logical_or,
        hyphen,
        prerelease,
        end_of_line
      };

      struct range_token {
        range_token_type type = range_token_type::none;
        std::uint8_t number = 0;
        range_operator op = range_operator::equal;
        version_tag tag = version_tag::none;
      };

      struct range_lexer {
        std::string_view text;
        std::size_t pos;

        constexpr explicit range_lexer(std::string_view text) noexcept
            : text{ text }
            , pos{ 0 } {}

        constexpr range_token get_next_token() noexcept {
          while (!end_of_line()) {

            if (is_space(text[pos])) {
              advance(1);
              continue;
            }

            if (is_logical_or(text[pos])) {
              advance(2);
              return { range_token_type::logical_or };
            }

            if (is_operator(text[pos])) {
              const auto op = get_operator();
              return { range_token_type::range_operator, 0, op };
            }

            if (is_digit(text[pos])) {
              const auto number = get_number();
              return { range_token_type::number, number };
            }

            if (is_dot(text[pos])) {
              advance(1);
              return { range_token_type::dot };
            }

            if (is_hyphen(text[pos])) {
              advance(1);
              return { range_token_type::hyphen };
            }

            if (is_letter(text[pos])) {
              const auto prerelease = get_prerelease();
              return { range_token_type::prerelease, 0, range_operator::equal, prerelease };
            }
          }

          return { range_token_type::end_of_line };
        }

        constexpr bool end_of_line() const noexcept { return pos >= text.length(); }

        constexpr void advance(std::size_t i) noexcept { pos += i; }

        constexpr range_operator get_operator() noexcept {
          if (text[pos] == '<') {
            advance(1);
            if (text[pos] == '=') {
              advance(1);
              return range_operator::less_or_equal;
            }
            return range_operator::less;
          }
          else if (text[pos] == '>') {
            advance(1);
            if (text[pos] == '=') {
              advance(1);
              return range_operator::greater_or_equal;
            }
            return range_operator::greater;
          }
          else if (text[pos] == '=') {
            advance(1);
            return range_operator::equal;
          }

          return range_operator::equal;
        }

        constexpr std::uint8_t get_number() noexcept {
          const auto first = text.data() + pos;
          const auto last = text.data() + text.length();
          if (std::uint8_t n{}; from_chars(first, last, n) != nullptr) {
            advance(length(n));
            return n;
          }

          return 0;
        }

        constexpr version_tag get_prerelease() noexcept {
          const auto first = text.data() + pos;
          const auto last = text.data() + text.length();
          if (first > last) {
            advance(1);
            return version_tag::none;
          }

          if (version_tag p{}; from_chars(first, last, p) != nullptr) {
            advance(length(p));
            return p;
          }

          advance(1);

          return version_tag::none;
        }
      };

      struct range_parser {
        range_lexer lexer;
        range_token current_token;

        constexpr explicit range_parser(std::string_view str)
            : lexer{ str }
            , current_token{ range_token_type::none } {
          advance_token(range_token_type::none);
        }

        constexpr void advance_token(range_token_type token_type) {
          if (current_token.type != token_type) {
            FST_VERSION_THROW_INVALID_ARG("fst::range unexpected token.");
          }
          current_token = lexer.get_next_token();
        }

        constexpr range_comparator parse_range() {
          if (current_token.type == range_token_type::number) {
            const auto version = parse_version();
            return { range_operator::equal, version };
          }
          else if (current_token.type == range_token_type::range_operator) {
            const auto range_operator = current_token.op;
            advance_token(range_token_type::range_operator);
            const auto version = parse_version();
            return { range_operator, version };
          }

          return { range_operator::equal, version{} };
        }

        constexpr version parse_version() {
          const auto major = parse_number();

          advance_token(range_token_type::dot);
          const auto minor = parse_number();

          advance_token(range_token_type::dot);
          const auto patch = parse_number();

          version_tag prerelease = version_tag::none;
          std::uint8_t tag_number = 0;

          if (current_token.type == range_token_type::hyphen) {
            advance_token(range_token_type::hyphen);
            prerelease = parse_prerelease();
            advance_token(range_token_type::dot);
            tag_number = parse_number();
          }

          return { major, minor, patch, prerelease, tag_number };
        }

        constexpr std::uint8_t parse_number() {
          const auto token = current_token;
          advance_token(range_token_type::number);

          return token.number;
        }

        constexpr version_tag parse_prerelease() {
          const auto token = current_token;
          advance_token(range_token_type::prerelease);

          return token.tag;
        }
      };

      std::string_view str_;
    };

  } // namespace detail

  enum struct satisfies_option : std::uint8_t { exclude_prerelease, include_prerelease };

  constexpr bool satisfies(
      const version& ver, std::string_view str, satisfies_option option = satisfies_option::exclude_prerelease) {
    switch (option) {
    case satisfies_option::exclude_prerelease:
      return detail::range{ str }.satisfies(ver, false);
    case satisfies_option::include_prerelease:
      return detail::range{ str }.satisfies(ver, true);
    default:
      FST_VERSION_THROW_INVALID_ARG("fst::range unexpected satisfies_option.");
    }
  }

} // namespace version_range
} // namespace fst

#undef FST_VERSION_THROW_INVALID_ARG

#if __FST_CLANG__
#pragma clang diagnostic pop
#endif
