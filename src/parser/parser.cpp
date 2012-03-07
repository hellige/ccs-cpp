#include "parser/parser.h"

#include <iostream>
#include <istream>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/tuple/tuple.hpp>

using std::cout;
using std::string;
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

BOOST_FUSION_ADAPT_STRUCT(
    ccs::ast::Import,
    (string, location_)
)

BOOST_FUSION_ADAPT_STRUCT(
    ccs::ast::Nested,
    (ccs::ast::SelectorBranch *, selector_)
    (std::vector<ccs::ast::AstRule>, rules_)
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
  qi::rule<I, ast::SelectorBranch*(), typeof(skipper)> selector;
  qi::rule<I, void(Key &, const string &)> vals;
  qi::rule<I, void(Key &), qi::locals<string>> singlestep;
  qi::rule<I, void(Key &)> stepsuffix;
  qi::rule<I, ast::SelectorLeaf*(), typeof(skipper), qi::locals<Key>> step;
  qi::rule<I, ast::SelectorLeaf*(), typeof(skipper)> term;
  qi::rule<I, ast::SelectorLeaf*(), typeof(skipper)> product;
  qi::rule<I, ast::SelectorLeaf*(), typeof(skipper)> sum;

  qi::rule<I, ast::Import(), typeof(skipper)> import;
  qi::rule<I, ast::Constraint(), typeof(skipper)> constraint;
  qi::rule<I, ast::Nested(), typeof(skipper)> nested;
  qi::rule<I, ast::AstRule(), typeof(skipper)> rulebody;
  qi::rule<I, ast::AstRule()> rule;
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
    using phoenix::bind;

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
            [bind(&ast::PropDef::override_, _r1) = true] >> skipper) ^
        (lit("@local")
            [bind(&ast::PropDef::local_, _r1) = true ] >> skipper));
    property = modifiers(_val)
        >> ident [bind(&ast::PropDef::name_, _val) = _1]
        >> '=' >> val [bind(&ast::PropDef::value_, _val) = _1];

    // selectors...
    vals = lit('.') >> ident[bind(&Key::addValue, _r1, _r2, _1)]
                             >> -vals(_r1, _r2);
    stepsuffix = lit('/') >> singlestep(_r1);
    singlestep = ident [bind(&Key::addName, _r1, _1), _a = _1]
                        >> -vals(_r1, _a) >> -stepsuffix(_r1);

    term = step [_val = _1] >> *(lit('>') >> step
        [_val = bind(&ast::SelectorLeaf::descendant, _val, _1)]);
    product = term [_val = _1] >> *(term
        [_val = bind(&ast::SelectorLeaf::conjunction, _val, _1)]);
    sum = product [_val = _1] >> *(',' >> product
        [_val = bind(&ast::SelectorLeaf::disjunction, _val, _1)]);
    step = singlestep(_a) [_val = bind(&ast::SelectorLeaf::step, _a)]
        | '(' >> sum [_val = _1] >> ')';
    selector = (sum >> -lit('>'))
        [bind(&ast::SelectorBranch::conjunction, _1)];

    // rules, rulesets...
    import %= lit("@import") >> strng;
    constraint = lit("@constraint")
        >> singlestep(bind(&ast::Constraint::key_, _val));
    nested = selector [bind(&ast::Nested::selector_, _val) = _1] >>
        (':' >> (import | constraint | property)
            [bind(&ast::Nested::addRule, _val, _1)]
        | ('{' >> *rule >> '}')
            [bind(&ast::Nested::rules_, _val) = _1]);
    rulebody = import | constraint | property | nested;
    rule = qi::skip(skipper.alias())[rulebody] >>
        (lit(';') | skipper | &lit('}') | eoi);
    auto context = lit("@context") >> '(' >> selector >> ')' >> -lit(';');
    ruleset %= -context >> *rule >> eoi;
  }

  bool parse(I &iter, I end, ast::Nested &ast) {
    return qi::phrase_parse(iter, end, *this, skipper, ast);
  }
};

}


bool Parser::parseCcsStream(std::istream &stream, ast::Nested &ast) {
  stream.unsetf(std::ios::skipws);

  boost::spirit::istream_iterator iter(stream);
  boost::spirit::istream_iterator end;

  ccs_grammar<typeof(iter)> grammar;
  bool r = grammar.parse(iter, end, ast);

  if (r && iter == end) return true;

  string rest(iter, end);
  cout << "Parsing failed, stopped at: \"" << rest << "\"\n";
  return false;
}


}
