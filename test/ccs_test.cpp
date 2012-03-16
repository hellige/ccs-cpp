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

struct StringImportResolver : ccs::ImportResolver {
  std::string ccs;

  explicit StringImportResolver(const std::string &ccs) : ccs(ccs) {}

  virtual bool resolve(const std::string &location,
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
  CcsContext ctx = ccs.build().constrain("baz", {"bar"});
  ASSERT_NO_THROW(EXPECT_EQ("nitz", ctx.getString("frob")));
}
