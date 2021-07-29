#include <gtest/gtest.h>
#include "fst/small_vector.h"

namespace {
TEST(small_vector, constructor) {
  fst::small_vector<int, 32> vec;
  vec.push_back(32);
  EXPECT_EQ(vec.capacity(), 32);
  EXPECT_EQ(vec.size(), 1);
  EXPECT_EQ(vec[0], 32);
}
} // namespace
