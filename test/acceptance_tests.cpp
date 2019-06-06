#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ccs/ccs.h"

using ccs::ImportResolver;
using ccs::CcsDomain;
using ccs::CcsContext;
using std::string;

namespace {

struct Assertion {
  typedef std::pair<string, std::vector<string>> NameVals;
  typedef std::vector<std::vector<NameVals>> C;
  C constraints;
  string property;
  string value;
};

struct CcsTestCase {
  string name;
  string ccs;
  std::vector<Assertion> assertions;
};

std::ostream &operator<<(std::ostream &os, const CcsTestCase &test) {
  return os << test.name;
}

void expect(std::istream &reader, const std::string &expect) {
  std::string line;
  ASSERT_TRUE(std::getline(reader, line));
  ASSERT_EQ(expect, line);
}

template <typename F>
void doUntil(std::istream &reader, const std::string &delim, F &&f) {
  std::string line;
  while (std::getline(reader, line) && line != delim)
    f(line);
}

std::string readUntil(std::istream &reader, const std::string &delim) {
  std::ostringstream result;
  doUntil(reader, delim, [&](auto &&str) { result << str << "\n"; });
  return result.str();
}

std::string trim(const std::string &str) {
  size_t first = str.find_first_not_of(' ');
  if (string::npos == first) return str;
  size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}

Assertion::NameVals parseElem(std::string elem) {
  Assertion::NameVals nameVals;
  size_t dot;
  auto seenName = false;
  do {
    dot = elem.find('.');
    auto word = elem.substr(0, dot);
    elem = elem.substr(dot+1);
    if (seenName) {
      nameVals.second.push_back(trim(word));
    } else {
      nameVals.first = trim(word);
      seenName = true;
    }
  } while (dot != string::npos);
  if (!seenName)
    throw std::runtime_error("Malformed assertion");
  return nameVals;
}

Assertion::C::value_type parseStep(std::string step) {
  Assertion::C::value_type result;
  size_t slash;
  do {
    slash = step.find('/');
    result.push_back(parseElem(step.substr(0, slash)));
    step = step.substr(slash+1);
  } while (slash != string::npos);
  return result;
}

Assertion parseAssertion(const std::string &line) {
  Assertion result;
  auto colon = line.find(':');
  if (colon != string::npos) {
    auto ctx = line.substr(0, colon);
    size_t space;
    do {
      space = ctx.find(' ');
      result.constraints.push_back(
        parseStep(ctx.substr(0, space)));
      ctx = ctx.substr(space+1);
    } while (space != string::npos);
  }

  auto rest = line.substr(colon+1);
  auto eq = rest.find('=');
  if (eq == string::npos)
    throw std::runtime_error("Malformed assertion");

  result.property = trim(rest.substr(0, eq));
  result.value = trim(rest.substr(eq+1));

  return result;
}

bool parseTestCase(std::istream &reader,
    std::vector<CcsTestCase> &cases) {
  std::string name;
  if (!std::getline(reader, name)) return false;

  expect(reader, "---");
  auto ccs = readUntil(reader, "---");
  std::vector<Assertion> assertions;
  doUntil(reader, "===", [&](auto &&str) {
    assertions.emplace_back(parseAssertion(str));
  });
  expect(reader, "");
  cases.push_back(CcsTestCase {name, ccs, assertions});
  return true;
}

class AcceptanceTests : public ::testing::TestWithParam<CcsTestCase> {
public:
  static std::vector<CcsTestCase> loadValues() {
    std::ifstream cases;
    cases.open("tests.txt", std::ifstream::in);
    return parseStream(cases);
  }

  static std::vector<CcsTestCase> parseStream(std::istream &stream) {
    stream.unsetf(std::ios::skipws);

    std::vector<CcsTestCase> result;
    while (!stream.eof() && parseTestCase(stream, result));
    return result;

    // std::cout << "Parsing failed, stopped at: \"" << rest << "\"\n";
    // throw std::runtime_error("Couldn't parse test cases.");
  }
};

TEST_P(AcceptanceTests, Load) {
  const CcsTestCase &test = GetParam();
  std::cout << "Running test: " << test.name << std::endl;
  CcsDomain ccs;
  std::istringstream input(test.ccs);
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext root = ccs.build();

  //ccs.logRuleDag(std::cout);
  //std::cout << std::endl;

  for (auto it = test.assertions.cbegin(); it != test.assertions.cend(); ++it) {
    CcsContext ctx = root;
    auto &cs = it->constraints;
    for (auto it2 = cs.cbegin(); it2 != cs.cend(); ++it2) {
      CcsContext::Builder b = ctx.builder();
      for (auto it3 = it2->cbegin(); it3 != it2->cend(); ++it3)
        b.add(it3->first, it3->second);
      ctx = b.build();
    }
    ASSERT_NO_THROW(EXPECT_EQ(it->value, ctx.getString(it->property)));
  }
}

}

INSTANTIATE_TEST_CASE_P(Run,
    AcceptanceTests,
    ::testing::ValuesIn(AcceptanceTests::loadValues()));
