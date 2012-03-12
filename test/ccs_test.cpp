#include <istream>

#include <gtest/gtest.h>

#include "ccs/ccs.h"

using namespace ccs;

namespace ccs { class ImportResolver {}; } // TODO delete

TEST(CcsTest, Load) {
  CcsDomain ccs;
  std::istringstream input("foo.bar > baz.quux: frob = 'nitz'");
  ImportResolver resolver;
  ccs.loadCcsStream(input, "<literal>", resolver);
  CcsContext root = ccs.build();
  CcsContext ctx = root.constrain("foo", {"bar"}).constrain("baz", {"quux"});
  ASSERT_NO_THROW(EXPECT_EQ("nitz", ctx.getString("frob")));
}
