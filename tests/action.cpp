#include <gtest/gtest.h>

#include "fst/action.h"

struct executor {
  fst::action_manager<executor> actions;

  inline void execute() { actions.execute(); }
};

namespace {
TEST(action, execute) {

  int a = 32;
  int b = 54;
  int c = 64;

  executor e;

  e.actions.add([&a]() { a = 100; });

  EXPECT_EQ(e.actions.size(), 1);

  e.actions.add([&b]() { b = 120; });

  EXPECT_EQ(e.actions.size(), 2);

  EXPECT_EQ(a, 32);
  EXPECT_EQ(b, 54);

  e.execute();

  EXPECT_EQ(e.actions.size(), 0);
  EXPECT_TRUE(e.actions.empty());
  EXPECT_EQ(a, 100);
  EXPECT_EQ(b, 120);

  // Add multiple action in one action.
  e.actions.add([&e, &a, &b, &c]() {
    a = 200;

    e.actions.add([&e, &b, &c]() {
      b = 400;

      e.actions.add([&c]() { c = 1001; });
    });
  });

  EXPECT_EQ(e.actions.size(), 1);
  e.execute();

  EXPECT_EQ(e.actions.size(), 0);
  EXPECT_EQ(a, 200);
  EXPECT_EQ(b, 400);
  EXPECT_EQ(c, 1001);

  // Add multiple action in one action but don't call right away.
  e.actions.add([&e, &a, &b]() {
    a = 500;

    e.actions.add([&b]() { b = 800; }, false);
  });

  EXPECT_EQ(e.actions.size(), 1);
  e.execute();

  EXPECT_EQ(e.actions.size(), 1);
  EXPECT_EQ(a, 500);
  EXPECT_EQ(b, 400);

  e.execute();
  EXPECT_EQ(e.actions.size(), 0);
  EXPECT_EQ(a, 500);
  EXPECT_EQ(b, 800);

  a = 1;
  b = 2;
  c = 3;

  // Add multiple action in one action.
  e.actions.add([&e, &a, &b, &c]() {
    a = 1000;

    e.actions.add(
        [&e, &b, &c]() {
          b = 1001;

          e.actions.add([&c]() { c = 1002; }, false);
        },
        false);
  });

  EXPECT_EQ(e.actions.size(), 1);

  e.execute();
  EXPECT_EQ(e.actions.size(), 1);
  EXPECT_EQ(a, 1000);
  EXPECT_EQ(b, 2);
  EXPECT_EQ(c, 3);

  e.execute();
  EXPECT_EQ(e.actions.size(), 1);
  EXPECT_EQ(a, 1000);
  EXPECT_EQ(b, 1001);
  EXPECT_EQ(c, 3);

  e.execute();
  EXPECT_EQ(e.actions.size(), 0);
  EXPECT_EQ(a, 1000);
  EXPECT_EQ(b, 1001);
  EXPECT_EQ(c, 1002);
}

TEST(action, construct) {
  int a = 1;
  //  int b = 2;
  //  int c = 3;

  executor e;
  fst::action act0([&a]() { a = 4; });

  e.actions.add(std::move(act0));

  act0.call();
  EXPECT_EQ(a, 1);

  e.execute();
  EXPECT_EQ(a, 4);
}
} // namespace
