#include <gtest/gtest.h>

#include "fst/memory.h"
#include <fst/print>

namespace {
TEST(memory, info) {

  fst::print("PAGE SIZE", fst::memory::get_page_size());
  fst::print("CACHE SIZE", fst::memory::get_cache_size());
}
} // namespace
