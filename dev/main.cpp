#include <fst/assert>
#include <fst/print>
#include <fst/binary_file.h>

struct abc {
  int a, b, c;
};

struct abcd {};

void check_loader(fst::binary_file::loader& l) {
  fst::byte_view bv0 = l.get_data("a0");
  if (bv0.empty()) {
    fst::errprint("Cound not load a0.");
    return -1;
  }

  const abc& b0 = bv0.as_ref<abc>(0);
  fst::print(b0.a, b0.b, b0.c);

  fst::byte_view bv1 = l["a1"];
  if (bv1.empty()) {
    fst::errprint("Cound not load a1.");
    return -1;
  }

  const abc& b1 = bv1.as_ref<abc>(0);
  fst::print(b1.a, b1.b, b1.c);
}

int main() {
  abc a0 = { 0, 1, 2 };
  abc a1 = { 3, 4, 12 };

  fst::binary_file::writer w;
  w.add_chunk("a0", a0);
  w.add_chunk("a1", a1);

  fst::print("ABCD", w.add_chunk("a2", abcd{}));

  if (!w.write_to_file("/Users/alexarse/Desktop/data_file.data")) {
    fst::errprint("Cound not write data.");
    return -1;
  }

  fst::byte_vector data = w.write_to_buffer();

  fst::binary_file::loader data_loader;
  if (!data_loader.load(data)) {
    fst::errprint("Cound not load data.");
    return -1;
  }

  check_loader(data_loader);

  fst::binary_file::loader file_loader;
  if (!file_loader.load("/Users/alexarse/Desktop/data_file.data")) {
    fst::errprint("Cound not load data.");
    return -1;
  }

  check_loader(file_loader);
  return 0;
}
