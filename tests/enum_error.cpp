#include <gtest/gtest.h>
#include "fst/enum_error.h"

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

} // namespace
