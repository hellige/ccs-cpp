#include "parser/parser.h"

#include <iostream>
#include <istream>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/tuple/tuple.hpp>

using std::cout;
using std::string;
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace classic = boost::spirit::classic;

BOOST_FUSION_ADAPT_STRUCT(
    ccs::Interpolant,
    (string, name)
)

BOOST_FUSION_ADAPT_STRUCT(
    ccs::StringVal,
    (std::vector<ccs::StringElem>, elements_)
)

BOOST_FUSION_ADAPT_STRUCT(
    ccs::ast::Import,
    (string, location)
)

BOOST_FUSION_ADAPT_STRUCT(
    ccs::ast::Nested,
    (ccs::ast::SelectorBranch::P, selector_)
    (std::vector<ccs::ast::AstRule>, rules_)
)

namespace ccs {

namespace {


// MMH it really might be better at this point to ditch spirit's 'skipper'
// stuff entirely and go with explicit optional/required whitespace, as in
// the java version.
template <typename Iterator>
struct ccs_grammar : qi::grammar<Iterator, ast::Nested(), qi::rule<Iterator>> {
  typedef Iterator I;

  qi::rule<I> blockComment;
  qi::rule<I> skipper;

  qi::rule<I, string()> ident;
  qi::rule<I, string()> strng;
  qi::rule<I, Interpolant()> interpolant;
  qi::rule<I, StringVal()> strngval;
  qi::rule<I, char()> escape;
  qi::rule<I, string(char)> stringelem;
  qi::rule<I, void(ast::PropDef &)> modifiers;
  qi::rule<I, Value()> val;
  qi::rule<I, ast::PropDef(), typeof(skipper)> property;
  qi::rule<I, ast::SelectorBranch::P(), typeof(skipper)> selector;
  qi::rule<I, void(Key &, const string &)> vals;
  qi::rule<I, void(Key &), qi::locals<string>> singlestep;
  qi::rule<I, void(Key &)> stepsuffix;
  qi::rule<I, ast::SelectorLeaf::P(), typeof(skipper), qi::locals<Key>> step;
  qi::rule<I, ast::SelectorLeaf::P(), typeof(skipper)> term;
  qi::rule<I, ast::SelectorLeaf::P(), typeof(skipper)> product;
  qi::rule<I, ast::SelectorLeaf::P(), typeof(skipper)> sum;

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

    interpolant = "${" > +char_("A-Za-z0-9_") > '}';
    escape = lit('\\') >
      (lit('$') [_val = '$'] |
       lit('\'') [_val = '\''] |
       lit('"') [_val = '"'] |
       lit('\\') [_val = '\\'] |
       lit('t') [_val = '\t'] |
       lit('n') [_val = '\n'] |
       lit('r') [_val = '\r']);
    stringelem = +(lit("\\\n") | escape | (char_ - (lit(_r1) | '$' | eol)));
    strng = qi::lexeme['\'' >> -stringelem('\'') >> '\'']
           | qi::lexeme['"' >> -stringelem('"') >> '"'];
    strngval =
          qi::lexeme['\'' >> *(interpolant | stringelem('\'')) >> '\'']
        | qi::lexeme['"' >> *(interpolant | stringelem('"')) >> '"'];

    val = lit("0x") >> qi::hex [bind(&Value::setInt, _val, _1)]
        | qi::long_long [bind(&Value::setInt, _val, _1)]
                         >> !(lit('.') | lit('e') | lit('E'))
        | qi::double_ [bind(&Value::setDouble, _val, _1)]
        | qi::bool_ [bind(&Value::setBool, _val, _1)]
        | strngval [bind(&Value::setString, _val, _1)];
    ident %= +char_("A-Za-z0-9$_") | strng;

    // properties...
    modifiers =
        -(lit("@override")
            [bind(&ast::PropDef::override_, _r1) = true] >> +skipper);
    property = modifiers(_val)
        >> ident [bind(&ast::PropDef::name_, _val) = _1]
        >> '=' > val [bind(&ast::PropDef::value_, _val) = _1];
    qi::on_success(property, bind(setOrigin, _val, _1));

