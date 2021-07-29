#pragma once
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstring>
#include "fst/assert.h"

namespace fst {
class date {
public:
  static constexpr std::uint16_t invalid_year = std::numeric_limits<std::uint16_t>::max();
  static constexpr std::uint8_t invalid_month = std::numeric_limits<std::uint8_t>::max();
  static constexpr std::uint8_t invalid_day = std::numeric_limits<std::uint8_t>::max();

  date() noexcept = default;
  date(const date&) noexcept = default;
  date(date&&) noexcept = default;

  inline date(std::uint16_t y, std::uint8_t m, std::uint8_t d) noexcept
      : _year(y) {
    _month = m < 12 ? m : invalid_month;
    _day = d < 31 ? d : invalid_day;
  }

  inline date(const std::string& str) { from_string(str.c_str()); }
  inline date(const char* str) { from_string(str); }
  inline date& operator=(const date&) noexcept = default;
  inline date& operator=(date&&) noexcept = default;
  inline date& operator=(const std::string& str) { return from_string(str.c_str()); }
  inline date& operator=(const char* str) { return from_string(str); }

  inline bool is_valid() const { return _year != invalid_year && _month != invalid_month && _day != invalid_day; }
  inline std::uint16_t year() const { return _year; }
  inline std::uint8_t month() const { return _month; }
  inline std::uint8_t day() const { return _day; }

  inline void set_year(std::uint16_t y) { _year = y; }
  inline void set_month(std::uint8_t m) { _month = m < 12 ? m : invalid_month; }
  inline void set_day(std::uint8_t d) { _day = d < 31 ? d : invalid_day; }

  inline std::string to_string() const {
    char str[date_size + 1];
    return std::snprintf(str, date_size + 1, "%4hd-%02hhd-%02hhd", _year, _month, _day) == date_size ? str : "";
  }

private:
  static constexpr std::size_t date_size = std::char_traits<char>::length("YYYY-MM-DD");
  std::uint16_t _year = invalid_year;
  std::uint8_t _month = invalid_month;
  std::uint8_t _day = invalid_day;

  inline date& from_string(const char* str) {
    int y, m, d;
    if (std::sscanf(str, "%d-%d-%d", &y, &m, &d) != 3) {
      _year = invalid_year;
      _month = invalid_month;
      _day = invalid_day;
      return *this;
    }

    _year = y >= 0 ? (std::uint16_t)y : invalid_year;
    _month = m < 12 ? (std::uint8_t)m : invalid_month;
    _day = d < 31 ? (std::uint8_t)d : invalid_day;
    return *this;
  }
};

class time {
public:
  static constexpr std::uint8_t invalid_hour = std::numeric_limits<std::uint8_t>::max();
  static constexpr std::uint8_t invalid_minute = std::numeric_limits<std::uint8_t>::max();
  static constexpr std::uint8_t invalid_second = std::numeric_limits<std::uint8_t>::max();

  time() noexcept = default;
  time(const time&) noexcept = default;
  time(time&&) noexcept = default;

  inline time(std::uint8_t h, std::uint8_t m, std::uint8_t s) noexcept {
    _hour = h < 24 ? h : invalid_hour;
    _min = m < 60 ? m : invalid_minute;
    _sec = s < 60 ? s : invalid_second;
  }

  inline time(const std::string& str) { from_string(str.c_str()); }
  inline time(const char* str) { from_string(str); }
  inline time& operator=(const time&) noexcept = default;
  inline time& operator=(time&&) noexcept = default;
  inline time& operator=(const std::string& str) { return from_string(str.c_str()); }
  inline time& operator=(const char* str) { return from_string(str); }

  inline bool is_valid() const { return !(_hour == invalid_hour || _min == invalid_minute || _sec == invalid_second); }
  inline std::uint8_t hour() const { return _hour; }
  inline std::uint8_t minute() const { return _min; }
  inline std::uint8_t second() const { return _sec; }

  inline void set_hour(std::uint8_t h) { _hour = h < 60 ? h : invalid_hour; }
  inline void set_minute(std::uint8_t m) { _min = m < 60 ? m : invalid_minute; }
  inline void set_second(std::uint8_t s) { _sec = s < 60 ? s : invalid_second; }

  inline std::string to_string() const {
    char str[time_size + 1];
    return std::snprintf(str, time_size + 1, "%02hhd:%02hhd:%02hhd", _hour, _min, _sec) == time_size ? str : "";
  }

private:
  static constexpr std::size_t time_size = std::char_traits<char>::length("HH:MM:SS");
  std::uint8_t _hour = invalid_hour;
  std::uint8_t _min = invalid_minute;
  std::uint8_t _sec = invalid_second;

  inline time& from_string(const char* str) {
    int h, m, s;
    if (std::sscanf(str, "%d:%d:%d", &h, &m, &s) != 3) {
      _hour = invalid_hour;
      _min = invalid_minute;
      _sec = invalid_second;
      return *this;
    }

    _hour = h < 24 ? (std::uint8_t)h : invalid_hour;
    _min = m < 60 ? (std::uint8_t)m : invalid_minute;
    _sec = s < 60 ? (std::uint8_t)s : invalid_second;
    return *this;
  }
};

/// Only this format 'YYYY-MM-DDTHH:MM:SSZ' is supported for now.
class date_and_time {
public:
  date_and_time() noexcept = default;
  date_and_time(const date_and_time&) noexcept = default;
  date_and_time(date_and_time&&) noexcept = default;

  inline date_and_time(
      std::uint16_t y, std::uint8_t mon, std::uint8_t d, std::uint8_t h, std::uint8_t m, std::uint8_t s) noexcept
      : _date(y, mon, d)
      , _time(h, m, s) {}

