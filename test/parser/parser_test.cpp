#include <stdexcept>

#include <gtest/gtest.h>

#include "parser/ast.h"
#include "parser/parser.h"

using namespace ccs;

namespace {

struct P {
  std::shared_ptr<CcsTracer> trace;
  Parser parser;
  P() :
    trace(CcsTracer::makeLoggingTracer(CcsLogger::makeStdErrLogger())),
    parser(*trace) {}
  bool parse(const std::string &input) {
    std::istringstream str(input);
    ast::Nested ast;
    return parser.parseCcsStream("<literal>", str, ast);
  }

  std::pair<bool, Value> parseAndReturnValue(const std::string &input) {
    std::istringstream str(input);
    ast::Nested ast;
    if (!parser.parseCcsStream("<literal>", str, ast)) return {false, Value()};
    if (ast.rules_.size() != 1) return {false, Value()};
    ast::PropDef *propDef = dynamic_cast<ast::PropDef*>(ast.rules_[0].get());
    if (!propDef) return {false, Value()};
    return {true, propDef->value_};
  }

  bool parseDouble(const std::string &input, double &out) {
    auto pr = parseAndReturnValue(input);
    if (!pr.first) return false;
    try {
      out = pr.second.asDouble();
    } catch (ccs::bad_coercion &) {
      return false;
    }
    return true;
  }

