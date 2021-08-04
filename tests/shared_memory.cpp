//#include <gtest/gtest.h>
//#include <fst/print>
////#include <thread>
////#include <chrono>
//#include "fst/shared_memory.h"
//#include <fst/span>
//
// namespace {
// TEST(shared_memory, constructor) {
//  fst::shared_memory sm;
//  fst::shared_memory sm2;
//
//  fst::shared_memory::error_type err = sm.create("fst_shared", 64);
//  EXPECT_EQ(err, fst::shared_memory::error_type::none);
//  EXPECT_EQ(sm.size(), 64);
//
//  err = sm2.create("fst_shared", 64);
//  EXPECT_EQ(err, fst::shared_memory::error_type::creation_failed);
//  //  EXPECT_EQ(sm.size(), 64);
//
//  fst::span<std::uint8_t> buffer(sm.data(), sm.size());
//  for (std::size_t i = 0; i < buffer.size(); i++) {
//    buffer[i] = i;
//  }
//  //  sm.data()[0] = 45;
//}
//
// TEST(shared_memory, double) {
//  fst::shared_memory sm;
//
//  fst::shared_memory::error_type err = sm.create("fst_shared", 64);
//  EXPECT_EQ(err, fst::shared_memory::error_type::none);
//  EXPECT_EQ(sm.size(), 64);
//
//  fst::span<std::uint8_t> buffer(sm.data(), sm.size());
//  for (std::size_t i = 0; i < buffer.size(); i++) {
//    buffer[i] = i;
//  }
//
//  fst::shared_memory sm2;
//  err = sm2.open("fst_shared", 64);
//  EXPECT_EQ(err, fst::shared_memory::error_type::none);
//  EXPECT_EQ(sm2.size(), 64);
//
//  sm2.data()[0] = 89;
//
//  EXPECT_EQ(sm2.data()[0], sm.data()[0]);
//  EXPECT_EQ(sm2.data()[1], sm.data()[1]);
//
//  //  fst::shared_memory sm3 = sm2;
//}
//} // namespace
