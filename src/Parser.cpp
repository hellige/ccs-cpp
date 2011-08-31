#include "Parser.h"

#include <iostream>
#include <boost/spirit/include/qi.hpp>

#include "Node.h"

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace ccs {

namespace {

template <typename Iterator>
struct ccs_grammar : qi::grammar<Iterator, qi::rule<Iterator>> {
  typedef qi::rule<Iterator, qi::rule<Iterator>> spacerule;
  typedef qi::rule<Iterator> nospacerule;

  nospacerule blockComment;
  nospacerule skipper;

  nospacerule ident;
  spacerule property;
  spacerule selector;
  nospacerule step;
  nospacerule stepsuffix;
  spacerule rule;
  spacerule file;

  ccs_grammar() : ccs_grammar::base_type(file) {
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

    auto string = qi::lexeme['\'' >> *(char_ - ('\'' | eol)) >> '\''];
    auto val = string | qi::long_long | qi::double_ | lit("0x") >> qi::hex;
    ident = +char_("A-Za-z0-9$_") | string;
    auto context = lit("@context") >> -char_(">+") >> '(' >> selector >> ')'
        >> -lit(';');
    auto import = lit("@import") >> string >> -lit(';');
    property = ident >> '=' >> val >> -lit(';');
    // TODO make '+' and ',' lower precedence than '>' and ' '...
    // TODO allow operator at start of selector...
    selector = step >> '>' >> selector
             | step >> '+' >> selector
             | step >> selector
             | step;
    step = ('*' | ident) >> -stepsuffix
         | stepsuffix
         | '(' >> qi::skip(skipper.alias())[selector] >> ')';
    auto pseudo = lit("root") >> !ident;
    stepsuffix = '.' >> ident >> -stepsuffix
               | ':' >> pseudo >> -stepsuffix
               | '#' >> ident >> -stepsuffix
               | '[' >> qi::skip(skipper.alias())[ident >> '=' >> val >> ']' ]
                                                  >> -stepsuffix;
    rule = import | property | (selector >> '{' >> +rule >> '}' >> -lit(';'));
    file = *context >> *rule;
  }

  bool parse(Iterator &iter, Iterator end) {
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
