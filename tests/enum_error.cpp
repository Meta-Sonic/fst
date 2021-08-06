#include <gtest/gtest.h>
#include "fst/enum_error.h"
#include "fst/enum_array.h"
#include <string_view>

namespace {
enum class error_type { none, type_1, type_2 };

TEST(enum_error, default_valid_result) { EXPECT_EQ(fst::enum_error<error_type>::valid_result, error_type::none); }

using result_t = fst::enum_error<error_type, error_type::none>;

TEST(enum_error, valid_result) { EXPECT_EQ(result_t::valid_result, error_type::none); }

TEST(enum_error, without_error) {
  result_t r0;
  EXPECT_EQ(r0.is_valid(), true);
  EXPECT_EQ(r0, error_type::none);
  EXPECT_EQ(r0.get(), error_type::none);
  EXPECT_FALSE(r0);
}

TEST(enum_error, with_error) {
  result_t r0(error_type::type_1);
  EXPECT_EQ(r0.is_valid(), false);
  EXPECT_EQ(r0, error_type::type_1);
  EXPECT_EQ(r0.get(), error_type::type_1);
  EXPECT_NE(r0, error_type::none);
  EXPECT_TRUE(r0);
}

TEST(enum_error, reassign) {
  result_t r0(error_type::type_1);
  EXPECT_EQ(r0.is_valid(), false);
  EXPECT_EQ(r0, error_type::type_1);
  EXPECT_EQ(r0.get(), error_type::type_1);
  EXPECT_NE(r0, error_type::none);
  EXPECT_TRUE(r0);

  r0 = error_type::none;
  EXPECT_EQ(r0.is_valid(), true);
  EXPECT_EQ(r0, error_type::none);
  EXPECT_EQ(r0.get(), error_type::none);
  EXPECT_FALSE(r0);
}

TEST(enum_error, copy) {
  result_t r0(error_type::type_1);
  EXPECT_EQ(r0.is_valid(), false);
  EXPECT_EQ(r0, error_type::type_1);
  EXPECT_EQ(r0.get(), error_type::type_1);
  EXPECT_NE(r0, error_type::none);
  EXPECT_TRUE(r0);

  result_t r1 = r0;
  EXPECT_EQ(r1.is_valid(), false);
  EXPECT_EQ(r1, error_type::type_1);
  EXPECT_NE(r1, error_type::none);
  EXPECT_TRUE(r1);
}

enum class error_type2 { none, type_1, type_2, count };
struct bbb {
  static constexpr fst::enum_array<const char*, error_type2> array = { { "No error", "type_1", "type_2" } };
};
using error_t = fst::enum_error<error_type2, error_type2::none, bbb>;

TEST(enum_error, to_string) {
  error_t e0 = error_type2::none;
  EXPECT_EQ(e0.to_string(), "No error");

  e0 = error_type2::type_1;
  EXPECT_EQ(e0.to_string(), "type_1");

  e0 = error_type2::type_2;
  EXPECT_EQ(e0.to_string(), "type_2");
  EXPECT_EQ(e0, "type_2");

  EXPECT_EQ(e0.is_valid(), false);
  EXPECT_TRUE(e0);
}

} // namespace
