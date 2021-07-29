#include <gtest/gtest.h>

#include "fst/slot_map.h"

namespace {
TEST(slot_map, constructor) {
  using map_type = fst::slot_map<std::string>;
  using key_type = map_type::key_type;
  map_type map;

  key_type k1 = map.insert("Bingo1");
  key_type k2 = map.insert("Bingo2");

  {
    auto it = map.find(k1);
    EXPECT_TRUE(it != map.end());
    EXPECT_EQ(*it, "Bingo1");
  }

  {
    auto it = map.find(k2);
    EXPECT_TRUE(it != map.end());
    EXPECT_EQ(*it, "Bingo2");
  }

  map.erase(k1);

  {
    auto it = map.find(k1);
    EXPECT_TRUE(it == map.end());
  }

  {
    auto it = map.find(k2);
    EXPECT_TRUE(it != map.end());
    EXPECT_EQ(*it, "Bingo2");
  }
}
} // namespace
