#include "Parser.h"

#include <iostream>
#include <boost/spirit/include/qi.hpp>

#include "Node.h"

namespace qi = boost::spirit::qi;

namespace ccs {

namespace {

template <typename Iterator>
struct skip_grammar : qi::grammar<Iterator> {
  qi::rule<Iterator> blockComment;
  qi::rule<Iterator> start;

  skip_grammar() : skip_grammar::base_type(start) {
    using qi::char_;
    using qi::space;
    using qi::eol;
    using qi::eoi;

    blockComment = "/*" >> *(blockComment | (char_ - "*/")) >> "*/";
    start = blockComment
          | "//" >> *(char_ - eol) >> (eol | eoi)
          | space;
  }
};

template <typename Iterator>
struct ccs_grammar : qi::grammar<Iterator, skip_grammar<Iterator>> {
  typedef qi::rule<Iterator, skip_grammar<Iterator> > spacerule;
  typedef qi::rule<Iterator> nospacerule;

  nospacerule ident;
  spacerule directive;
  spacerule property;
  spacerule selector;
  nospacerule step;
  nospacerule stepsuffix;
  spacerule rule;
  spacerule file;

  ccs_grammar() : ccs_grammar::base_type(file) {
    using qi::eol;
    using qi::lit;
    using qi::char_;

    auto string = qi::lexeme['\'' >> *(char_ - ('\'' | eol)) >> '\''];
    auto val = string | qi::long_long | qi::double_ | lit("0x") >> qi::hex;
    ident = +char_("A-Za-z0-9$_") | string;
    directive = lit("@import") >> string >> -lit(';');
    property = ident >> '=' >> val >> -lit(';');
    selector = step >> '>' >> selector
             | step >> selector
             | step;
    step = ident >> -stepsuffix
         | stepsuffix;
    auto pseudo = lit("root");
    stepsuffix = '.' >> ident >> -stepsuffix
               | ':' >> pseudo >> -stepsuffix
               | '#' >> ident >> -stepsuffix;
    rule = property | (selector >> '{' >> +property >> '}' >> -lit(';'));
    file = *(rule | directive);
  }
};

}


bool Parser::parseString(std::string input) {
  auto iter = input.begin();
  skip_grammar<typeof(iter)> skip;
  ccs_grammar<typeof(iter)> grammar;
  bool r = qi::phrase_parse(iter, input.end(), grammar, skip);

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
  skip_grammar<typeof(iter)> skip;
  ccs_grammar<typeof(iter)> grammar;
  bool r = qi::phrase_parse(iter, input.end(), grammar, skip);

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
