#include "marjoram/maybe.hpp"
#include "gtest/gtest.h"

using ma::Maybe;
using ma::Just;
using ma::Nothing;

TEST(Maybe, flatMap) {
  auto five = Just(5);
  auto msqInts = [](int i) { return Just(i * i); };
  auto squared = five.flatMap(msqInts);
  auto twiceSquared = five.flatMap(msqInts).flatMap(msqInts);

  ASSERT_TRUE(squared.isJust());
  ASSERT_FALSE(squared.isNothing());
  EXPECT_EQ(squared.get(), 25);
  ASSERT_TRUE(twiceSquared.isJust());
  EXPECT_EQ(twiceSquared.get(), 25 * 25);
}

TEST(Maybe, map) {
  auto five = Just(5);
  auto sqInts = [](const int& i) { return i * i; };
  auto squared = five.map(sqInts);
  auto twiceSquared = five.map(sqInts).map(sqInts);

  ASSERT_TRUE(squared.isJust());
  ASSERT_FALSE(squared.isNothing());
  EXPECT_EQ(squared.get(), 25);
  ASSERT_TRUE(twiceSquared.isJust());
  EXPECT_EQ(twiceSquared.get(), 25 * 25);
}

Maybe<int> f(bool just) {
  if (just) {
    return Just(5);
  }
  return Nothing;
}

TEST(Maybe, getOrElse) {
  EXPECT_TRUE(f(true).isJust());
  auto something = f(true);
  auto nada = f(false);

  EXPECT_EQ(something.getOrElse(-42), 5);
  EXPECT_EQ(nada.getOrElse(-42), -42);
}

struct NoCopy_t {
  NoCopy_t() {
    hasBrains = true;
    newCount++;
  }
  NoCopy_t(const NoCopy_t&) = delete;
  NoCopy_t(NoCopy_t&& other) {
    other.hasBrains = false;
    hasBrains = true;
    moveCount++;
  }

  void quip() const { return; }

  bool hasBrains;
  static std::size_t moveCount;
  static std::size_t newCount;
};

std::size_t NoCopy_t::moveCount = 0;
std::size_t NoCopy_t::newCount = 0;

struct Needy_t {
  Needy_t(NoCopy_t&& t) {
    EXPECT_TRUE(t.hasBrains) << "Move ctor from invalid object";
  }
};

TEST(Maybe, NoCopyType) {
  auto nc = Just(NoCopy_t());
  auto minusOne = nc.flatMap([](NoCopy_t&&) { return Just(-1); });
  ASSERT_EQ(minusOne.get(), -1);

  ASSERT_EQ(NoCopy_t::newCount, 1LU);

  auto nc2 = Just(NoCopy_t());
  nc2.get().quip();
  ASSERT_EQ(NoCopy_t::newCount, 2LU);

  auto nc3 = Just(NoCopy_t());
  auto maybeNeedy =
      nc3.flatMap([](NoCopy_t&& ncc) { return Just(Needy_t(std::move(ncc))); });
  ASSERT_EQ(NoCopy_t::newCount, 3LU);
}

TEST(Maybe, For) {
  Maybe<int> five = Just(5);
  Maybe<int> notFive;

  for (int i : five) {
    ASSERT_EQ(i, 5);
  }

  auto b = notFive.begin();
  auto e = notFive.end();
  ASSERT_FALSE(b != e);

  for (int i : notFive) {
    ASSERT_FALSE(i == i);
  }
}

TEST(Maybe, ConstFor) {
  const Maybe<int> five = Just(5);
  const Maybe<int> notFive;

  for (int i : five) {
    ASSERT_EQ(i, 5);
  }

  auto b = notFive.begin();
  auto e = notFive.end();
  ASSERT_FALSE(b != e);

  for (int i : notFive) {
    ASSERT_FALSE(i == i);
  }
}

/* arguably this is a compile time test... oh well */
TEST(Maybe, ImmutableFor) {
  const Maybe<int> five = Just(5);

  for (auto& i : five) {
    using nonref_t = std::remove_reference_t<decltype(i)>;
    static_assert(std::is_const<nonref_t>::value,
                  "Contents of Maybe must be marked const");
  }
}

TEST(Maybe, NoCopyFor) {
  auto Mnc = Just(NoCopy_t());
  bool ran = false;
  for (auto& nc : Mnc) {
    (void)nc;  // avoid unused variable warning
    ran = true;
  }
  ASSERT_TRUE(ran);
}
