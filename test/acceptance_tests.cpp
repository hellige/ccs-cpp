#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ccs/ccs.h"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

using ccs::ImportResolver;
using ccs::CcsDomain;
using ccs::CcsContext;
using std::string;
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

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

BOOST_FUSION_ADAPT_STRUCT(
    Assertion,
    (Assertion::C, constraints)
    (string, property)
    (string, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    CcsTestCase,
    (string, name)
    (string, ccs)
    (std::vector<Assertion>, assertions)
)

template <typename Iterator>
struct grammar : qi::grammar<Iterator, std::vector<CcsTestCase>()> {
  typedef Iterator I;

  qi::rule<I, std::vector<CcsTestCase>()> testcases;
  qi::rule<I, CcsTestCase()> testcase;
  qi::rule<I, string()> line, block, strng, ident;
  qi::rule<I, Assertion::NameVals()> namevals;
  qi::rule<I, std::vector<Assertion::NameVals>()> constraints;
  qi::rule<I, Assertion()> assertion;

  grammar() : grammar::base_type(testcases) {
    strng = qi::lexeme['\'' >> *(qi::char_ - ('\'' | qi::eol)) >> '\'']
           | qi::lexeme['"' >> *(qi::char_ - ('"' | qi::eol)) >> '"'];
    ident %= +qi::char_("A-Za-z0-9$_") | strng;
    auto sep = "---" >> qi::eol;
    line %= *(qi::print - qi::eol) >> qi::eol;
    block %= *(qi::char_ - sep);
    namevals = ident >> *("." >> ident);
    constraints = namevals >> *("/" >> namevals);
    assertion %= -(constraints % +qi::space >> ": ") >> ident >> " = " >> ident
        >> qi::eol;
    testcase = line >> sep >> block >> sep >> +assertion >> "===" >> +qi::eol;
    testcases %= *testcase >> qi::eoi;
  }
};

class AcceptanceTests : public ::testing::TestWithParam<CcsTestCase> {
public:
  static std::vector<CcsTestCase> loadValues() {
    std::ifstream cases;
    cases.open("tests.txt", std::ifstream::in);
    return parseStream(cases);
  }

  static std::vector<CcsTestCase> parseStream(std::istream &stream) {
    stream.unsetf(std::ios::skipws);

    boost::spirit::istream_iterator iter(stream);
    boost::spirit::istream_iterator end;

    grammar<typeof(iter)> grammar;
    std::vector<CcsTestCase> result;
    bool r = qi::parse(iter, end, grammar, result);

    if (r && iter == end) return result;

    string rest(iter, end);
    std::cout << "Parsing failed, stopped at: \"" << rest << "\"\n";
    throw std::runtime_error("Couldn't parse test cases.");
  }
};

TEST_P(AcceptanceTests, Load) {
  const CcsTestCase &test = GetParam();
  std::cout << "Running test: " << test.name << std::endl;
  CcsDomain ccs;
  std::istringstream input(test.ccs);
  ccs.loadCcsStream(input, "<literal>", ImportResolver::None);
  CcsContext root = ccs.build();

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

INSTANTIATE_TEST_CASE_P(Run,
    AcceptanceTests,
    ::testing::ValuesIn(AcceptanceTests::loadValues()));
