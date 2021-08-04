#include <benchmark/benchmark.h>
#include "fst/allocator.h"
#include "fst/print.h"
#include <array>
#include <vector>
#include <string>

inline constexpr std::size_t loop_size = 64;

using pool_allocator_type = fst::memory_pool_allocator<fst::crt_allocator>;
using allocator_type = fst::allocator<int, pool_allocator_type>;

static void fst_bench_alloc_loop(benchmark::State& state) {
  std::array<std::uint8_t, loop_size * sizeof(int) * sizeof(float) + pool_allocator_type::minimum_content_size> data;
  //  pool_allocator_type pool(data.data(), data.size());
  for (auto _ : state) {

    pool_allocator_type pool(data.data(), data.size());

    std::vector<int, allocator_type> buffer1((pool));
    std::vector<float, fst::allocator<float, pool_allocator_type>> buffer2((pool));

    buffer1.resize(loop_size);
    buffer2.resize(loop_size);

    for (std::size_t i = 0; i < buffer1.size(); i++) {
      buffer1[i] = i;
      buffer2[i] = i * 2;
    }
    benchmark::ClobberMemory();
  }
}
BENCHMARK(fst_bench_alloc_loop);

static void fst_bench_alloc_std_loop(benchmark::State& state) {

  for (auto _ : state) {
    std::vector<int> buffer1;
    std::vector<float> buffer2;

    buffer1.resize(loop_size);
    buffer2.resize(loop_size);

    for (std::size_t i = 0; i < buffer1.size(); i++) {
      buffer1[i] = i;
      buffer2[i] = i * 2;
    }
    benchmark::ClobberMemory();
  }
}
BENCHMARK(fst_bench_alloc_std_loop);

static void fst_bench_alloc_std_array_loop(benchmark::State& state) {
  float k = 0;
  for (auto _ : state) {
    std::array<int, loop_size> buffer1;
    std::array<float, loop_size> buffer2;

    //    buffer1.resize(64);
    //    buffer2.resize(64);

    for (std::size_t i = 0; i < buffer1.size(); i++) {
      buffer1[i] = i;
      buffer2[i] = i * 2;
    }
    benchmark::ClobberMemory();
    k += buffer2[3] + buffer1[2];
  }

  benchmark::DoNotOptimize(k);
}
BENCHMARK(fst_bench_alloc_std_array_loop);