    // selectors...
    vals = lit('.') > ident[bind(&Key::addValue, _r1, _r2, _1)]
                             >> -vals(_r1, _r2);
    stepsuffix = lit('/') > singlestep(_r1);
    singlestep = ident [bind(&Key::addName, _r1, _1), _a = _1]
                        >> -vals(_r1, _a) >> -stepsuffix(_r1);

    term = step [_val = _1] >> *(lit('>') >> step
        [_val = bind(&ast::SelectorLeaf::desc, _val, _1)]);
    product = term [_val = _1] >> *(term
        [_val = bind(&ast::SelectorLeaf::conj, _val, _1)]);
    sum = product [_val = _1] >> *(',' >> product
        [_val = bind(&ast::SelectorLeaf::disj, _val, _1)]);
    step = singlestep(_a) [_val = bind(&ast::SelectorLeaf::step, _a)]
        | ('(' > sum [_val = _1] >> ')');
    selector = (sum >> -qi::string(">")) [_val = bind(branch, _1, _2)];

    // rules, rulesets...
    import %= qi::lexeme[lit("@import") > +skipper > strng];
    constraint %= qi::lexeme[lit("@constrain") > +skipper
        > singlestep(bind(&ast::Constraint::key_, _val))];
    nested = selector [bind(&ast::Nested::selector_, _val) = _1] >>
        ((':' > (import | constraint | property)
            [bind(&ast::Nested::addRule, _val, _1)])
        | ('{' > *rule > '}')
            [bind(&ast::Nested::rules_, _val) = _1]);
    rulebody = import | constraint | property | nested;
    rule = qi::skip(skipper.alias())[rulebody] >>
        ((+skipper || lit(';')) | &lit('}') | eoi);
    // cf http://stackoverflow.com/questions/20763665/boost-spirit-v2-qi-bug-associated-with-optimization-level/20766909#20766909
    const auto context = boost::proto::deep_copy(
        lit("@context") > '(' > selector > ')' > -lit(';'));
    ruleset %= -context > *rule > eoi;
  }

  static void setOrigin(ast::PropDef &propDef, Iterator it) {
    const classic::file_position_base<std::string> &pos =
        it.get_position();
    propDef.origin_ = Origin(pos.file, pos.line);
  }

  static std::shared_ptr<ast::SelectorBranch> branch(ast::SelectorLeaf::P leaf,
      boost::optional<string> opt) {
    return opt ? ast::SelectorBranch::descendant(leaf)
               : ast::SelectorBranch::conjunction(leaf);
  }

  bool parse(I &iter, I end, ast::Nested &ast) {
    return qi::phrase_parse(iter, end, *this, skipper, ast);
  }
};

}

struct Parser::Impl {
  typedef boost::spirit::istream_iterator fwd_iterator_type;
  typedef classic::position_iterator2<fwd_iterator_type> pos_iterator_type;
  ccs_grammar<pos_iterator_type> grammar;
};

Parser::Parser(CcsLogger &log) : log(log), impl(new Impl()) {}
Parser::~Parser() {}

bool Parser::parseCcsStreamDEAD(const std::string &fileName, std::istream &stream,
    ast::Nested &ast) {
  stream.unsetf(std::ios::skipws);

  Impl::fwd_iterator_type fwd_begin(stream);
  Impl::fwd_iterator_type fwd_end;

  // wrap forward iterator with position iterator, to record the position
  Impl::pos_iterator_type iter(fwd_begin, fwd_end, fileName);
  Impl::pos_iterator_type end;

  try {
    if (impl->grammar.parse(iter, end, ast))
      return true;
    std::ostringstream msg;
    msg << "Unknown error parsing " << fileName + "!";
    log.error(msg.str());
  } catch (const qi::expectation_failure<Impl::pos_iterator_type>& e) {
    const classic::file_position_base<std::string> &pos =
        e.first.get_position();
    std::ostringstream msg;
    msg << "Parse error at file " << pos.file << ':' << pos.line << ':'
      << pos.column << ": '" << e.first.get_currentline() << "'";
    log.error(msg.str());
  }

  return false;
}


}
