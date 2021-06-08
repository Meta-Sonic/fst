#include <fst/assert>
#include <fst/print>
#include <fst/uuid>

int main() {
  fst::print("Test");

  fst::uuid u0 = fst::uuid::create();
  fst::print("GG", u0);

  fst::print("FSLKJFS", fst::uuid::is_valid("717432bc-4f8e-4934-a302-e76fad0cfb45"));

  fst::uuid u1 = fst::uuid::from_string("c8119cad-bb6e-4f32-aecb-f662315473aa");
  fst::print("GG", u1);
  fst::print("FSLKJFS", u1.is_valid());

  fst::uuid u2;
  fst::print("GG", u2);
  fst::print("FSLKJFS", u2.is_valid());

  return 0;
}
