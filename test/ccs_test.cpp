#include <cstdlib>
#include <istream>

#include <gtest/gtest.h>

#include "ccs/ccs.h"

using namespace ccs;

namespace {

std::vector<std::string> v(const std::string &s) {
  return std::vector<std::string>{s};
}

}


TEST(CcsTest, Load) {
  CcsDomain ccs;
  std::istringstream input("foo.bar > baz.quux: frob = 'nitz'");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext root = ccs.build();
  CcsContext ctx = root.constrain("foo", v("bar")).constrain("baz", v("quux"));
  ASSERT_NO_THROW(EXPECT_EQ("nitz", ctx.getString("frob")));
}

TEST(CcsTest, Memory) {
  CcsDomain *ccs = new CcsDomain();
  std::istringstream input("foo.bar > baz.quux: frob = 'nitz'");
  ccs->loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext root = ccs->build();
  delete ccs;
  CcsContext ctx = root.constrain("foo", v("bar")).constrain("baz", v("quux"));
  ASSERT_NO_THROW(EXPECT_EQ("nitz", ctx.getString("frob")));
}

struct StringImportResolver : ccs::ImportResolver {
  std::string ccs;

  explicit StringImportResolver(const std::string &ccs) : ccs(ccs) {}

  virtual bool resolve(const std::string &,
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
  CcsContext ctx = ccs.build().constrain("baz", v("bar"));
  ASSERT_NO_THROW(EXPECT_EQ("nitz", ctx.getString("frob")));
}

TEST(CcsTest, Types) {
  CcsDomain ccs;
  std::istringstream input("a = 3; a2 = 0xff; b = true; c = 1.23; d = 'ok'");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext ctx = ccs.build();

  ASSERT_NO_THROW(EXPECT_EQ(3, ctx.getInt("a")));
  ASSERT_NO_THROW(EXPECT_EQ(3.0, ctx.getDouble("a")));
  ASSERT_THROW(ctx.getBool("a"), bad_coercion);
  ASSERT_NO_THROW(EXPECT_EQ("3", ctx.getString("a")));

  ASSERT_NO_THROW(EXPECT_EQ(255, ctx.getInt("a2")));
  ASSERT_NO_THROW(EXPECT_EQ(255.0, ctx.getDouble("a2")));
  ASSERT_THROW(ctx.getBool("a2"), bad_coercion);
  ASSERT_NO_THROW(EXPECT_EQ("255", ctx.getString("a2")));

  ASSERT_THROW(ctx.getInt("b"), bad_coercion);
  ASSERT_THROW(ctx.getDouble("b"), bad_coercion);
  ASSERT_NO_THROW(EXPECT_EQ(true, ctx.getBool("b")));
  ASSERT_NO_THROW(EXPECT_EQ("true", ctx.getString("b")));

  ASSERT_THROW(ctx.getInt("c"), bad_coercion);
  ASSERT_NO_THROW(EXPECT_EQ(1.23, ctx.getDouble("c")));
  ASSERT_THROW(ctx.getBool("c"), bad_coercion);
  ASSERT_NO_THROW(EXPECT_EQ("1.23", ctx.getString("c")));

  ASSERT_THROW(ctx.getInt("d"), bad_coercion);
  ASSERT_THROW(ctx.getDouble("d"), bad_coercion);
  ASSERT_THROW(ctx.getBool("d"), bad_coercion);
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

TEST(CcsTest, TemplatedBool) {
  CcsDomain ccs;
  std::istringstream input(
      "a = true; b = 'true'; c = false; d = 'false'; e = '1'");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext ctx = ccs.build();
  EXPECT_TRUE(ctx.get<bool>("a"));
  EXPECT_TRUE(ctx.get<bool>("b"));
  EXPECT_FALSE(ctx.get<bool>("c"));
  EXPECT_FALSE(ctx.get<bool>("d"));
  EXPECT_ANY_THROW(ctx.get<bool>("e"));
}

TEST(CcsTest, SameStep) {
  CcsDomain ccs;
  std::istringstream input("a.b.c d.e: test = 'nope'; a.b.c/d.e: test = 'yep'");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext ctx = ccs.build();
  ctx = ctx.builder()
      .add("a", std::vector<std::string>{"b", "c"})
      .add("d", std::vector<std::string>{"e"})
      .build();
  EXPECT_EQ("yep", ctx.getString("test"));
}

TEST(CcsTest, Interpolation) {
  CcsDomain ccs;
  setenv("VAR1", "bar", true);
  std::istringstream input("a = 'foo${VAR1} baz'; b = 'foo\\${VAR1} baz'");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext ctx = ccs.build();
  EXPECT_EQ("foobar baz", ctx.getString("a"));
  EXPECT_EQ("foo${VAR1} baz", ctx.getString("b"));
  setenv("VAR1", "quux", true);
  EXPECT_EQ("foobar baz", ctx.getString("a"));
  unsetenv("VAR1");
}

TEST(CcsTest, InterpolationIsDoneAtLoadTime) {
  CcsDomain ccs;
  std::istringstream input("a = 'foo${VAR1} baz'");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext ctx = ccs.build();
  EXPECT_EQ("foo baz", ctx.getString("a"));
  setenv("VAR1", "bar", true);
  EXPECT_EQ("foo baz", ctx.getString("a"));
  unsetenv("VAR1");
}

TEST(CcsTest, Escapes) {
  CcsDomain ccs;
  std::istringstream input(
      "a = '\\\"\\t\\n\\'s'; b = '\\\\'; c = 'Hi \\\nthere'");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext ctx = ccs.build();
  EXPECT_EQ("\"\t\n's", ctx.getString("a"));
  EXPECT_EQ("\\", ctx.getString("b"));
  EXPECT_EQ("Hi there", ctx.getString("c"));
}

TEST(CcsTest, DomainBuilder) {
  CcsDomain ccs;
  ccs.ruleBuilder()
      .set("a", "base")
      .select("a", v("b")).set("a", "123").pop()
      .select("c").set("b", "true").pop()
      .select("c", v("d")).set("b", "false").pop();
  CcsContext ctx = ccs.build();
  EXPECT_EQ("base", ctx.getString("a"));
  EXPECT_EQ("123", ctx.constrain("a", v("b")).getString("a"));
  EXPECT_EQ("base", ctx.constrain("c").getString("a"));
}

TEST(CcsTest, Coercion) {
  CcsDomain ccs;
  ccs.ruleBuilder()
      .set("int", "123")
      .set("double", "123.0")
      .set("bool", "true");
  CcsContext ctx = ccs.build();
  EXPECT_EQ(123, ctx.getInt("int"));
  EXPECT_EQ(123.0, ctx.getDouble("int"));
  EXPECT_EQ(123.0, ctx.getDouble("double"));
  EXPECT_EQ(true, ctx.getBool("bool"));
}

TEST(CcsTest, CoercionInterpolation) {
  setenv("F1", "23", true);
  setenv("F2", ".0", true);
  CcsDomain ccs;
  std::istringstream input(
      "int1 = '${F1}'; int2 = '1${F1}4'; double1 = '${F2}'; double2 = '1${F2}1'");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext ctx = ccs.build();
  EXPECT_EQ(23, ctx.getInt("int1"));
  EXPECT_EQ(1234, ctx.getInt("int2"));
  EXPECT_EQ(0, ctx.getDouble("double1"));
  EXPECT_EQ(1.01, ctx.getDouble("double2"));
}

