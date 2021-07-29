#include <gtest/gtest.h>
#include "fst/listener.h"
#include "fst/small_vector.h"
#include <mutex>

class listener {
public:
  virtual ~listener() = default;
  virtual void on_action(std::size_t& count) = 0;
};

class listener_imp : public listener {
public:
  virtual ~listener_imp() = default;
  virtual void on_action(std::size_t& count) { count++; }
};

template <typename T>
using vector_type = fst::small_vector<T, 2>;

TEST(listener, bb) {

  fst::mt::listener_manager<listener, std::mutex, vector_type> listeners;
  EXPECT_TRUE(listeners.empty());
  EXPECT_EQ(listeners.size(), 0);

  listener_imp a;
  listener_imp b;
  listener_imp c;

  listeners.add(&a);
  EXPECT_EQ(listeners.size(), 1);

  listeners.add(&b);
  EXPECT_EQ(listeners.size(), 2);

  std::size_t count = 0;
  listeners.notify<&listener::on_action>(count);
  EXPECT_EQ(count, 2);

  listeners.remove(&c);
  EXPECT_EQ(listeners.size(), 2);

  EXPECT_EQ(listeners.get().capacity(), 2);

  listeners.add(&c);
  EXPECT_EQ(listeners.size(), 3);

//  EXPECT_TRUE(listeners.get().capacity() > 2);

  listeners.remove(&b);
  EXPECT_EQ(listeners.size(), 2);

  listeners.remove(&a);
  EXPECT_EQ(listeners.size(), 1);

  listeners.remove(&c);
  EXPECT_EQ(listeners.size(), 0);

  //    listeners.reset();
  //    EXPECT_EQ(listeners.get().capacity(), 2);
}

TEST(listener, vector) {
  fst::mt::listener_manager<listener, std::mutex> listeners;
  EXPECT_TRUE(listeners.empty());
  EXPECT_EQ(listeners.size(), 0);

  listener_imp a;
  listener_imp b;
  listener_imp c;

  listeners.add(&a);
  EXPECT_EQ(listeners.size(), 1);

  listeners.add(&b);
  EXPECT_EQ(listeners.size(), 2);

  std::size_t count = 0;
  listeners.notify<&listener::on_action>(count);
  EXPECT_EQ(count, 2);

  listeners.remove(&c);
  EXPECT_EQ(listeners.size(), 2);

//  EXPECT_EQ(listeners.get().capacity(), 2);

  listeners.add(&c);
  EXPECT_EQ(listeners.size(), 3);

//  EXPECT_EQ(listeners.get().capacity(), 4);

  listeners.remove(&b);
  EXPECT_EQ(listeners.size(), 2);

  listeners.remove(&a);
  EXPECT_EQ(listeners.size(), 1);

  listeners.remove(&c);
  EXPECT_EQ(listeners.size(), 0);

  listeners.reset();
//  EXPECT_EQ(listeners.get().capacity(), 0);
}
