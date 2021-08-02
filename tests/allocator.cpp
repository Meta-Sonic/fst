#include <gtest/gtest.h>
#include <fst/allocator.h>
#include <fst/print>

namespace {

//#define RAPIDJSON_ALIGN16(x) (((x) + static_cast<std::size_t>(15u)) & ~static_cast<std::size_t>(15u))
//#define RAPIDJSON_ALIGN32(x) (((x) + static_cast<std::size_t>(31u)) & ~static_cast<std::size_t>(31u))

TEST(allocator, constructor) {
  fst::memory_pool_allocator pool;
  void* data = pool.allocate(64);
  EXPECT_NE(data, nullptr);

  //  fst::print("ASAS 2", RAPIDJSON_ALIGN32(8), std::alignment_of<int>::value);
}

} // namespace
