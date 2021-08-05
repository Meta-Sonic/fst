#include <fst/assert>
#include <fst/print>
#include <fst/enum_error.h>

enum class error_type { none, big_time, big_error };

inline fst::enum_error<error_type, error_type::none> check() { return error_type::big_error; }

int main() {
  if (auto res = check()) {
    fst::print("HAS ERROR", (int)(error_type)res);
  }

  fst::enum_error<error_type, error_type::none> r(error_type::big_time);

  if (r) {
    fst::print("Has error 0.");
  }

  r = error_type::none;

  if (r) {
    fst::print("Has error 1.");
  }

  r = error_type::big_time;

  if (r == error_type::big_time) {
    fst::print("SAME good");
  }
  return 0;
}
