#include <stdexcept>

#include <gtest/gtest.h>

#include "parser/ast.h"
#include "parser/parser.h"

using namespace ccs;

struct P {
  Parser parser;
  bool parse(const std::string &input) {
    std::istringstream str(input);
    ast::Nested ast;
    return parser.parseCcsStream(str, ast);
  }
};

TEST(ParserTest, BasicPhrases) {
  P parser;
  EXPECT_TRUE(parser.parse(""));
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
  EXPECT_TRUE(parser.parse("foo {p = 1 x = 2}"));
  EXPECT_TRUE(parser.parse("foo{p=1;x=2}"));
  EXPECT_FALSE(parser.parse("foo{@localp=1}"));
  EXPECT_TRUE(parser.parse("foo{@local p=1}"));
  EXPECT_FALSE(parser.parse("foo{@overridep=1}"));
  EXPECT_FALSE(parser.parse("foo{@override@localp=1}"));
  EXPECT_TRUE(parser.parse("foo{@override @local p=1}"));
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
