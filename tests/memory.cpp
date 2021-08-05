#include <gtest/gtest.h>

#include "fst/memory.h"
#include <fst/print>

namespace {
TEST(memory, new_delete) {

  //  int* a = fst::memory::__new<int[2]>();
  int* a = fst::memory::new_array<int>(2);
  a[0] = 1;
  a[1] = 2;
  //  fst::memory::__delete<int[]>(a);
  fst::memory::delete_array(a);

  int* b = fst::memory::new_pointer<int>(32);
  EXPECT_EQ(*b, 32);
  fst::memory::__delete<int>(b);
  //  int* b = new int[8];
  //  int* b = fst::memory::__new<int[2]>();
  //  EXPECT_EQ(*a, 32);
  //  fst::memory::__delete(a);

  //  fst::print("PAGE SIZE", fst::memory::get_page_size());
  //  fst::print("CACHE SIZE", fst::memory::get_cache_size());
}

TEST(memory, info) {

  fst::print("PAGE SIZE", fst::memory::get_page_size());
  fst::print("CACHE SIZE", fst::memory::get_cache_size());
}
} // namespace
