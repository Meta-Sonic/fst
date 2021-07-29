#include <gtest/gtest.h>

#include "fst/inplace_function.h"

namespace {
TEST(inplace_function, constructor) {
  fst::inplace_function<int()> fct = []() { return 32; };
  EXPECT_EQ(fct(), 32);
}

TEST(inplace_function, size_limit) {
  char k1 = 22;
  char k2 = 33;
  fst::inplace_function<int(), 2> fct = [k1, k2]() { return k1 + k2; };
  EXPECT_EQ(fct(), k1 + k2);
}
} // namespace
