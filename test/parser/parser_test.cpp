#include <stdexcept>

#include <gtest/gtest.h>

#include "parser/ast.h"
#include "parser/parser.h"

using namespace ccs;

namespace {

struct P {
  Parser parser;
  P() : parser(CcsLogger::StdErr) {}
  bool parse(const std::string &input) {
    std::istringstream str(input);
    ast::Nested ast;
    return parser.parseCcsStream("<literal>", str, ast);
  }

  template<typename T>
  bool parseAndReturnValue(const std::string &input, T &out) {
    std::istringstream str(input);
    ast::Nested ast;
    if (!parser.parseCcsStream("<literal>", str, ast)) return false;
    if (ast.rules_.size() != 1) return false;
    ast::PropDef *propDef = boost::get<ast::PropDef>(&ast.rules_[0]);
    if (!propDef) return false;
    T *v = boost::get<T>(&propDef->value_.val_);
    if (!v) return false;
    out = *v;
    return true;
  }
};

}

TEST(ParserTest, BasicPhrases) {
  P parser;
  EXPECT_TRUE(parser.parse(""));
  EXPECT_TRUE(parser.parse("\n"));
  EXPECT_TRUE(parser.parse("@import 'file'"));
  EXPECT_TRUE(parser.parse("@context (foo.bar > baz)"));
  EXPECT_TRUE(parser.parse("@context (foo x.bar > baz >)"));
  EXPECT_TRUE(parser.parse("prop = 'val'"));
  EXPECT_TRUE(parser.parse("elem.id {}"));
  EXPECT_TRUE(parser.parse("elem.id {prop = 'val'}"));
  EXPECT_TRUE(parser.parse("a.class.class blah > elem.id {prop=43}"));
  EXPECT_TRUE(parser.parse("a.class.class blah > elem.id {prop=2.3}"));
  EXPECT_TRUE(parser.parse("a.class.class blah > elem.id {prop=\"val\"}"));
  EXPECT_TRUE(parser.parse("a.class.class blah > elem.id {prop=0xAB12}"));
  EXPECT_FALSE(parser.parse("a.class.class blah > elem. id {prop=2.3}"));
  EXPECT_FALSE(parser.parse("a.class. class > elem.id {prop=\"val\"}"));
  EXPECT_FALSE(parser.parse("blah"));
  EXPECT_FALSE(parser.parse("@import 'file'; @context (foo)"));
  EXPECT_TRUE(parser.parse("@import 'file' ; @constrain foo"));
  EXPECT_TRUE(parser.parse("a.class { @import 'file' }"));
  EXPECT_FALSE(parser.parse("a.class { @context (foo) }"));
  EXPECT_TRUE(parser.parse("elem.id { prop = 'val'; prop2 = 31337 }"));
  EXPECT_TRUE(parser.parse("prop.'val'/a.foo/p.'hmm' { p = 1; }"));
  EXPECT_TRUE(parser.parse("a b > c d {p=1}"));
  EXPECT_TRUE(parser.parse("(a > b) (c > d) {p=1}"));
  EXPECT_TRUE(parser.parse("a > (b c) > d {p=1}"));
  EXPECT_TRUE(parser.parse("a.\"foo\" 'bar' {'test' = 1};"));
}

TEST(ParserTest, Comments) {
  P parser;
  EXPECT_TRUE(parser.parse("// single line comment\n"));
  EXPECT_TRUE(parser.parse("// single line comment nonl"));
  EXPECT_TRUE(parser.parse("/* multi-line comment */"));
  EXPECT_TRUE(parser.parse("prop = /* comment */ 'val'"));
  EXPECT_TRUE(parser.parse("prop = /* comment /*nest*/ more */ 'val'"));
  EXPECT_TRUE(parser.parse("elem.id /* comment */ {prop = 'val'}"));
  EXPECT_TRUE(parser.parse("// comment\nelem { prop = 'val' prop = 'val' }"));
}

