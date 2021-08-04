#include <fst/assert>
#include <fst/print>
#include <fst/final_action.h>

int main() {
  auto ff = fst::final_action([]() { fst::print("Bango"); });

  auto fff = fst::final_action([]() { fst::print("Bongo"); });

  //  auto f = fst::final_action([]() { fst::print("Bingo"); });

  auto f = fst::final_action([]() {

  });

  auto gg = std::move(ff);

  fst::print("Banana");

  return 0;
}
