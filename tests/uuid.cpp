#include <gtest/gtest.h>
#include "fst/uuid.h"

namespace {
TEST(uuid, constructor) {
  fst::uuid u0;
  EXPECT_EQ(u0.is_valid(), false);

  fst::uuid u1 = fst::uuid::create();
  EXPECT_EQ(u1.is_valid(), true);

  fst::uuid u2 = fst::uuid::from_string("47183823-2574-4bfd-b411-99ed177d3e43");
  fst::uuid u3 = fst::uuid::from_string("47183823-2574-4bfd-b411-99ed177d3e43");
  EXPECT_EQ(u2.is_valid(), true);
  EXPECT_EQ(u3.is_valid(), true);
  EXPECT_EQ(u2.to_string(), "47183823-2574-4bfd-b411-99ed177d3e43");
  EXPECT_EQ(u3.to_string(), "47183823-2574-4bfd-b411-99ed177d3e43");
  EXPECT_EQ(u2, u3);

  fst::uuid u4 = fst::uuid::create();
  fst::uuid u5 = fst::uuid::create();
  EXPECT_EQ(u4.is_valid(), true);
  EXPECT_EQ(u5.is_valid(), true);
  EXPECT_NE(u4, u5);
  u5 = u4;
  EXPECT_EQ(u4, u5);
}
} // namespace
