#include <fst/assert>
#include <fst/print>
#include <span>

int main() {
  fst::print("Test");
  return 0;
}

//#include "fst/span.h"
//#include "fst/math.h"
//#include "fst/string_conv.h"
//#include "fst/unmanaged_string.h"
//#include <span>
// int main() {
//  std::array<char, 32> buffer;
//  fst::print(fst::string_conv::to_string<2>(buffer, 9.0f));
//  fst::print(fst::string_conv::to_string<1>(buffer, 51.0f));
//  fst::print(fst::string_conv::to_string<2>(buffer, -51.00f));
//  fst::print(fst::string_conv::to_string<2>(buffer, 51.00f));
//  fst::print(fst::string_conv::to_string<2>(buffer, 52.00f));
//  fst::print(fst::string_conv::to_string<2>(buffer, 50.00f));
//  fst::print(fst::string_conv::to_string<2>(buffer, 50.10f));
//  return 0;
//}
