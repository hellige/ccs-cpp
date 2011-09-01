#include <stdexcept>

#include <gtest/gtest.h>

#include "Parser.h"

using namespace ccs;

TEST(ParserTest, BasicPhrases) {
  Parser parser;
  EXPECT_TRUE(parser.parseString(""));
  EXPECT_TRUE(parser.parseString("@import 'file'"));
  EXPECT_TRUE(parser.parseString("@context (foo #bar > baz)"));
  EXPECT_TRUE(parser.parseString("@context + (foo .bar > baz)"));
  EXPECT_TRUE(parser.parseString("prop = 'val'"));
  EXPECT_TRUE(parser.parseString("elem#id {}"));
  EXPECT_TRUE(parser.parseString("elem#id {prop = 'val'}"));
  EXPECT_TRUE(parser.parseString(".class.class blah > elem#id {prop=43}"));
  EXPECT_TRUE(parser.parseString(".class.class blah > elem#id {prop=2.3}"));
  EXPECT_TRUE(parser.parseString(".class.class blah > elem#id {prop=\"val\"}"));
  EXPECT_TRUE(parser.parseString(".class.class blah > elem#id {prop=0xAB12}"));
  EXPECT_FALSE(parser.parseString(".class.class blah > elem# id {prop=2.3}"));
  EXPECT_FALSE(parser.parseString(".class. class > elem#id {prop=\"val\"}"));
  EXPECT_FALSE(parser.parseString("blah"));
  EXPECT_FALSE(parser.parseString("@import 'file'; @context (foo)"));
  EXPECT_TRUE(parser.parseString(".class { @import 'file' }"));
  EXPECT_FALSE(parser.parseString(".class { @context + (foo) }"));
  EXPECT_TRUE(parser.parseString("elem#id { prop = 'val'; prop2 = 31337 }"));
  EXPECT_TRUE(parser.parseString("* > * foo *.blah { p = 1; }"));
  EXPECT_TRUE(parser.parseString("[prop='val'].foo[ p = 'hmm'] { p = 1; }"));
  EXPECT_TRUE(parser.parseString("a b + c d {p=1}"));
  EXPECT_TRUE(parser.parseString("(a b) + (c d) {p=1}"));
  EXPECT_TRUE(parser.parseString(".\"foo\" 'bar' {'test' = 1};"));
}

TEST(ParserTest, Comments) {
  Parser parser;
  EXPECT_TRUE(parser.parseString("// single line comment\n"));
  EXPECT_TRUE(parser.parseString("// single line comment nonl"));
  EXPECT_TRUE(parser.parseString("/* multi-line comment */"));
  EXPECT_TRUE(parser.parseString("prop = /* comment */ 'val'"));
  EXPECT_TRUE(parser.parseString("prop = /* comment /*nest*/ more */ 'val'"));
  EXPECT_TRUE(parser.parseString("elem#id /* comment */ {prop = 'val'}"));
  EXPECT_TRUE(parser.parseString("// comment\nelem { prop = 'val' prop = 'val' }"));
}

TEST(ParserTest, UglyAbutments) {
  Parser parser;
  EXPECT_FALSE(parser.parseString("foo {p = 1x = 2}"));
  EXPECT_TRUE(parser.parseString("foo {p = 1 x = 2}"));
  EXPECT_TRUE(parser.parseString("foo{p=1;x=2}"));
}

TEST(ParserTest, SelectorSections) {
  Parser parser;
  EXPECT_TRUE(parser.parseString("foo { + bar {}}"));
  EXPECT_TRUE(parser.parseString("foo { + bar + baz {}}"));
  EXPECT_TRUE(parser.parseString("+ bar > baz {}"));
  EXPECT_TRUE(parser.parseString(", bar + baz {}"));
  EXPECT_TRUE(parser.parseString("> bar + baz {}"));
}
