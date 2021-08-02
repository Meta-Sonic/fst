#include <gtest/gtest.h>
#include <fst/allocator.h>
#include <fst/print>
#include <fst/span>
#include <array>
#include <vector>

namespace {
// TEST(allocator, constructor) {
//
//  constexpr std::size_t r_size = 64;
//  //  std::array<std::uint8_t, 512 + 64 * 1024> raw_data;
//  //  raw_data.fill(0);
//
//  fst::memory_pool_allocator pool;
//  void* data = pool.allocate(r_size);
//  EXPECT_NE(data, nullptr);
//
//  fst::span<std::uint8_t> buffer((std::uint8_t*)data, r_size);
//  for (std::size_t i = 0; i < buffer.size(); i++) {
//    buffer[i] = i;
//  }
//
//  for (std::size_t i = 0; i < r_size; i++) {
//    EXPECT_EQ(((std::uint8_t*)data)[i], buffer[i]);
//  }
//
//  pool.free(data);
//}

TEST(allocator, std) {
  using pool_allocator_type = fst::memory_pool_allocator<>;
  using allocator_type = fst::allocator<int, pool_allocator_type>;

  constexpr std::size_t n_int = 64;
  constexpr std::size_t data_size = n_int * sizeof(int) + pool_allocator_type::minimum_content_size;

  std::array<std::uint8_t, data_size> data;
  pool_allocator_type pool(data.data(), data.size());

  {
    std::vector<int, allocator_type> buffer((allocator_type(pool)));

    buffer.resize(n_int);

    for (std::size_t i = 0; i < buffer.size(); i++) {
      buffer[i] = i;
    }

    for (std::size_t i = 0; i < buffer.size(); i++) {
      EXPECT_EQ(i, buffer[i]);
    }
  }

  { std::vector<int, allocator_type> buffer((allocator_type(pool))); }
}

TEST(allocator, std2) {
  //  using pool_allocator_type = fst::memory_pool_allocator<>;
  using allocator_type = fst::allocator<int>;

  //  constexpr std::size_t n_int = 64;
  //  constexpr std::size_t data_size = n_int * sizeof(int) + pool_allocator_type::minimum_content_size;

  //  std::array<std::uint8_t, data_size> data;
  //  pool_allocator_type pool(data.data(), data.size());
  std::vector<int, allocator_type> buffer;
  buffer.resize(64);

  for (std::size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = i;
  }
}
} // namespace
