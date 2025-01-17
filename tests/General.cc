#include <iostream>

#include "gtest/gtest.h"

#include "sigs.h"

TEST(General, instantiate)
{
  sigs::Signal<void()> s;
  sigs::Signal<void(int)> s2;
  sigs::Signal<int()> s3;
}

inline void addOne(int &i)
{
  i++;
}

TEST(General, function)
{
  sigs::Signal<void(int &)> s;
  s.connect(addOne);

  int i = 0;
  s(i);

  EXPECT_EQ(i, 1);
}

TEST(General, multipleFunctions)
{
  sigs::Signal<void(int &)> s;
  s.connect(addOne);
  s.connect(addOne);
  s.connect(addOne);

  int i = 0;
  s(i);

  EXPECT_EQ(i, 3);
}

TEST(General, functor)
{
  class AddOneFunctor {
  public:
    void operator()(int &i) const
    {
      i++;
    }
  };

  sigs::Signal<void(int &)> s;
  s.connect(AddOneFunctor());

  int i = 0;
  s(i);

  EXPECT_EQ(i, 1);
}

TEST(General, instanceMethod)
{
  class Foo {
  public:
    void test(int &i) const
    {
      i++;
    }
  };

  sigs::Signal<void(int &)> s;

  Foo foo;
  s.connect(&foo, &Foo::test);

  int i = 0;
  s(i);

  EXPECT_EQ(i, 1);
}

TEST(General, lambda)
{
  sigs::Signal<void(int &)> s;
  s.connect([](int &i) { i++; });

  int i = 0;
  s(i);
  EXPECT_EQ(i, 1);
}

TEST(General, connectionDisconnectDirectly)
{
  sigs::Signal<void(int &)> s;
  auto conn = s.connect([](int &i) { i++; });

  int i = 0;
  s(i);
  EXPECT_EQ(i, 1);

  conn->disconnect();

  s(i);
  EXPECT_EQ(i, 1);
}

TEST(General, connectionDisconnectOnSignal)
{
  sigs::Signal<void(int &)> s;
  auto conn = s.connect([](int &i) { i++; });

  int i = 0;
  s(i);
  EXPECT_EQ(i, 1);

  s.disconnect(conn);

  s(i);
  EXPECT_EQ(i, 1);
}

TEST(General, connectSignalToSignal)
{
  sigs::Signal<void(int &)> s1;
  s1.connect([](int &i) { i++; });

  decltype(s1) s2;
  s2.connect(s1);

  int i = 0;
  s2(i);
  EXPECT_EQ(i, 1);
}

TEST(General, disconnectSignalFromSignal)
{
  sigs::Signal<void(int &)> s1;
  s1.connect([](int &i) { i++; });

  decltype(s1) s2;
  s2.connect(s1);
  s2.disconnect(s1);

  int i = 0;
  s2(i);
  EXPECT_EQ(i, 0);
}

// Check for debug assertion.
#ifndef NDEBUG
TEST(General, disconnectSignalFromSelf)
{
  sigs::Signal<void()> s;
  EXPECT_DEATH(s.disconnect(s), "Disconnecting from self has no effect.");
}
#endif

TEST(General, ambiguousMembers)
{
  class Ambiguous {
  public:
    void foo(int i)
    {
      value += i;
    }

    void foo(float j)
    {
      value += static_cast<int>(j);
    }

    int value = 0;
  };

  sigs::Signal<void(int)> s;

  Ambiguous amb;
  auto conn = s.connect(&amb, sigs::Use<int>::overloadOf(&Ambiguous::foo));
  s(42);
  EXPECT_EQ(amb.value, 42);
  conn->disconnect();

  // This one only works because int can be coerced into float.
  s.connect(&amb, sigs::Use<float>::overloadOf(&Ambiguous::foo));
  s(1);
  EXPECT_EQ(amb.value, 43);
}

TEST(General, returnValues)
{
  sigs::Signal<int()> s;
  s.connect([] { return 1; });
  s.connect([] { return 2; });
  s.connect([] { return 3; });

  int sum = 0;
  s([&sum](int retVal) { sum += retVal; });

  EXPECT_EQ(sum, 1 + 2 + 3);
}

TEST(General, returnValuesWithSignals)
{
  sigs::Signal<int()> s, s2, s3;
  s3.connect([] { return 1; });
  s2.connect([] { return 2; });
  s2.connect([] { return 3; });
  s.connect(s2);
  s.connect(s3);
  s.connect([] { return 4; });

  int sum = 0;
  s([&sum](int retVal) { sum += retVal; });

  EXPECT_EQ(sum, 1 + 2 + 3 + 4);
}

