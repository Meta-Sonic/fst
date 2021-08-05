#include <gtest/gtest.h>
#include <fst/allocator.h>
#include <fst/print>
#include <fst/span>
#include <array>
#include <vector>

namespace {
TEST(allocator, vector) {
  fst::print("ALLLL", sizeof(fst::memory_pool_allocator<>::shared_data));
  using pool_allocator_type = fst::memory_pool_allocator<>;
  using allocator_type = fst::allocator<int, pool_allocator_type>;

  constexpr std::size_t n_int = 64;
  constexpr std::size_t data_size = n_int * sizeof(int) + pool_allocator_type::minimum_content_size;

  std::array<std::uint8_t, data_size> data;
  pool_allocator_type pool(data.data(), data.size());

  EXPECT_FALSE(pool.is_shared());

  std::vector<int, allocator_type> vec((pool));

  EXPECT_TRUE(pool.is_shared());

  //  std::vector<int, allocator_type> vec2 = std::move(vec);

  //  EXPECT_FALSE(pool.is_shared());
}

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
      buffer[i] = (int)i;
    }

    for (std::size_t i = 0; i < buffer.size(); i++) {
      EXPECT_EQ((int)i, buffer[i]);
    }
  }

  { std::vector<int, allocator_type> buffer((allocator_type(pool))); }
}

TEST(allocator, std2) {
  using allocator_type = fst::allocator<int>;

  std::vector<int, allocator_type> buffer;
  buffer.resize(64);

  for (std::size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = (int)i;
  }
}

TEST(allocator, std3) {
  using pool_allocator_type = fst::memory_pool_allocator<>;
  using allocator_type = fst::allocator<int, pool_allocator_type>;

  constexpr std::size_t n_int = 64;
  pool_allocator_type pool;

  {
    std::vector<int, allocator_type> buffer;

    buffer.resize(n_int);

    for (std::size_t i = 0; i < buffer.size(); i++) {
      buffer[i] = (int)i;
    }

    for (std::size_t i = 0; i < buffer.size(); i++) {
      EXPECT_EQ(i, buffer[i]);
    }
  }
}

TEST(allocator, std4) {
  using pool_allocator_type = fst::memory_pool_allocator<fst::crt_allocator>;
  using allocator_type = fst::allocator<int, pool_allocator_type>;

  std::array<std::uint8_t, 1024> data;
  pool_allocator_type pool(data.data(), data.size());

  EXPECT_EQ(pool.is_freeable, false);
  EXPECT_EQ(pool.is_ref_counted, true);

  bool alway_eq = fst::allocator<float, pool_allocator_type>::is_always_equal::value;
  EXPECT_EQ(alway_eq, false);

  std::vector<int, allocator_type> buffer1((pool));
  std::vector<float, fst::allocator<float, pool_allocator_type>> buffer2((pool));

  buffer1.resize(64);
  buffer2.resize(64);

  for (std::size_t i = 0; i < buffer1.size(); i++) {
    buffer1[i] = (int)i;
    buffer2[i] = (float)i * 2;
  }

  for (std::size_t i = 0; i < buffer1.size(); i++) {
    EXPECT_EQ(buffer1[i] * 2, buffer2[i]);
  }
}
} // namespace
