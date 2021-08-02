#include <gtest/gtest.h>
#include <fst/allocator.h>
#include <fst/print>
#include <fst/span>
#include <array>

namespace {
TEST(allocator, constructor) {

  constexpr std::size_t r_size = 64;
  std::array<std::uint8_t, 512 + 64 * 1024> raw_data;
  raw_data.fill(0);

  fst::memory_pool_allocator pool(raw_data.data(), raw_data.size());
  void* data = pool.allocate(r_size);
  EXPECT_NE(data, nullptr);

  fst::span<std::uint8_t> buffer((std::uint8_t*)data, r_size);
  for (std::size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = i;
  }

  //  fst::print("ASLKAJSAKLJSKLASA", (void*)raw_data.data(), data);
  //  for (std::size_t i = 0; i < r_size; i++) {
  //    fst::print("ASLKAJSAKLJSKLASA", (int)raw_data[i]);
  //    //    EXPECT_EQ(raw_data[i], buffer[i]);
  //  }

  pool.free(data);
}

} // namespace
