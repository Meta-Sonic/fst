#include <gtest/gtest.h>
#include "fst/binary_file.h"

namespace {
struct abc {
  int a, b, c;
};

struct abcd {};

void check_loader(fst::binary_file::loader& l) {
  fst::byte_view bv0 = l.get_data("a0");

  EXPECT_FALSE(bv0.empty());

  const abc& b0 = bv0.as_ref<abc>(0);

  EXPECT_EQ(b0.a, 0);
  EXPECT_EQ(b0.b, 1);
  EXPECT_EQ(b0.c, 2);

  fst::byte_view bv1 = l["a1"];
  EXPECT_FALSE(bv1.empty());

  const abc& b1 = bv1.as_ref<abc>(0);
  EXPECT_EQ(b1.a, 3);
  EXPECT_EQ(b1.b, 4);
  EXPECT_EQ(b1.c, 5);
}

TEST(binary_file, std) {
  abc a0 = { 0, 1, 2 };
  abc a1 = { 3, 4, 5 };

  fst::binary_file::writer w;
  w.add_chunk("a0", a0);
  w.add_chunk("a1", a1);

  EXPECT_EQ(w.add_chunk("a1", a1), false);
  EXPECT_EQ(w.add_chunk("a2", abcd{}), false);

  fst::byte_vector data = w.write_to_buffer();

  fst::binary_file::loader data_loader;
  EXPECT_TRUE(data_loader.load(data));
  check_loader(data_loader);

  EXPECT_TRUE(w.write_to_file(std::filesystem::temp_directory_path() / "data_file.data"));

  fst::binary_file::loader file_loader;
  EXPECT_TRUE(file_loader.load(std::filesystem::temp_directory_path() / "data_file.data"));
  check_loader(file_loader);
}
} // namespace
