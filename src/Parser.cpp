#include "Parser.h"

#include <iostream>
#include <boost/spirit/include/qi.hpp>

namespace qi = boost::spirit::qi;

namespace ccs {

namespace {

template <typename Iterator>
struct ccs_grammar : qi::grammar<Iterator>
{
  ccs_grammar() : ccs_grammar::base_type(file) {
    using qi::lit;

    directive = lit("@include") >> '(' >> ')';
    property = lit("prop") >> '=' >> lit("val");
    selector = step >> '>' >> selector
             | step >> selector
             | step;
    step = lit("elem") >> stepsuffix
         | stepsuffix;
    stepsuffix = '.' >> lit("class") >> -stepsuffix
               | ':' >> lit("pseudo") >> -stepsuffix
               | '#' >> lit("id") >> -stepsuffix;
    rule = property | (selector >> '{' >> +rule >> '}');
    file = *(rule | directive);
  }

  qi::rule<Iterator> directive;
  qi::rule<Iterator> property;
  qi::rule<Iterator> selector;
  qi::rule<Iterator> step;
  qi::rule<Iterator> stepsuffix;
  qi::rule<Iterator> rule;
  qi::rule<Iterator> file;
};

}


bool Parser::parseString(std::string input) {
  auto iter = input.begin();
  bool r = qi::parse(iter, input.end(), ccs_grammar<std::string::iterator>());

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
