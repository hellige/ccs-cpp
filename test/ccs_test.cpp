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