  bool parseInt(const std::string &input, int64_t &out) {
    auto pr = parseAndReturnValue(input);
    if (!pr.first) return false;
    try {
      out = pr.second.asInt();
    } catch (ccs::bad_coercion &) {
      return false;
    }
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
  EXPECT_FALSE(parser.parse("@context (foo x.bar > # baz >)"));
  EXPECT_TRUE(parser.parse("prop = 'val'"));
  EXPECT_TRUE(parser.parse("elem.id {}"));
  EXPECT_TRUE(parser.parse("elem.id {prop = 'val'}"));
  EXPECT_FALSE(parser.parse("elem.id {prop = @override 'hi'}"));
  EXPECT_TRUE(parser.parse("a.class.class blah > elem.id {prop=3}"));
  EXPECT_TRUE(parser.parse("a.class.class blah > elem.id {prop=2.3}"));
  EXPECT_TRUE(parser.parse("a.class.class blah > elem.id {prop=\"val\"}"));
  EXPECT_FALSE(parser.parse("a.class.class blah : elem.id { prop=\"val\" }"));
  EXPECT_FALSE(parser.parse("a.class.class blah elem.id prop=\"val\" }"));
  EXPECT_TRUE(parser.parse("a.class.class blah > elem.id {prop=0xAB12}"));
  EXPECT_TRUE(parser.parse("a.class.class blah > elem. id {prop=2.3}"));
  EXPECT_TRUE(parser.parse("a.class. class > elem.id {prop=\"val\"}"));
  EXPECT_FALSE(parser.parse("blah"));
  EXPECT_FALSE(parser.parse("@import 'file'; @context (foo)"));
  EXPECT_FALSE(parser.parse("@yuno?"));
  EXPECT_TRUE(parser.parse("@import 'file' ; @constrain foo"));
  EXPECT_TRUE(parser.parse("a.class { @import 'file' }"));
  EXPECT_FALSE(parser.parse("a.class { @context (foo) }"));
  EXPECT_TRUE(parser.parse("elem.id { prop = 'val'; prop2 = 31337 }"));
  EXPECT_TRUE(parser.parse("prop.'val'/a.foo/p.'hmm' { p = 1; }"));
  EXPECT_TRUE(parser.parse("a b > c d {p=1}"));
  EXPECT_TRUE(parser.parse("(a > b) (c > d) {p=1}"));
  EXPECT_TRUE(parser.parse("a > b > c {p=1}"));
  EXPECT_TRUE(parser.parse("a > (b c) > d {p=1}"));
  EXPECT_TRUE(parser.parse("a.\"foo\" 'bar' {'test' = 1}"));
}

TEST(ParserTest, Comments) {
  P parser;
  EXPECT_TRUE(parser.parse("// single line comment\n"));
  EXPECT_TRUE(parser.parse("// single line comment nonl"));
  EXPECT_TRUE(parser.parse("/* multi-line comment */"));
  EXPECT_TRUE(parser.parse("prop = /* comment */ 'val'"));
  EXPECT_TRUE(parser.parse("prop = /*/ comment */ 'val'"));
  EXPECT_TRUE(parser.parse("prop = /**/ 'val'"));
  EXPECT_TRUE(parser.parse("prop = /* comment /*nest*/ more */ 'val'"));
  EXPECT_TRUE(parser.parse("elem.id /* comment */ {prop = 'val'}"));
  EXPECT_FALSE(parser.parse("elem.id /* comment {prop = 'val'}"));
  EXPECT_TRUE(parser.parse("// comment\nelem { prop = 'val' prop = 'val' }"));
}

TEST(ParserTest, UglyAbutments) {
  P parser;
  EXPECT_FALSE(parser.parse("foo {p = 1x = 2}"));
  EXPECT_TRUE(parser.parse("foo {p = 1x p2 = 2}"));
  EXPECT_TRUE(parser.parse("foo {p = 'x'x = 2}"));
  EXPECT_FALSE(parser.parse("value=12asdf.foo {}"));
  EXPECT_TRUE(parser.parse("value=12asdf.foo nextsel {}"));
  EXPECT_TRUE(parser.parse("foo {p = 1 x = 2}"));
  EXPECT_TRUE(parser.parse("foo{p=1;x=2}"));
  EXPECT_FALSE(parser.parse("foo{@overridep=1}"));
  EXPECT_TRUE(parser.parse("foo{@override /*hi*/ p=1}"));
  EXPECT_TRUE(parser.parse("@import'asdf'"));
  EXPECT_FALSE(parser.parse("@constrainasdf"));
  EXPECT_TRUE(parser.parse(
      "@import 'asdf' \n ; \n @constrain asdf \n ; @import 'foo'  "));
  EXPECT_TRUE(parser.parse("@import /*hi*/ 'asdf'"));
  EXPECT_TRUE(parser.parse("env.foo/* some comment */{ }"));
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

TEST(ParserTest, StringsWithInterpolation) {
  P parser;
  EXPECT_TRUE(parser.parse("a = 'hi'"));
  EXPECT_FALSE(parser.parse("a = 'hi"));
  EXPECT_FALSE(parser.parse("a = 'hi\\"));
  EXPECT_FALSE(parser.parse("a = 'hi\\4 there'"));
  EXPECT_TRUE(parser.parse("a = 'h${there}i'"));
  EXPECT_FALSE(parser.parse("a = 'h$there}i'"));
  EXPECT_FALSE(parser.parse("a = 'h${t-here}i'"));
}

TEST(ParserTest, ParsesIntegers) {
  P parser;
  int64_t v64 = 0;
  ASSERT_TRUE(parser.parseInt("value = 100", v64));
  EXPECT_EQ(100, v64);
  ASSERT_TRUE(parser.parseInt("value = 0", v64));
  EXPECT_EQ(0, v64);
  ASSERT_TRUE(parser.parseInt("value = -0", v64));
  EXPECT_EQ(0, v64);
  ASSERT_TRUE(parser.parseInt("value = -100", v64));
  EXPECT_EQ(-100, v64);
  ASSERT_TRUE(parser.parseInt("value = 0x1a", v64));
  EXPECT_EQ(26, v64);
  ASSERT_TRUE(parser.parseInt("value = 0x1F", v64));
  EXPECT_EQ(31, v64);
  ASSERT_TRUE(parser.parseInt("value = 0x0", v64));
  EXPECT_EQ(0, v64);
  ASSERT_FALSE(parser.parseInt("value = 100.123", v64));
  ASSERT_TRUE(parser.parseInt("value = '100'", v64));
  EXPECT_EQ(100, v64);
}

TEST(ParserTest, ParsesDoubles) {
  P parser;
  double vDouble = 0.0;
  ASSERT_TRUE(parser.parseDouble("value = 100.", vDouble));
  EXPECT_DOUBLE_EQ(100., vDouble);
  ASSERT_TRUE(parser.parseDouble("value = 100.0000", vDouble));
  EXPECT_DOUBLE_EQ(100., vDouble);
  ASSERT_TRUE(parser.parseDouble("value = 0.0000", vDouble));
  EXPECT_DOUBLE_EQ(0., vDouble);
  ASSERT_TRUE(parser.parseDouble("value = -0.0000", vDouble));
  EXPECT_DOUBLE_EQ(0., vDouble);
  ASSERT_TRUE(parser.parseDouble("value = 1.0e-2", vDouble));
  EXPECT_DOUBLE_EQ(0.01, vDouble);
  ASSERT_TRUE(parser.parseDouble("value = 1.0E-2", vDouble));
  EXPECT_DOUBLE_EQ(0.01, vDouble);
  ASSERT_TRUE(parser.parseDouble("value = 1e-2", vDouble));
  EXPECT_DOUBLE_EQ(0.01, vDouble);
  ASSERT_TRUE(parser.parseDouble("value = 1E-2", vDouble));
  EXPECT_DOUBLE_EQ(0.01, vDouble);
  ASSERT_TRUE(parser.parseDouble("value = 100", vDouble));
  EXPECT_DOUBLE_EQ(100, vDouble);
  ASSERT_TRUE(parser.parseDouble("value = '100.0'", vDouble));
  EXPECT_DOUBLE_EQ(100, vDouble);
}
