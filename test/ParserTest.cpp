#include <stdexcept>

#include <gtest/gtest.h>

#include "Parser.h"

using namespace ccs;

TEST(ParserTest, BasicPhraseTests) {
  Parser parser;
  EXPECT_TRUE(parser.parseString(""));
  EXPECT_TRUE(parser.parseString("@include()"));
  EXPECT_TRUE(parser.parseString("prop=val"));
  EXPECT_TRUE(parser.parseString("elem#id{prop=val}"));
  EXPECT_TRUE(parser.parseString(".class.class>:pseudo>elem#id{prop=val}"));
  EXPECT_FALSE(parser.parseString("blah"));
}

