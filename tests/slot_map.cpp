#include <gtest/gtest.h>

#include "fst/slot_map.h"
#include "fst/small_vector.h"

namespace {
TEST(slot_map, constructor) {
  using map_type = fst::slot_map<std::string>;
  using key_type = map_type::key_type;
  map_type map;

  key_type k1 = map.insert("Bingo1");
  key_type k2 = map.insert("Bingo2");
  key_type k3 = map.emplace("Bingo3");

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

template <typename T>
using smap_vector = fst::small_vector<T, 2>;
using smap_key = fst::slot_map_key<unsigned int, unsigned int>;

TEST(slot_map, small_vector) {
  using map_type = fst::slot_map<std::string, smap_key, smap_vector>;
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

  EXPECT_EQ(map.capacity(), 2);
  EXPECT_EQ(map.size(), 1);

  key_type k3 = map.insert("Bingo3");
  EXPECT_EQ(map.capacity(), 2);
  EXPECT_EQ(map.size(), 2);

  key_type k4 = map.insert("Bingo4");
  EXPECT_TRUE(map.capacity() > 2);
  EXPECT_EQ(map.size(), 3);
}
} // namespace
