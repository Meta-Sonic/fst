#include <gtest/gtest.h>

#include "fst/version.h"

namespace {
TEST(version, constructor) {
  {
    fst::version v(0, 0, 1);
    std::string sv = v.to_string();
    EXPECT_EQ(sv, "0.0.1");
  }

  {
    fst::version v(0, 0, 1, fst::version_tag::alpha);
    std::string sv = v.to_string();
    EXPECT_EQ(sv, "0.0.1-alpha");
  }

  {
    fst::version v(0, 0, 1, fst::version_tag::alpha, 1);
    std::string sv = v.to_string();
    EXPECT_EQ(sv, "0.0.1-alpha.1");
  }

  {
    fst::version v(12, 0, 1, fst::version_tag::oem, 32);
    std::string sv = v.to_string();
    EXPECT_EQ(sv, "12.0.1-oem.32");
  }

  {
    constexpr fst::version v = { 1, 2, 3 };
    std::string sv = v.to_string();
    EXPECT_EQ(sv, "1.2.3");
  }

  {
    constexpr fst::version v1 = { 1, 2, 3 };
    constexpr fst::version v2 = { 1, 2, 3 };
    EXPECT_EQ(v1, v2);
  }

  {
    constexpr fst::version v1 = { 1, 2, 3 };
    constexpr fst::version v2 = { 0, 2, 3 };
    EXPECT_TRUE(v1 != v2);
  }

  {
    constexpr fst::version v1 = { 1, 2, 3 };
    std::string sv1 = v1;
    EXPECT_EQ(v1.to_string(), "1.2.3");
    EXPECT_EQ(sv1, "1.2.3");
  }

  {
    fst::version va("0.0.1-alpha");
    EXPECT_EQ(va.tag, fst::version_tag::alpha);

    fst::version vb("0.0.1-beta");
    EXPECT_EQ(vb.tag, fst::version_tag::beta);

    fst::version vo("0.0.1-oem.3");
    EXPECT_EQ(vo.tag, fst::version_tag::oem);
    EXPECT_EQ(vo.tag_number, 3);
  }
}
} // namespace