TEST(ParserTest, UglyAbutments) {
  P parser;
  EXPECT_FALSE(parser.parse("foo {p = 1x = 2}"));
  EXPECT_FALSE(parser.parse("foo {p = 'x'x = 2}"));
  EXPECT_FALSE(parser.parse("value=12env.foo {}"));
  EXPECT_TRUE(parser.parse("foo {p = 1 x = 2}"));
  EXPECT_TRUE(parser.parse("foo{p=1;x=2}"));
  EXPECT_FALSE(parser.parse("foo{@overridep=1}"));
  EXPECT_TRUE(parser.parse("foo{@override /*hi*/ p=1}"));
  EXPECT_FALSE(parser.parse("@import'asdf'"));
  EXPECT_FALSE(parser.parse("@constrainasdf"));
  EXPECT_TRUE(parser.parse(
      "@import 'asdf' \n ; \n @constrain asdf \n ; @import 'foo'  "));
  EXPECT_TRUE(parser.parse("@import /*hi*/ 'asdf'"));
}

TEST(ParserTest, SelectorSections) {
  P parser;
  EXPECT_TRUE(parser.parse("foo > { bar {}}"));
  EXPECT_TRUE(parser.parse("foo > { bar > baz {}}"));
  EXPECT_TRUE(parser.parse("bar > baz {}"));
  EXPECT_TRUE(parser.parse("bar baz {}"));
}

TEST(ParserTest, Constraints) {
  P parser;
  EXPECT_TRUE(parser.parse("a.b: @constrain a.c"));
}


TEST(ParserTest, ParsesIntegers) {
  P parser;
  int64_t v64 = 0;
  ASSERT_TRUE(parser.parseAndReturnValue("value = 100", v64));
  EXPECT_EQ(100, v64);
  ASSERT_TRUE(parser.parseAndReturnValue("value = 0", v64));
  EXPECT_EQ(0, v64);
  ASSERT_TRUE(parser.parseAndReturnValue("value = -0", v64));
  EXPECT_EQ(0, v64);
  ASSERT_TRUE(parser.parseAndReturnValue("value = -100", v64));
  EXPECT_EQ(-100, v64);
  ASSERT_FALSE(parser.parseAndReturnValue("value = 100.123", v64));
  ASSERT_FALSE(parser.parseAndReturnValue("value = '100", v64));
}

TEST(ParserTest, ParsesDoubles) {
  P parser;
  double vDouble = 0.0;
  ASSERT_TRUE(parser.parseAndReturnValue("value = 100.", vDouble));
  EXPECT_DOUBLE_EQ(100., vDouble);
  ASSERT_TRUE(parser.parseAndReturnValue("value = 100.0000", vDouble));
  EXPECT_DOUBLE_EQ(100., vDouble);
  ASSERT_TRUE(parser.parseAndReturnValue("value = 0.0000", vDouble));
  EXPECT_DOUBLE_EQ(0., vDouble);
  ASSERT_TRUE(parser.parseAndReturnValue("value = -0.0000", vDouble));
  EXPECT_DOUBLE_EQ(0., vDouble);
  ASSERT_TRUE(parser.parseAndReturnValue("value = 1.0e-2", vDouble));
  EXPECT_DOUBLE_EQ(0.01, vDouble);
  ASSERT_TRUE(parser.parseAndReturnValue("value = 1.0E-2", vDouble));
  EXPECT_DOUBLE_EQ(0.01, vDouble);
  ASSERT_TRUE(parser.parseAndReturnValue("value = 1e-2", vDouble));
  EXPECT_DOUBLE_EQ(0.01, vDouble);
  ASSERT_TRUE(parser.parseAndReturnValue("value = 1E-2", vDouble));
  EXPECT_DOUBLE_EQ(0.01, vDouble);
  ASSERT_FALSE(parser.parseAndReturnValue("value = 100", vDouble));
  ASSERT_FALSE(parser.parseAndReturnValue("value = '100.0", vDouble));
}