  inline date_and_time(const std::string& str) { from_string(str.c_str()); }
  inline date_and_time(const char* str) { from_string(str); }
  inline date_and_time& operator=(const date_and_time&) noexcept = default;
  inline date_and_time& operator=(date_and_time&&) noexcept = default;
  inline date_and_time& operator=(const std::string& str) { return from_string(str.c_str()); }
  inline date_and_time& operator=(const char* str) { return from_string(str); }

  inline date_and_time& set_utc(const char* str) {
    constexpr std::size_t time_size = std::char_traits<char>::length("YYYY-MM-DDTHH:MM:SSZ");

    std::size_t length = std::strlen(str);
    if (length != time_size) {
      _date = date();
      _time = time();
      return *this;
    }

    if (str[10] != 'T') {
      _date = date();
      _time = time();
      return *this;
    }

    if (str[time_size - 1] != 'Z') {
      fst_error("date_and_time : Only Z timezone is supported for UTC date format.");
      _date = date();
      _time = time();
      return *this;
    }

    _date = date(str);
    _time = time(str + 11);
    return *this;
  }

  inline static date_and_time from_utc(const char* str) {
    date_and_time dt;
    dt.set_utc(str);
    return dt;
  }

  inline bool is_valid() const { return _date.is_valid() && _time.is_valid(); }
  inline date& get_date() { return _date; }
  inline const date& get_date() const { return _date; }
  inline time& get_time() { return _time; }
  inline const time& get_time() const { return _time; }
  inline std::uint16_t year() const { return _date.year(); }
  inline std::uint8_t month() const { return _date.month(); }
  inline std::uint8_t day() const { return _date.day(); }
  inline std::uint8_t hour() const { return _time.hour(); }
  inline std::uint8_t minute() const { return _time.minute(); }
  inline std::uint8_t second() const { return _time.second(); }

  inline void set_date(const date& d) { _date = d; }
  inline void set_time(const time& t) { _time = t; }
  inline void set_year(std::uint16_t y) { _date.set_year(y); }
  inline void set_month(std::uint8_t m) { _date.set_month(m); }
  inline void set_day(std::uint8_t d) { _date.set_day(d); }
  inline void set_hour(std::uint8_t h) { _time.set_hour(h); }
  inline void set_minute(std::uint8_t m) { _time.set_minute(m); }
  inline void set_second(std::uint8_t s) { _time.set_second(s); }

  inline std::string to_string() const { return _date.to_string() + ':' + _time.to_string(); }

  inline std::string to_utc_string() { return _date.to_string() + 'T' + _time.to_string() + 'Z'; }

private:
  static constexpr std::size_t str_size = std::char_traits<char>::length("YYYY-MM-DD:HH:MM:SS");

  date _date;
  time _time;

  inline date_and_time& from_string(const char* str) {
    std::size_t length = std::strlen(str);
    if (length != str_size) {
      _date = date();
      _time = time();
      return *this;
    }

    _date = date(str);
    _time = time(str + 11);
    return *this;
  }
};

/// YYYY-MM-DD
inline std::string current_date() {
  constexpr std::size_t date_size = std::char_traits<char>::length("YYYY-MM-DD");
  char str[date_size + 1];

  std::time_t t = std::time(nullptr);
  return std::strftime(str, sizeof(str), "%F", std::localtime(&t)) == date_size ? str : "";
}

/// YYYY-MM-DD:HH:MM:SS
inline std::string current_date_and_time() {
  constexpr std::size_t str_size = std::char_traits<char>::length("YYYY-MM-DD:HH:MM:SS");
  char str[str_size + 1];

  std::time_t t = std::time(nullptr);
  return std::strftime(str, sizeof(str), "%F:%T", std::localtime(&t)) == str_size ? str : "";
}

/// HH:MM:SS
inline std::string current_time() {
  constexpr std::size_t str_size = std::char_traits<char>::length("HH:MM:SS");
  char str[str_size + 1];

  std::time_t t = std::time(nullptr);
  return std::strftime(str, sizeof(str), "%T", std::localtime(&t)) == str_size ? str : "";
}

/// HH:MM:SS:MMM
inline std::string current_time_ms() {
  // https://stackoverflow.com/a/35157784
  constexpr std::size_t time_size = std::char_traits<char>::length("HH:MM:SS");
  constexpr std::size_t ms_size = std::char_traits<char>::length(":MMM");
  char str[time_size + ms_size + 1];

  // Current time.
  std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);

  if (std::strftime(str, sizeof(str), "%T", std::localtime(&t)) != time_size) {
    return "";
  }

  // Get number of milliseconds for the current second (remainder after division into seconds).
  std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

  // If a call to sprintf or snprintf causes copying to take place between objects that overlap,
  // the behavior is undefined (e.g. sprintf(buf, "%s text", buf);).
  return std::snprintf(str + time_size, ms_size + 1, ":%03d", (int)ms.count()) == ms_size ? str : "";
}

/// Current date and time expressed according to ISO 8601.
/// YYYY-MM-DDTHH:MM:SSZ
inline std::string current_utc_date_and_time() {
  constexpr std::size_t time_size = std::char_traits<char>::length("YYYY-MM-DDTHH:MM:SSZ");
  char str[time_size + 1];

  // Current date/time based on current system
  std::time_t now = std::time(nullptr);

  // Convert now to tm struct for UTC.
  std::tm* ltm = std::gmtime(&now);

  return std::snprintf(str, time_size + 1, "%04d-%02d-%02dT%02d:%02d:%02dZ", 1900 + ltm->tm_year, 1 + ltm->tm_mon,
             ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec)
          == time_size
      ? str
      : "";
}
} // namespace fst.
