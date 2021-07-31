#include <gtest/gtest.h>
#include "fst/compressed_pair.h"
#include <utility>
namespace {

struct empty_banana {};

TEST(compressed_pair, constructor) {
  //  fst::compressed_pair<int, int> p;
  //  p.first() = 2;
  //
  //  fst::compressed_pair<int, int> p2(1, 2);
  //  fst::compressed_pair<int, int> p3 = {13, 14};
  //  fst::compressed_pair<int, int> p4(13);
  //  fst::compressed_pair<int, empty_banana> p5(32);
  //  fst::compressed_pair<int, empty_banana> p6(empty_banana{});
  //  fst::compressed_pair<int, empty_banana> p7(34, empty_banana{});

  EXPECT_EQ(sizeof(fst::compressed_pair<int, int>), sizeof(int) + sizeof(int));

  EXPECT_EQ(sizeof(fst::compressed_pair<int, empty_banana>), sizeof(int));
  EXPECT_TRUE(sizeof(fst::compressed_pair<int, empty_banana>) < sizeof(std::pair<int, empty_banana>));

  EXPECT_EQ(sizeof(fst::compressed_pair<empty_banana, int>), sizeof(int));
  EXPECT_TRUE(sizeof(fst::compressed_pair<empty_banana, int>) < sizeof(std::pair<empty_banana, int>));

  EXPECT_EQ(sizeof(fst::compressed_pair<empty_banana, empty_banana>), sizeof(empty_banana));
  EXPECT_EQ(sizeof(empty_banana), 1);
  EXPECT_TRUE(sizeof(fst::compressed_pair<empty_banana, empty_banana>) < sizeof(std::pair<empty_banana, empty_banana>));
}
} // namespace