TEST(General, returnValuesBlocked)
{
  sigs::Signal<int()> s;
  s.connect([] { return 1; });
  s.setBlocked(true);

  int sum = 0;
  s([&sum](int retVal) { sum += retVal; });

  EXPECT_EQ(sum, 0);
}

TEST(General, sameSlotManyConnections)
{
  int calls = 0;
  const auto slot = [&calls] { calls++; };

  sigs::Signal<void()> s;
  s.connect(slot);
  s();
  EXPECT_EQ(calls, 1);

  s.connect(slot);
  s();
  EXPECT_EQ(calls, 3);

  // This yielded 4 calls when eraseEntries() didn't clear correctly.
  s.clear();
  s();
  EXPECT_EQ(calls, 3);
}

TEST(General, clearEquivalentToAllDisconnects)
{
  int calls = 0;
  const auto slot = [&calls] { calls++; };

  sigs::Signal<void()> s;

  {
    calls = 0;
    auto conn1 = s.connect(slot);
    auto conn2 = s.connect(slot);
    s();
    EXPECT_EQ(calls, 2);

    s.clear();
    s();
    EXPECT_EQ(calls, 2);
  }

  {
    calls = 0;
    auto conn1 = s.connect(slot);
    auto conn2 = s.connect(slot);
    s();
    EXPECT_EQ(calls, 2);

    s.disconnect(conn1);
    s.disconnect(conn2);
    s();
    EXPECT_EQ(calls, 2);
  }
}

TEST(General, size)
{
  sigs::Signal<void()> s;
  ASSERT_EQ(s.size(), 0);

  s.connect([] {});
  s.connect([] {});
  ASSERT_EQ(s.size(), 2);

  s.clear();
  ASSERT_EQ(s.size(), 0);
}

TEST(General, empty)
{
  sigs::Signal<void()> s;
  ASSERT_TRUE(s.empty());

  s.connect([] {});
  ASSERT_FALSE(s.empty());

  s.clear();
  ASSERT_TRUE(s.empty());
}

TEST(General, disconnectWithNoSlotClearsAll)
{
  sigs::Signal<void()> s;
  s.connect([] {});
  s.connect([] {});
  s.disconnect();
  ASSERT_TRUE(s.empty());
}

TEST(General, blocked)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.blocked());
  s.setBlocked(true);
  ASSERT_TRUE(s.blocked());
}

TEST(General, blockedPreviousValue)
{
  sigs::Signal<void()> s;
  ASSERT_FALSE(s.setBlocked(true));
  ASSERT_TRUE(s.setBlocked(true));
}

TEST(General, blockedSlots)
{
  int calls = 0;
  const auto slot = [&calls] { calls++; };

  sigs::Signal<void()> s;
  s.connect(slot);

  s();
  ASSERT_EQ(calls, 1);

  s.setBlocked(true);
  ASSERT_TRUE(s.blocked());

  s();
  ASSERT_EQ(calls, 1);

  s.setBlocked(false);
  ASSERT_FALSE(s.blocked());

  s();
  ASSERT_EQ(calls, 2);
}

TEST(General, blockedSignals)
{
  int calls = 0;
  const auto slot = [&calls] { calls++; };

  sigs::Signal<void()> s, s2;
  s2.connect(slot);
  s.connect(s2);

  s();
  ASSERT_EQ(calls, 1);

  // Block outer signal.
  s.setBlocked(true);
  ASSERT_TRUE(s.blocked());
  ASSERT_FALSE(s2.blocked());

  s();
  ASSERT_EQ(calls, 1);

  s.setBlocked(false);
  ASSERT_FALSE(s.blocked());

  s();
  ASSERT_EQ(calls, 2);

  // Block inner signal.
  s2.setBlocked(true);
  ASSERT_TRUE(s2.blocked());
  ASSERT_FALSE(s.blocked());

  s();
  ASSERT_EQ(calls, 2);
}

TEST(General, copyConstructible)
{
  sigs::Signal<void()> s;
  s.connect([] {});
  s.connect([] {});
  s.setBlocked(true);

  decltype(s) s2(s);
  ASSERT_EQ(s2.size(), 2);
  ASSERT_TRUE(s2.blocked());
}

TEST(General, copyAssignable)
{
  sigs::Signal<void()> s;
  s.connect([] {});
  s.connect([] {});
  s.setBlocked(true);

  decltype(s) s2;
  s2 = s;
  ASSERT_EQ(s2.size(), 2);
  ASSERT_TRUE(s2.blocked());
}
