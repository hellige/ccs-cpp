#include <istream>

#include <gtest/gtest.h>

#include "ccs/ccs.h"

using namespace ccs;

namespace ccs { class ImportResolver {}; } // TODO delete

// TODO
TEST(CcsTest, Load) {
  CcsDomain ccs;
  std::istringstream input("");
  ImportResolver resolver;
  ccs.loadCcsStream(input, "<literal>", resolver);
  EXPECT_TRUE(true);
}
