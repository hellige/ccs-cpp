#include <stdexcept>

#include <gtest/gtest.h>

#include "Parser.h"

using namespace ccs;

TEST(ParserTest, BasicPhrases) {
  Parser parser;
  EXPECT_TRUE(parser.parseString(""));
  EXPECT_TRUE(parser.parseString("@import file"));
  EXPECT_TRUE(parser.parseString("prop = val"));
  EXPECT_TRUE(parser.parseString("elem#id {prop = val}"));
  EXPECT_TRUE(parser.parseString(".class.class :pseudo > elem#id {prop=val}"));
  EXPECT_FALSE(parser.parseString(".class.class :pseudo > elem# id {prop=val}"));
  EXPECT_FALSE(parser.parseString(".class. class :pseudo > elem#id {prop=val}"));
  EXPECT_FALSE(parser.parseString(".class.class : pseudo > elem#id {prop=val}"));
  EXPECT_FALSE(parser.parseString("blah"));
  EXPECT_TRUE(parser.parseString("elem#id { prop = val; prop2 = val }"));
}

TEST(ParserTest, Comments) {
  Parser parser;
  EXPECT_TRUE(parser.parseString("// single line comment"));
  EXPECT_TRUE(parser.parseString("/* multi-line comment */"));
  EXPECT_TRUE(parser.parseString("prop = /* comment */ val"));
  EXPECT_TRUE(parser.parseString("prop = /* comment /*nest*/ more */ val"));
  EXPECT_TRUE(parser.parseString("elem#id /* comment */ {prop = val}"));
  EXPECT_TRUE(parser.parseString("// comment\nelem { prop = val prop = val }"));
}

