#include <gtest/gtest.h>

#include <array>
#include <fst/ui/rect>

namespace {
TEST(rect, constructor) {
  fst::ui::rect r0 = { 0, 2, 12, 24 };
  EXPECT_EQ(r0.x, 0);
  EXPECT_EQ(r0.position.x, 0);
}

} // namespace
