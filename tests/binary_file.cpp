#include <gtest/gtest.h>
#include "fst/binary_file.h"
#include "fst/small_vector.h"

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

  EXPECT_EQ(w.add_chunk("a1", a1), fst::binary_file::writer::error_type::duplicate_name);
  EXPECT_EQ(w.add_chunk("a2", abcd{}), fst::binary_file::writer::error_type::empty_data);

  fst::byte_vector data = w.write_to_buffer();

  fst::binary_file::loader data_loader;
  EXPECT_FALSE(data_loader.load(data));
  check_loader(data_loader);

  EXPECT_FALSE(w.write_to_file(std::filesystem::temp_directory_path() / "data_file.data"));

  fst::binary_file::loader file_loader;
  EXPECT_FALSE(file_loader.load(std::filesystem::temp_directory_path() / "data_file.data"));
  check_loader(file_loader);
}

TEST(binary_file, view) {

  abc a0 = { 0, 1, 2 };
  abc a1 = { 3, 4, 5 };

  fst::binary_file::writer w;
  w.add_chunk("a0", a0);
  w.add_chunk_ref("a1", a1);
  //  w.add_view("a1", fst::byte_view((const std::uint8_t*)&a1, sizeof(abc)));
  //
  //  EXPECT_EQ(w.add_chunk("a1", a1), false);
  //  EXPECT_EQ(w.add_chunk("a2", abcd{}), false);

  fst::byte_vector data = w.write_to_buffer();

  fst::binary_file::loader data_loader;
  EXPECT_FALSE(data_loader.load(data));
  check_loader(data_loader);
}

template <typename T>
using vector_type = fst::small_vector<T, 8>;

using writer_type = fst::binary_file::writer_t<vector_type>;

TEST(binary_file, custom_vector) {
  abc a0 = { 0, 1, 2 };
  abc a1 = { 3, 4, 5 };

  writer_type w;
  w.add_chunk("a0", a0);
  w.add_chunk("a1", a1);

  EXPECT_EQ(w.add_chunk("a1", a1), fst::binary_file::write_error::duplicate_name);
  EXPECT_EQ(w.add_chunk("a2", abcd{}), writer_type::error_type::empty_data);

  fst::byte_vector data = w.write_to_buffer();

  fst::binary_file::loader data_loader;
  EXPECT_FALSE(data_loader.load(data));
  check_loader(data_loader);

  EXPECT_FALSE(w.write_to_file(std::filesystem::temp_directory_path() / "data_file.data"));

  fst::binary_file::loader file_loader;
  EXPECT_FALSE(file_loader.load(std::filesystem::temp_directory_path() / "data_file.data"));
  check_loader(file_loader);
}
} // namespace
