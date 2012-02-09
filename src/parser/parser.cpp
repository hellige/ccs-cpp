#include "parser/parser.h"

#include <iostream>
#include <unordered_map>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/tuple/tuple.hpp>

#include "Node.h"
#include "parser/ast.h"

using namespace std;
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

BOOST_FUSION_ADAPT_STRUCT(
    ccs::ast::Import,
    (string, location_)
)

namespace ccs {

namespace {


template <typename Iterator>
struct ccs_grammar : qi::grammar<Iterator, ast::Nested(), qi::rule<Iterator>> {
  typedef Iterator I;

  qi::rule<I> blockComment;
  qi::rule<I> skipper;

  typedef qi::rule<I, typeof(skipper)> spacerule;

  qi::rule<I, string()> ident;
  qi::rule<I, string()> strng;
  qi::rule<I, void(ast::PropDef &)> modifiers;
  qi::rule<I, ast::Value()> val;
  qi::rule<I, ast::PropDef(), typeof(skipper)> property;
  spacerule selector;
  qi::rule<I, void(Key &, const string &)> vals;
  qi::rule<I, void(Key &), qi::locals<string>> singlestep;
  qi::rule<I, void(Key &)> stepsuffix;
  qi::rule<I, Key(), typeof(skipper)> step;

  qi::rule<I, ast::Import(), typeof(skipper)> import;
  qi::rule<I, ast::Constraint(), typeof(skipper)> constraint;
  qi::rule<I, typeof(skipper)> rulebody;
  qi::rule<I> rule;
  qi::rule<I, ast::Nested(), typeof(skipper)> ruleset;

  ccs_grammar() : ccs_grammar::base_type(ruleset, "CCS") {
    using qi::lit;
    using qi::char_;
    using qi::space;
    using qi::eol;
    using qi::eoi;
    using qi::_r1;
    using qi::_r2;
    using qi::_a;
    using boost::spirit::_val;
    using boost::spirit::_1;
    using boost::spirit::_2;
    using boost::spirit::_3;

    // comments and whitespace...
    blockComment = "/*" >> *(blockComment | (char_ - "*/")) >> "*/";
    skipper = blockComment
            | "//" >> *(char_ - eol) >> (eol | eoi)
            | space;

    strng = qi::lexeme['\'' >> *(char_ - ('\'' | eol)) >> '\'']
           | qi::lexeme['"' >> *(char_ - ('"' | eol)) >> '"'];
    val = lit("0x") >> qi::hex [_val = _1]
        | qi::long_long [_val = _1] >> !lit('.')
        | qi::double_ [_val = _1]
        | strng;
    ident %= +char_("A-Za-z0-9$_") | strng;

    // properties...
    modifiers =
        -((lit("@override")
            [phoenix::bind(&ast::PropDef::override_, _r1) = true] >> skipper) ^
        (lit("@local")
            [phoenix::bind(&ast::PropDef::local_, _r1) = true ] >> skipper));
    property = modifiers(_val)
        >> ident [phoenix::bind(&ast::PropDef::name_, _val) = _1]
        >> '=' >> val [phoenix::bind(&ast::PropDef::value_, _val) = _1];

    // selectors...
    vals = lit('.') >> ident[phoenix::bind(&Key::addValue, _r1, _r2, _1)]
                             >> -vals(_r1, _r2);
    stepsuffix = lit('/') >> singlestep(_r1);
    singlestep = ident [phoenix::bind(&Key::addName, _r1, _1), _a = _1]
                        >> -vals(_r1, _a) >> -stepsuffix(_r1);

    auto term = step >> *(lit('>') >> step);
    auto product = term >> *term;
    auto sum = product >> *(',' >> product);
    step = singlestep(_val)
        | '(' >> sum >> ')';
    selector = sum >> -lit('>');

    // rules, rulesets...
    import %= lit("@import") >> strng;
    constraint = lit("@constraint")
        >> singlestep(phoenix::bind(&ast::Constraint::key_, _val));
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
