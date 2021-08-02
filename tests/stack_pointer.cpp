#include <gtest/gtest.h>

#include "fst/stack_pointer.h"
#include "fst/print.h"

namespace {
struct abc {
  int a, b, c;
};

TEST(stack_variable, copy) {

  fst::stack_variable<abc> a(abc{ 0, 1, 2 });
  fst::weak_stack_variable<abc> w(a);
  fst::stack_variable<abc> b = a;

  EXPECT_EQ(a.ref_count(), 1);
  EXPECT_EQ(b.ref_count(), 0);

  //  fst::stack_variable<abc> c(a);
  //    c = b;
}

TEST(stack_variable, constructor) {
  fst::stack_variable<abc> a(abc{ 1, 2, 3 });
  EXPECT_EQ(a->a, 1);
  EXPECT_EQ(a->b, 2);
  EXPECT_EQ(a->c, 3);

  fst::weak_stack_variable<abc> w(a);
  abc* ptr = w.get();
  EXPECT_NE(ptr, nullptr);
}

TEST(stack_variable, cpy) {
  fst::weak_stack_variable<abc> w;

  {
    fst::stack_variable<abc> a(abc{ 1, 2, 3 });
    EXPECT_EQ(a->a, 1);
    EXPECT_EQ(a->b, 2);
    EXPECT_EQ(a->c, 3);

    w = a;
    EXPECT_NE(w.get(), nullptr);
  }

  EXPECT_EQ(w.get(), nullptr);

  fst::weak_stack_variable<abc> w2 = w;
  EXPECT_EQ(w2.get(), nullptr);
}

TEST(stack_variable, ccc) {
  //  fst::stack_variable_unique_id uid = 0;
  {
    fst::weak_stack_variable<abc> w1;

    {
      fst::stack_variable<abc> a(abc{ 1, 2, 3 });
      fst::weak_stack_variable<abc> w2 = a;
      w1 = w2;
      //      uid = a.get_id();

      //      EXPECT_EQ(fst::stack_variable_manager::ref_count(a.get_id()), 0);
    }

    EXPECT_EQ(w1.get(), nullptr);
    //    EXPECT_EQ(fst::stack_variable_manager::ref_count(w1.get_id()), 1);

    EXPECT_EQ(fst::stack_variable_manager::size(), 1);
  }

  EXPECT_EQ(fst::stack_variable_manager::size(), 0);

  //  EXPECT_EQ(fst::stack_variable_manager::ref_count(uid), 0);
}

TEST(stack_variable, move) {
  //  fst::stack_variable_unique_id uid = 0;
  fst::weak_stack_variable<abc> w2;

  {
    fst::stack_variable<abc> a(abc{ 1, 2, 3 });
    //    uid = a.get_id();
    fst::weak_stack_variable<abc> w1 = a;
    w2 = std::move(w1);
    //    EXPECT_EQ(fst::stack_variable_manager::ref_count(a.get_id()), 0);
  }

  //  EXPECT_EQ(fst::stack_variable_manager::ref_count(w2.get_id()), 1);
  fst::weak_stack_variable<abc> w3 = w2;
  //  EXPECT_EQ(fst::stack_variable_manager::ref_count(w3.get_id()), 2);
}

class bingo {
public:
  fst::weak_stack_variable<abc> w;
};

TEST(stack_variable, bingo) {
  bingo c;
  {
    fst::stack_variable<abc> a(abc{ 0, 1, 2 });
    bingo b{ a };
    c.w = b.w;

    EXPECT_EQ(b.w->a, 0);
    EXPECT_EQ(b.w->b, 1);
    EXPECT_EQ(b.w->c, 2);

    EXPECT_NE(c.w.get(), nullptr);
  }

  EXPECT_EQ(c.w.get(), nullptr);
}

// TEST(stack_variable, copy) {
//    fst::stack_variable<abc> a(abc{0, 1, 2});
//    fst::stack_variable<abc> b = a;
//}
} // namespace
