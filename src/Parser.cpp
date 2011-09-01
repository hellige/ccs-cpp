#include "Parser.h"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <boost/spirit/include/qi.hpp>

#include "Node.h"

using namespace std;
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace ccs {

struct Selector {};
struct Rule {
  Selector selector;
  vector<Rule> nestedRules;
  unordered_map<string, Property> properties;
};

struct Ruleset {
  Selector context;
  vector<Rule> rules;
};


namespace {

template <typename Iterator>
struct ccs_grammar : qi::grammar<Iterator, qi::rule<Iterator>> {
  typedef Iterator I;
  typedef qi::rule<I> nospacerule;

  nospacerule blockComment;
  nospacerule skipper;

  typedef qi::rule<I, typeof(skipper)> spacerule;

  qi::rule<I, string()> ident;
  spacerule property;
  spacerule selector;
  nospacerule step;
  nospacerule stepsuffix;
  spacerule rule;
  spacerule ruleset;

  ccs_grammar() : ccs_grammar::base_type(ruleset, "CCS"),
    blockComment("block comment"),
    skipper("whitespace and comments"),
    ident("identifier"),
    property("property"),
    selector("selector"),
    step("selector step"),
    stepsuffix("selector step modifier"),
    rule("CCS rule"),
    ruleset("CCS ruleset")
  {
    using qi::lit;
    using qi::char_;
    using qi::space;
    using qi::eol;
    using qi::eoi;

    // comments and whitespace...
    blockComment = "/*" >> *(blockComment | (char_ - "*/")) >> "*/";
    skipper = blockComment
            | "//" >> *(char_ - eol) >> (eol | eoi)
            | space;

    auto string = qi::lexeme['\'' >> *(char_ - ('\'' | eol)) >> '\'']
                | qi::lexeme['"' >> *(char_ - ('"' | eol)) >> '"'];
    auto val = lit("0x") >> qi::hex
             | qi::long_long >> !lit('.')
             | qi::double_
             | string;
    ident %= +char_("A-Za-z0-9$_") | string;

    // properties...
    property = -lit("inherit") >> ident >> '=' >> qi::lexeme[val >> !ident];

    // selectors...
    auto term = step >> *(-lit('>') >> step);
    auto product = term >> *('+' >> term);
    auto sum = product >> *(',' >> product);
    step = ('*' | ident) >> -stepsuffix
         | stepsuffix
         | '(' >> qi::skip(skipper.alias())[sum] >> ')';
    stepsuffix = '.' >> ident >> -stepsuffix
               | '#' >> ident >> -stepsuffix
               | '[' >> qi::skip(skipper.alias())[ident >> '=' >> val >> ']' ]
                                                  >> -stepsuffix;
    selector = -char_("+>,") >> sum;

    // rules, rulesets...
    auto import = lit("@import") >> string;
    auto context = lit("@context") >> -char_(">+") >> '(' >> selector >> ')'
        >> -lit(';');
    rule = (import | property | selector >> '{' >> *rule >> '}') >> -lit(';');
    ruleset = *context >> *rule;
  }

  bool parse(I &iter, I end) {
    return qi::phrase_parse(iter, end, *this, skipper);
  }
};

}


bool Parser::parseString(std::string input) {
  auto iter = input.begin();
  ccs_grammar<typeof(iter)> grammar;
  bool r = grammar.parse(iter, input.end());

  if (r && iter == input.end()) {
    std::cout << "Parsing succeeded\n";
    //std::cout << "result = " << result << std::endl;
    return true;
  } else {
    std::string rest(iter, input.end());
    std::cout << "Parsing failed, stopped at: \"" << rest << "\"\n";
    return false;
  }
}


bool Parser::parseString(Node &root, std::string input) {
  auto iter = input.begin();
  ccs_grammar<typeof(iter)> grammar;
  bool r = grammar.parse(iter, input.end());

  if (r && iter == input.end()) {
    std::cout << "Parsing succeeded\n";
    //std::cout << "result = " << result << std::endl;
    return true;
  } else {
    std::string rest(iter, input.end());
    std::cout << "Parsing failed, stopped at: \"" << rest << "\"\n";
    return false;
  }
}

}
