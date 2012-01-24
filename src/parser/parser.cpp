#include "parser/parser.h"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <boost/spirit/include/qi.hpp>

#include "Node.h"
#include "parser/ast.h"

using namespace std;
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace ccs {

namespace {

template <typename Iterator>
struct ccs_grammar : qi::grammar<Iterator, ast::Nested(), qi::rule<Iterator>> {
  typedef Iterator I;
  typedef qi::rule<I> nospacerule;

  nospacerule blockComment;
  nospacerule skipper;

  typedef qi::rule<I, typeof(skipper)> spacerule;

  qi::rule<I, string()> ident;
  nospacerule modifiers;
  spacerule property;
  spacerule selector;
  nospacerule vals;
  nospacerule singlestep;
  nospacerule stepsuffix;
  spacerule step;
  qi::rule<I, typeof(skipper)> rulebody;
  qi::rule<I> rule;
  qi::rule<I, ast::Nested(), typeof(skipper)> ruleset;

  ccs_grammar() : ccs_grammar::base_type(ruleset, "CCS"),
    blockComment("block comment"),
    skipper("whitespace and comments"),
    ident("identifier"),
    modifiers("property modifiers"),
    property("property"),
    selector("selector"),
    vals("selector values"),
    singlestep("single selector step"),
    stepsuffix("single selector step modifier"),
    step("selector step"),
    rulebody(string("CCS rule body")),
    rule(string("CCS rule")),
    ruleset(string("CCS ruleset"))
  {
    using qi::lit;
    using qi::char_;
    using qi::space;
    using qi::eol;
    using qi::eoi;
    using boost::spirit::_val;
    using boost::spirit::_1;

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
    modifiers = -((lit("@override") >> skipper) ^
        (lit("@local") >> skipper));
    property = modifiers >> ident >> '=' >> val;

    // selectors...
    vals = lit('.') >> ident >> -vals;
    auto namevals = ident >> -vals;
    stepsuffix = lit('/') >> singlestep;
    singlestep = namevals >> -stepsuffix;

    auto term = step >> *(lit('>') >> step);
    auto product = term >> *term;
    auto sum = product >> *(',' >> product);
    step = singlestep
        | '(' >> sum >> ')';
    selector = sum >> -lit('>');

    // rules, rulesets...
    auto import = lit("@import") >> string;
    auto constraint = lit("@constraint") >> singlestep;
    auto nested = selector >>
        (':' >> (import | constraint | property)
            | '{' >> *rule >> '}');
    rulebody = import | constraint | property | nested;
    rule = qi::skip(skipper.alias())[rulebody] >>
        (lit(';') | skipper | &lit('}') | eoi);
    auto context = lit("@context") >> '(' >> selector >> ')' >> -lit(';');
    ruleset = -context
        >> *rule
        >> eoi;
  }

  bool parse(I &iter, I end) {
    ast::Nested ast;
    return qi::phrase_parse(iter, end, *this, skipper, ast);
  }
};

}


bool Parser::parseString(string input) {
  auto iter = input.begin();
  ccs_grammar<typeof(iter)> grammar;
  bool r = grammar.parse(iter, input.end());

  if (r && iter == input.end()) {
    cout << "Parsing succeeded\n";
    //cout << "result = " << result << endl;
    return true;
  } else {
    string rest(iter, input.end());
    cout << "Parsing failed, stopped at: \"" << rest << "\"\n";
    return false;
  }
}


bool Parser::parseString(Node &root, string input) {
  auto iter = input.begin();
  ccs_grammar<typeof(iter)> grammar;
  bool r = grammar.parse(iter, input.end());

  if (r && iter == input.end()) {
    cout << "Parsing succeeded\n";
    //cout << "result = " << result << endl;
    return true;
  } else {
    string rest(iter, input.end());
    cout << "Parsing failed, stopped at: \"" << rest << "\"\n";
    return false;
  }
}

}
