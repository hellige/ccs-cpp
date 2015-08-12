#include <cstdlib>
#include <istream>
#include <sstream>

#include <gtest/gtest.h>

#include "ccs/ccs.h"

using namespace ccs;

TEST(ContextTest, StringFormat) {
  CcsDomain ccs;
  std::istringstream input("b c : @constrain d.e");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext root = ccs.build();
  CcsContext ctx = root.constrain("c").constrain("b");

  std::ostringstream os;
  os << ctx;
  EXPECT_EQ("c > b/d.e", os.str());

  os.str("");
  os << root;
  EXPECT_EQ("<root>", os.str());
}

TEST(ContextTest, StringFormatWithRootConstraint) {
  CcsDomain ccs;
  std::istringstream input("@constrain a.b; b c : @constrain d.e");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext root = ccs.build();
  CcsContext ctx = root.constrain("c").constrain("b");
  std::ostringstream os;
  os << ctx;
  EXPECT_EQ("a.b > c > b/d.e", os.str());
}

TEST(ContextTest, StringFormatWithEmptyContext) {
  CcsDomain ccs;
  std::istringstream input("b c : @constrain d.e");
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext root = ccs.build();
  CcsContext ctx = root.constrain("c").builder().build().constrain("b");
  std::ostringstream os;
  os << ctx;
  EXPECT_EQ("c > b/d.e", os.str());
}
