#include <istream>

#include <gtest/gtest.h>

#include "ccs/ccs.h"

using namespace ccs;

TEST(CcsTest, Load) {
  CcsDomain ccs;
  std::istringstream input("foo.bar > baz.quux: frob = 'nitz'");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext root = ccs.build();
  CcsContext ctx = root.constrain("foo", {"bar"}).constrain("baz", {"quux"});
  ASSERT_NO_THROW(EXPECT_EQ("nitz", ctx.getString("frob")));
}

TEST(CcsTest, Memory) {
  CcsDomain *ccs = new CcsDomain();
  std::istringstream input("foo.bar > baz.quux: frob = 'nitz'");
  ccs->loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext root = ccs->build();
  delete ccs;
  CcsContext ctx = root.constrain("foo", {"bar"}).constrain("baz", {"quux"});
  ASSERT_NO_THROW(EXPECT_EQ("nitz", ctx.getString("frob")));
}

struct StringImportResolver : ccs::ImportResolver {
  std::string ccs;

  explicit StringImportResolver(const std::string &ccs) : ccs(ccs) {}

  virtual bool resolve(const std::string &location,
      std::function<bool(std::istream &)> load) {
    std::istringstream stream(ccs);
    return load(stream);
  }
};

TEST(CcsTest, Import) {
  CcsDomain ccs;
  std::istringstream input("@import 'foo'");
  StringImportResolver ir("baz.bar: frob = 'nitz'");
  ccs.loadCcsStream(input, "<literal>", ir);
  CcsContext ctx = ccs.build().constrain("baz", {"bar"});
  ASSERT_NO_THROW(EXPECT_EQ("nitz", ctx.getString("frob")));
}

TEST(CcsTest, Types) {
  CcsDomain ccs;
  std::istringstream input("a = 3; a2 = 0xff; b = true; c = 1.23; d = 'ok'");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext ctx = ccs.build();

  ASSERT_NO_THROW(EXPECT_EQ(3, ctx.getInt("a")));
  ASSERT_THROW(ctx.getDouble("a"), wrong_type);
  ASSERT_THROW(ctx.getBool("a"), wrong_type);
  ASSERT_NO_THROW(EXPECT_EQ("3", ctx.getString("a")));

  ASSERT_NO_THROW(EXPECT_EQ(255, ctx.getInt("a2")));
  ASSERT_THROW(ctx.getDouble("a2"), wrong_type);
  ASSERT_THROW(ctx.getBool("a2"), wrong_type);
  ASSERT_NO_THROW(EXPECT_EQ("255", ctx.getString("a2")));

  ASSERT_THROW(ctx.getInt("b"), wrong_type);
  ASSERT_THROW(ctx.getDouble("b"), wrong_type);
  ASSERT_NO_THROW(EXPECT_EQ(true, ctx.getBool("b")));
  ASSERT_NO_THROW(EXPECT_EQ("true", ctx.getString("b")));

  ASSERT_THROW(ctx.getInt("c"), wrong_type);
  ASSERT_NO_THROW(EXPECT_EQ(1.23, ctx.getDouble("c")));
  ASSERT_THROW(ctx.getBool("c"), wrong_type);
  ASSERT_NO_THROW(EXPECT_EQ("1.23", ctx.getString("c")));

  ASSERT_THROW(ctx.getInt("d"), wrong_type);
  ASSERT_THROW(ctx.getDouble("d"), wrong_type);
  ASSERT_THROW(ctx.getBool("d"), wrong_type);
  ASSERT_NO_THROW(EXPECT_EQ("ok", ctx.getString("d")));
}

namespace {

struct Foo {
  int x;
};

std::istream &operator>>(std::istream &str, Foo &foo) {
  str >> foo.x;
  return str;
}

}

TEST(CcsTest, TemplateGetters) {
  CcsDomain ccs;
  std::istringstream input("a = 3; b = 0x10");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext ctx = ccs.build();
  Foo foo = ctx.get<Foo>("a");
  EXPECT_EQ(3, foo.x);
  ASSERT_TRUE(ctx.getInto(foo, "b"));
  EXPECT_EQ(16, foo.x);
}
