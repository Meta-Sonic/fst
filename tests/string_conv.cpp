#include <gtest/gtest.h>
#include <fst/print>
#include <fst/string_conv>
#include <cstdlib>
#include <array>
#include <random>
#include <chrono>

#include "fst/ascii.h"

namespace {
TEST(string_conv, to_string_int) {
  std::array<char, 32> buffer;

  std::array values = { 0, -1, 1, 99, -99, 100, -100, 999, -999, 1000, -1000, std::numeric_limits<int>::max(),
    std::numeric_limits<int>::min() };

  for (int v : values) {
    EXPECT_EQ(std::to_string(v), fst::string_conv::to_string(buffer, v));
  }
}

TEST(string_conv, to_string_uint) {
  std::array<char, 32> buffer;

  std::array values = { 0u, 1u, 99u, 100u, 999u, 1000u, 99999u, 5000001u, std::numeric_limits<unsigned int>::max() };

  for (unsigned int v : values) {
    EXPECT_EQ(std::to_string(v), fst::string_conv::to_string(buffer, v));
  }
}

TEST(string_conv, to_string_long) {
  std::array<char, 32> buffer;

  std::array values = { 0l, -1l, 1l, 99l, -99l, 100l, -100l, 999l, -999l, 1000l, -1000l,
    std::numeric_limits<long>::max(), std::numeric_limits<long>::min() };

  for (long v : values) {
    EXPECT_EQ(std::to_string(v), fst::string_conv::to_string(buffer, v));
  }
}

TEST(string_conv, to_string_ulong) {
  std::array<char, 32> buffer;

  std::array values
      = { 0ul, 1ul, 99ul, 100ul, 999ul, 1000ul, 99999ul, 5000001ul, std::numeric_limits<unsigned long>::max() };

  for (unsigned long v : values) {
    EXPECT_EQ(std::to_string(v), fst::string_conv::to_string(buffer, v));
  }
}

TEST(string_conv, to_string_longlong) {
  std::array<char, 32> buffer;

  std::array values
      = { 0ll, 1ll, 99ll, 100ll, 999ll, 1000ll, 99999ll, 5000001ll, std::numeric_limits<long long>::max() };

  for (long long v : values) {
    EXPECT_EQ(std::to_string(v), fst::string_conv::to_string(buffer, v));
  }
}

TEST(string_conv, to_string_ulonglong) {
  std::array<char, 32> buffer;

  std::array values = { 0ull, 1ull, 99ull, 100ull, 999ull, 1000ull, 99999ull, 5000001ull,
    std::numeric_limits<unsigned long long>::max() };

  for (unsigned long long v : values) {
    EXPECT_EQ(std::to_string(v), fst::string_conv::to_string(buffer, v));
  }
}

TEST(string_conv, to_string_float) {
  std::array<char, 32> buffer;
  EXPECT_EQ("0", fst::string_conv::to_string(buffer, 0.0f));
  EXPECT_EQ("0", fst::string_conv::to_string(buffer, -0.0f));
  EXPECT_EQ("1", fst::string_conv::to_string(buffer, 1.0f));
  EXPECT_EQ("-1", fst::string_conv::to_string(buffer, -1.0f));
  EXPECT_EQ("99", fst::string_conv::to_string(buffer, 99.0f));
  EXPECT_EQ("100", fst::string_conv::to_string(buffer, 100.0f));
  EXPECT_EQ("-100", fst::string_conv::to_string(buffer, -100.0f));

  EXPECT_EQ("0.2", fst::string_conv::to_string(buffer, 0.2f));
  EXPECT_EQ("-0.2", fst::string_conv::to_string(buffer, -0.2f));

  EXPECT_EQ("28.2", fst::string_conv::to_string(buffer, 28.2f));
  EXPECT_EQ("-28.2", fst::string_conv::to_string(buffer, -28.2f));

  EXPECT_EQ("123.456", fst::string_conv::to_string(buffer, 123.456f));
  EXPECT_EQ("-123.456", fst::string_conv::to_string(buffer, -123.456f));
}

TEST(string_conv, to_string_float_precision) {
  std::array<char, 32> buffer;
  EXPECT_EQ("123", fst::string_conv::to_string<0>(buffer, 123.456f));
  EXPECT_EQ("-123", fst::string_conv::to_string<0>(buffer, -123.456f));

  EXPECT_EQ("124", fst::string_conv::to_string<0>(buffer, 123.756f));
  EXPECT_EQ("-124", fst::string_conv::to_string<0>(buffer, -123.756f));

  EXPECT_EQ("123.5", fst::string_conv::to_string<1>(buffer, 123.456f));
  EXPECT_EQ("-123.5", fst::string_conv::to_string<1>(buffer, -123.456f));

  EXPECT_EQ("123.46", fst::string_conv::to_string<2>(buffer, 123.456f));
  EXPECT_EQ("-123.46", fst::string_conv::to_string<2>(buffer, -123.456f));

  EXPECT_EQ("40.00", fst::string_conv::to_string<2>(buffer, 40.0f));
  EXPECT_EQ("-40.00", fst::string_conv::to_string<2>(buffer, -40.0f));

  EXPECT_EQ("41.10", fst::string_conv::to_string<2>(buffer, 41.1f));
  EXPECT_EQ("-41.10", fst::string_conv::to_string<2>(buffer, -41.1f));

  EXPECT_EQ("41.12", fst::string_conv::to_string<2>(buffer, 41.12f));
  EXPECT_EQ("-41.12", fst::string_conv::to_string<2>(buffer, -41.12f));

  EXPECT_EQ("51.00", fst::string_conv::to_string<2>(buffer, 51.00f));
  EXPECT_EQ("-51.00", fst::string_conv::to_string<2>(buffer, -51.00f));

  EXPECT_EQ("0.70", fst::string_conv::to_string<2>(buffer, 0.70f));
  EXPECT_EQ("-0.70", fst::string_conv::to_string<2>(buffer, -0.70f));
}

