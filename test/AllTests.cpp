#include <cstdlib>
#include <gtest/gtest.h>

class TeamCityPrinter : public ::testing::EmptyTestEventListener {
  virtual void OnTestCaseStart(const ::testing::TestCase& test_case) {
    printf("##teamcity[testSuiteStarted name='%s']\n", test_case.name());
  }

  virtual void OnTestCaseEnd(const ::testing::TestCase& test_case) {
    printf("##teamcity[testSuiteFinished name='%s']\n", test_case.name());
  }

  virtual void OnTestStart(const ::testing::TestInfo& test_info) {
    printf("##teamcity[testStarted name='%s']\n", test_info.name());
  }

  virtual void OnTestEnd(const ::testing::TestInfo& test_info) {
    if (test_info.result()->Failed())
      printf("##teamcity[testFailed name='%s']\n", test_info.name());
    printf("##teamcity[testFinished name='%s']\n", test_info.name());
  }
};

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::TestEventListeners& listeners =
      ::testing::UnitTest::GetInstance()->listeners();
  listeners.Append(new TeamCityPrinter);
  return RUN_ALL_TESTS();
}