 TEST(string_conv, to_int) {
  {
    std::string str = std::to_string(-1000);
    EXPECT_EQ(std::atoi(str.c_str()), (int)fst::string_conv::to_number<int>(str));
  }

  {
    std::string str = std::to_string(1000);
    EXPECT_EQ(std::atoi(str.c_str()), (int)fst::string_conv::to_number<int>(str));
  }

  {
    std::string str = std::to_string(0);
    EXPECT_EQ(std::atoi(str.c_str()), (int)fst::string_conv::to_number<int>(str));
  }

  {
    std::string str = std::to_string(-0);
    EXPECT_EQ(std::atoi(str.c_str()), (int)fst::string_conv::to_number<int>(str));
  }

  {
    std::string str = std::to_string(std::numeric_limits<int>::max());
    EXPECT_EQ(std::atoi(str.c_str()), (int)fst::string_conv::to_number<int>(str));
  }

  {
    std::string str = std::to_string(std::numeric_limits<int>::min());
    EXPECT_EQ(std::atoi(str.c_str()), (int)fst::string_conv::to_number<int>(str));
  }
}

//
// TEST(string_conv, to_long) {
//  {
//    std::string str = std::to_string(-1000);
//    EXPECT_EQ(std::atol(str.c_str()), fst::string_conv::to_long(str));
//  }
//
//  {
//    std::string str = std::to_string(1000);
//    EXPECT_EQ(std::atol(str.c_str()), fst::string_conv::to_long(str));
//  }
//
//  {
//    std::string str = std::to_string(0);
//    EXPECT_EQ(std::atol(str.c_str()), fst::string_conv::to_long(str));
//  }
//
//  {
//    std::string str = std::to_string(-0);
//    EXPECT_EQ(std::atol(str.c_str()), fst::string_conv::to_long(str));
//  }
//
//  {
//    std::string str = std::to_string(std::numeric_limits<long>::max());
//    EXPECT_EQ(std::atol(str.c_str()), fst::string_conv::to_long(str));
//  }
//
//  //  {
//  //    std::string str = std::to_string(std::numeric_limits<long>::min());
//  //    EXPECT_EQ(std::atol(str.c_str()), fst::string_conv::to_long(str));
//  //  }
//}
//
////
// TEST(string_conv, to_float) {
//  {
//    std::string str = std::to_string(-1000);
//    EXPECT_EQ(std::atof(str.c_str()), fst::string_conv::to_float(str));
//  }
//
//  {
//    std::string str = std::to_string(1000);
//    EXPECT_EQ(std::atof(str.c_str()), fst::string_conv::to_float(str));
//  }
//
//  {
//    std::string str = std::to_string(0);
//    EXPECT_EQ(std::atof(str.c_str()), fst::string_conv::to_float(str));
//  }
//
//  {
//    std::string str = std::to_string(-0);
//    EXPECT_EQ(std::atof(str.c_str()), fst::string_conv::to_float(str));
//  }
//
//  {
//    std::string str = std::to_string(2.2);
//    EXPECT_EQ((float)std::atof(str.c_str()), fst::string_conv::to_float(str));
//  }
//
//  {
//
//    std::string str = std::to_string(-2.2);
//    EXPECT_EQ((float)std::atof(str.c_str()), fst::string_conv::to_float(str));
//  }
//
//  {
//
//    std::string str = std::to_string(-22093289.2232);
//    EXPECT_EQ((float)std::atof(str.c_str()), fst::string_conv::to_float(str));
//  }
//
//  {
//
//    std::string str = std::to_string(22093289.2232);
//    EXPECT_EQ((float)std::atof(str.c_str()), fst::string_conv::to_float(str));
//  }
//
//  //  {
//  //
//  //    std::string str = std::to_string(std::numeric_limits<float>::max());
//  //    EXPECT_EQ((float)std::atof(str.c_str()), fst::string_conv::to_float(str));
//  //  }
//}
//
// TEST(string_conv, constructor) {
//  //  float a = fst::to_float("32");
//
//  {
//    float b = std::strtof("32", nullptr);
//    EXPECT_EQ(b, 32.0f);
//  }
//
//  {
//    std::array<char, 3> a32 = { '3', '2', ' ' };
//    float b = std::strtof(a32.data(), nullptr);
//    EXPECT_EQ(b, 32.0f);
//  }
//
//  {
//    std::array<char, 2> a32 = { '3', '2' };
//    char k = '3';
//    float b = std::strtof(a32.data(), nullptr);
//    EXPECT_EQ(b, 32.0f);
//  }
//
//  {
//    char k[3] = { '3', '2' };
//    float b = std::strtof(k, nullptr);
//    EXPECT_EQ(b, 32.0f);
//  }
//
//  {
//    char k[3] = { '3', '2' };
//    float b = std::atof(k);
//    EXPECT_EQ(b, 32.0f);
//  }
//}
//
// TEST(string_conv, from_int) {
//  EXPECT_EQ(1, fst::string_conv::detail::get_int_10_size(0));
//  EXPECT_EQ(1, fst::string_conv::detail::get_int_10_size(9));
//  EXPECT_EQ(2, fst::string_conv::detail::get_int_10_size(10));
//  EXPECT_EQ(2, fst::string_conv::detail::get_int_10_size(99));
//  EXPECT_EQ(3, fst::string_conv::detail::get_int_10_size(100));
//  EXPECT_EQ(3, fst::string_conv::detail::get_int_10_size(999));
//  EXPECT_EQ(4, fst::string_conv::detail::get_int_10_size(1000));
//
//  fst::print("BACON", fst::string_conv::detail::int_to_string(0));
//  fst::print("BACON", fst::string_conv::detail::int_to_string(1));
//  fst::print("BACON", fst::string_conv::detail::int_to_string(-1));
//  fst::print("BACON", fst::string_conv::detail::int_to_string(9));
//  fst::print("BACON", fst::string_conv::detail::int_to_string(10));
//  fst::print("BACON", fst::string_conv::detail::int_to_string(99));
//  fst::print("BACON", fst::string_conv::detail::int_to_string(100));
//  fst::print("BACON", fst::string_conv::detail::int_to_string(955));
//  fst::print("BACON", fst::string_conv::detail::int_to_string(-955));
//  fst::print("BACON", fst::string_conv::detail::int_to_string(12345678));
//
//  std::array<char, 32> buffer;
//  fst::print("FBACON", fst::string_conv::detail::int_to_string(buffer, 0));
//  fst::print("FBACON", fst::string_conv::detail::int_to_string(buffer, 1));
//  fst::print("FBACON", fst::string_conv::detail::int_to_string(buffer, -1));
//  fst::print("FBACON", fst::string_conv::detail::int_to_string(buffer, 9));
//  fst::print("FBACON", fst::string_conv::detail::int_to_string(buffer, 10));
//  fst::print("FBACON", fst::string_conv::detail::int_to_string(buffer, 99));
//  fst::print("FBACON", fst::string_conv::detail::int_to_string(buffer, 100));
//  fst::print("FBACON", fst::string_conv::detail::int_to_string(buffer, 955));
//  fst::print("FBACON", fst::string_conv::detail::int_to_string(buffer, -955));
//  fst::print("FBACON", fst::string_conv::detail::int_to_string(buffer, 12345678));
//
//  fst::print("KBACON", fst::string_conv::int_to_string(buffer, 0));
//  fst::print("KBACON", fst::string_conv::int_to_string(buffer, 1));
//  fst::print("KBACON", fst::string_conv::int_to_string(buffer, -1));
//  fst::print("KBACON", fst::string_conv::int_to_string(buffer, 9));
//  fst::print("KBACON", fst::string_conv::int_to_string(buffer, 10));
//  fst::print("KBACON", fst::string_conv::int_to_string(buffer, 99));
//  fst::print("KBACON", fst::string_conv::int_to_string(buffer, 100));
//  fst::print("KBACON", fst::string_conv::int_to_string(buffer, 955));
//  fst::print("KBACON", fst::string_conv::int_to_string(buffer, -955));
//  fst::print("KBACON", fst::string_conv::int_to_string(buffer, 12345678));
//}
} // namespace
