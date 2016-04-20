#include "parser/parser.h"

#include <iostream>
#include <istream>
#include <regex>
#include <vector>

#define THROW(where, stuff) \
  do { \
    std::ostringstream _message; \
    _message << stuff; \
    throw parse_error(_message.str(), where); \
  } while (0)

namespace ccs {

namespace {

struct Location {
  uint32_t line;
  uint32_t column;

  Location(uint32_t line, uint32_t column)
  : line(line), column(column) {}
};

struct parse_error {
  std::string what;
  Location where;

  parse_error(const std::string &what, Location where)
  : what(what), where(where) {}
};

// TODO collapse IDENT and STRING?

// TODO this should be cleaned up, all these stupid "values" is dumb and
// confusing...
struct Token {
  enum Type {
    EOS,
    LPAREN, RPAREN, LBRACE, RBRACE, SEMI, COLON, COMMA, DOT, GT, EQ, SLASH,
    CONSTRAIN, CONTEXT, IMPORT, OVERRIDE,
    INT, DOUBLE, IDENT, NUMID, STRING
  };

  Type type;
  Location location;
  std::string value;
  StringVal stringValue;
  int64_t intValue; // only valid in case of ints...
  double doubleValue; // only valid in case of numbers...

  Token() : type(EOS), location(0, 0), intValue(0), doubleValue(0) {}
  Token(Type type, Location loc)
  : type(type), location(loc), intValue(0), doubleValue(0) {}
  Token(Type type, char first, Location loc)
  : type(type), location(loc), intValue(0), doubleValue(0) {
    value += first;
  }

  void append(char c) { value += c; }
};

std::ostream &operator<<(std::ostream &os, Token::Type type) {
  switch (type) {
  case Token::EOS: os << "end-of-input"; break;
  case Token::LPAREN: os << "'('"; break;
  case Token::RPAREN: os << "')"; break;
  case Token::LBRACE: os << "'{'"; break;
  case Token::RBRACE: os << "'}'"; break;
  case Token::SEMI: os << "';'"; break;
  case Token::COLON: os << "':'"; break;
  case Token::COMMA: os << "','"; break;
  case Token::DOT: os << "'.'"; break;
  case Token::GT: os << "'>'"; break;
  case Token::EQ: os << "'='"; break;
  case Token::SLASH: os << "'/'"; break;
  case Token::CONSTRAIN: os << "'@constrain'"; break;
  case Token::CONTEXT: os << "'@context'"; break;
  case Token::IMPORT: os << "'@import'"; break;
  case Token::OVERRIDE: os << "'@override'"; break;
  case Token::INT: os << "integer"; break;
  case Token::DOUBLE: os << "double"; break;
  case Token::IDENT: os << "identifier"; break;
  case Token::NUMID: os << "numeric/identifier"; break;
  case Token::STRING: os << "string literal"; break;
  }
  return os;
}

class Buf {
  std::istream &stream_;
  uint32_t line_;
  uint32_t column_;

public:
  explicit Buf(std::istream &stream) : stream_(stream), line_(1), column_(0) {
    stream_.unsetf(std::ios::skipws);
  }

  Buf(const Buf &) = delete;
  const Buf &operator=(const Buf &) = delete;

  int get() {
    auto c = stream_.get();
    // this way of tracking location gives funny results when get() returns
    // a newline, but we don't actually care about that anyway...
    column_++;
    if (c == '\n') {
      line_++;
      column_ = 0;
    }
    return c;
  }

  int peek() {
    return stream_.peek();
  }

  Location location() { return Location(line_, column_); }
  Location peekLocation() { return Location(line_, column_+1); }
};

class Lexer {
  Buf stream_;
  Token next_;

public:
  Lexer(std::istream &stream) : stream_(stream), next_(nextToken()) {}

  const Token &peek() const { return next_; }

  Token consume() {
    Token tmp = next_;
    next_ = nextToken();
    return tmp;
  }

private:
  Token nextToken() {
    int c = stream_.get();

    while (std::isspace(c) || comment(c)) c = stream_.get();

    auto where = stream_.location();

    switch (c) {
    case EOF: return Token(Token::EOS, where);
    case '(': return Token(Token::LPAREN, where);
    case ')': return Token(Token::RPAREN, where);
    case '{': return Token(Token::LBRACE, where);
    case '}': return Token(Token::RBRACE, where);
    case ';': return Token(Token::SEMI, where);
    case ':': return Token(Token::COLON, where);
    case ',': return Token(Token::COMMA, where);
    case '.': return Token(Token::DOT, where);
    case '>': return Token(Token::GT, where);
    case '=': return Token(Token::EQ, where);
    case '/': return Token(Token::SLASH, where);
    case '@': {
      Token tok = ident(c, where);
      if (tok.value == "@constrain") tok.type = Token::CONSTRAIN;
      else if (tok.value == "@context") tok.type = Token::CONTEXT;
      else if (tok.value == "@import") tok.type = Token::IMPORT;
      else if (tok.value == "@override") tok.type = Token::OVERRIDE;
      else THROW(where, "Unrecognized @-command: " << tok.value);
      return tok;
    }
    case '\'': return string(c, where);
    case '"': return string(c, where);
    }

    if (numIdInitChar(c)) return numId(c, where);
    if (identInitChar(c)) return ident(c, where);

    THROW(where, "Unexpected character: '" << (char)c << "' (0x" << std::hex
        << c << ")");
  }

  bool comment(int c) {
    if (c != '/') return false;
    if (stream_.peek() == '/') {
      stream_.get();
      for (int tmp = stream_.get(); tmp != '\n' && tmp != EOF;
          tmp = stream_.get());
      return true;
    } else if (stream_.peek() == '*') {
      stream_.get();
      multilineComment();
      return true;
    }
    return false;
  }

  void multilineComment() {
    while (true) {
      int c = stream_.get();
      if (c == EOF)
        THROW(stream_.location(), "Unterminated multi-line comment");
      if (c == '*' && stream_.peek() == '/') {
        stream_.get();
        return;
      }
      if (c == '/' && stream_.peek() == '*') {
        stream_.get();
        multilineComment();
      }
    }
  }

  bool identInitChar(int c) {
    if (c == '$') return true;
    if (c == '_') return true;
    if ('A' <= c && c <= 'Z') return true;
    if ('a' <= c && c <= 'z') return true;
    return false;
  }

  bool identChar(int c) {
    if (identInitChar(c)) return true;
    if ('0' <= c && c <= '9') return true;
    return false;
  }

  bool interpolantChar(int c) {
    if (c == '_') return true;
    if ('0' <= c && c <= '9') return true;
    if ('A' <= c && c <= 'Z') return true;
    if ('a' <= c && c <= 'z') return true;
    return false;
  }

  Token string(char first, Location where) {
    StringVal result;
    std::string current;
    while (stream_.peek() != first) {
      switch (stream_.peek()) {
      case EOF:
        THROW(stream_.peekLocation(), "Unterminated string literal");
      case '$': {
        stream_.get();
        if (stream_.peek() != '{')
          THROW(stream_.peekLocation(), "Expected '{'");
        stream_.get();
        if (!current.empty()) result.elements_.push_back(current);
        current = "";
        Interpolant interpolant;
        while (stream_.peek() != '}') {
          if (!interpolantChar(stream_.peek()))
              THROW(stream_.peekLocation(),
                  "Character not allowed in string interpolant: '" <<
                  (char)stream_.peek() << "' (0x" << std::hex << stream_.peek()
                  << ")");
          interpolant.name += stream_.get();
        }
        stream_.get();
        result.elements_.push_back(interpolant);
        break;
      }
      case '\\': {
        stream_.get();
        auto escape = stream_.get();
        switch (escape) {
          case EOF: THROW(stream_.location(), "Unterminated string literal");
          case '$': current += '$'; break;
          case '\'': current += '\''; break;
          case '"': current += '"'; break;
          case '\\': current += '\\'; break;
          case 't': current += '\t'; break;
          case 'n': current += '\n'; break;
          case 'r': current += '\r'; break;
          case '\n': break; // escaped newline: ignore
          default: THROW(stream_.location(), "Unrecognized escape sequence: '\\"
              << (char)escape << "' (0x" << std::hex << escape << ")");
        }
        break;
      }
      default:
        current += (char)stream_.get();
        break;
      }
    }
    stream_.get();
    if (!current.empty()) result.elements_.push_back(current);
    auto tok = Token(Token::STRING, where);
    tok.stringValue = result;
    return tok;
  }

  bool numIdInitChar(int c) {
    if ('0' <= c && c <= '9') return true;
    if (c == '-' || c == '+') return true;
    return false;
  }

  bool numIdChar(int c) {
    if (numIdInitChar(c)) return true;
    if (identChar(c)) return true;
    if (c == '.') return true;
    return false;
  }

  Token numId(char first, Location where) {
    static std::regex intRe("(\\+|-)?[0-9]+");
    static std::regex doubleRe("[-+]?[0-9]+\\.?[0-9]*([eE][-+]?[0-9]+)?");

    if (first == '0' and stream_.peek() == 'x') {
      stream_.get();
      return hexLiteral(where);
    }

    Token token(Token::NUMID, first, where);
    while (numIdChar(stream_.peek())) token.append(stream_.get());

    if (std::regex_match(token.value, intRe)) {
      try {
        token.type = Token::INT;
        token.intValue = boost::lexical_cast<int64_t>(token.value);
        return token;
      } catch (const std::bad_cast &e) {
        THROW(where, "Internal error: "
            << "integer regex matched, but lexical cast failed! '"
            << token.value << "'");
      }
    } else if (std::regex_match(token.value, doubleRe)) {
      try {
        token.type = Token::DOUBLE;
        token.doubleValue = boost::lexical_cast<double>(token.value);
        return token;
      } catch (const std::bad_cast &e) {
        THROW(where, "Internal error: "
            << "double regex matched, but lexical cast failed! '"
            << token.value << "'");
      }
    }

    return token; // it's a generic NUMID
  }

  int hexChar(int c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return 10 + c - 'a';
    if ('A' <= c && c <= 'F') return 10 + c - 'A';
    return -1;
  }

  Token hexLiteral(Location where) {
    Token token(Token::INT, where);
    for (int n = hexChar(stream_.peek()); n != -1;
        n = hexChar(stream_.peek())) {
      token.intValue = token.intValue * 16 + n;
      token.append(stream_.get());
    }
    token.doubleValue = token.intValue;
    return token;
  }

  Token ident(char first, Location where) {
    Token token(Token::IDENT, first, where);
    while (identChar(stream_.peek())) token.append(stream_.get());
    return token;
  }
};

}

class ParserImpl {
  std::string fileName_;
  Lexer lex_;
  Token cur_;
  Token last_;

public:
  ParserImpl(const std::string &fileName, std::istream &stream)
  : fileName_(fileName), lex_(stream) {}

  bool parseRuleset(ast::Nested &ast) {
    advance();
    if (advanceIf(Token::CONTEXT)) ast.selector_ = parseContext();
    while (cur_.type != Token::EOS) parseRule(ast);
    return true;
  }

private:
  void advance() { last_ = cur_; cur_ = lex_.consume(); }

  bool advanceIf(Token::Type type) {
    if (cur_.type == type) {
      advance();
      return true;
    }
    return false;
  }

  void expect(Token::Type type) {
    if (!advanceIf(type))
      THROW(cur_.location, "Expected " << type << ", found " << cur_.type);
  }

  ast::SelectorBranch::P parseContext() {
    expect(Token::LPAREN);
    auto result = parseSelector();
    expect(Token::RPAREN);
    advanceIf(Token::SEMI);
    return result;
  }

  void parseRule(ast::Nested &ast) {
    // the only ambiguity is between ident as start of a property setting
    // and ident as start of a selector, i.e.:
    //   foo = bar
    //   foo : bar = 'baz'
    // we can use the presence of '=' to disambiguate. parsePrimRule() performs
    // this lookahead without consuming the additional token.
    if (parsePrimRule(ast)) {
      advanceIf(Token::SEMI);
      return;
    }

    ast::Nested nested;
    nested.selector_ = parseSelector();

    if (advanceIf(Token::COLON)) {
      if (!parsePrimRule(nested))
        THROW(cur_.location,
            "Expected @import, @constrain, or property setting");
      advanceIf(Token::SEMI);
    } else if (advanceIf(Token::LBRACE)) {
      while (!advanceIf(Token::RBRACE)) parseRule(nested);
    } else {
      THROW(cur_.location, "Expected ':' or '{' following selector");
    }

    ast.addRule(nested);
  }

  bool parsePrimRule(ast::Nested &ast) {
    switch (cur_.type) {
    case Token::IMPORT:
      advance();
      expect(Token::STRING);
      if (last_.stringValue.interpolation())
        THROW(last_.location, "Interpolation not allowed in import statements");
      ast.addRule(ast::Import(last_.stringValue.str()));
      return true;
    case Token::CONSTRAIN:
      advance();
      ast.addRule(ast::Constraint(parseSingleStep()));
      return true;
    case Token::OVERRIDE:
      advance();
      parseProperty(ast, true);
      return true;
    case Token::IDENT:
    case Token::STRING:
      if (lex_.peek().type == Token::EQ) {
        parseProperty(ast, false);
        return true;
      }
      break;
    default: break;
    }
    return false;
  }

  void parseProperty(ast::Nested &ast, bool override) {
    ast::PropDef prop;
    prop.name_ = parseIdent("property name");
    prop.override_ = override;
    expect(Token::EQ);

    // we set the origin from the location of the equals sign. it's a bit
    // arbitrary, but it seems as good as anything.
    prop.origin_ = Origin(fileName_, last_.location.line);

    switch (cur_.type) {
    case Token::INT:
      prop.value_.setInt(cur_.intValue); break;
    case Token::DOUBLE:
      prop.value_.setDouble(cur_.doubleValue); break;
    case Token::STRING:
      prop.value_.setString(cur_.stringValue); break;
    case Token::NUMID:
      prop.value_.setString(StringVal(cur_.value)); break;
    case Token::IDENT:
      if (cur_.value == "true") prop.value_.setBool(true);
      else if (cur_.value == "false") prop.value_.setBool(false);
      else prop.value_.setString(StringVal(cur_.value));
      break;
    default:
      THROW(cur_.location, cur_.type
          << " cannot occur here. Expected property value "
          << "(number, identifier, string, or boolean)");
    }
    ast.addRule(prop);
    advance();
    return;
  }

  ast::SelectorBranch::P parseSelector() {
    auto leaf = parseSum();
    if (advanceIf(Token::GT)) {
      return ast::SelectorBranch::descendant(leaf);
    } else {
      return ast::SelectorBranch::conjunction(leaf);
    }
  }

  ast::SelectorLeaf::P parseSum() {
    auto left = parseProduct();
    while (advanceIf(Token::COMMA))
      left = ast::SelectorLeaf::disj(left, parseProduct());
    return left;
  }

  bool couldStartStep(const Token &token) {
    switch(token.type) {
    case Token::IDENT:
    case Token::STRING:
    case Token::LPAREN:
      return true;
    default:
      return false;
    }
  }

  ast::SelectorLeaf::P parseProduct() {
    auto left = parseTerm();
    // term starts with ident or '(', which is enough to disambiguate...
    while (couldStartStep(cur_))
      left = ast::SelectorLeaf::conj(left, parseTerm());
    return left;
  }

  ast::SelectorLeaf::P parseTerm() {
    auto left = parseStep();
    while (cur_.type == Token::GT) {
      // here we have to distinguish another step from a trailing '>'. again,
      // peeking for ident or '(' does the trick.
      if (!couldStartStep(lex_.peek())) return left;
      advance();
      left = ast::SelectorLeaf::desc(left, parseStep());
    }
    return left;
  }

  ast::SelectorLeaf::P parseStep() {
    if (advanceIf(Token::LPAREN)) {
      auto result = parseSum();
      expect(Token::RPAREN);
      return result;
    } else {
      return ast::SelectorLeaf::step(parseSingleStep());
    }
  }

  Key parseSingleStep() {
    Key key;
    do {
      auto name = parseIdent("selector name");
      key.addName(name);
      while (advanceIf(Token::DOT))
        key.addValue(name, parseIdent("selector value"));
    } while (advanceIf(Token::SLASH));
    return key;
  }

  std::string parseIdent(const char *what) {
    if (advanceIf(Token::IDENT)) return last_.value;
    if (advanceIf(Token::STRING)) {
      if (last_.stringValue.interpolation())
        THROW(last_.location, "Interpolation not allowed in " << what);
      return last_.stringValue.str();
    }
    THROW(cur_.location, cur_.type << " cannot occur here. Expected " << what);
  }
};

Parser::Parser(CcsLogger &log) : log(log) {}
Parser::~Parser() {}

bool Parser::parseCcsStream(const std::string &fileName, std::istream &stream,
    ast::Nested &ast) {
  try {
    ParserImpl p(fileName, stream);
    if (p.parseRuleset(ast))
      return true;
    std::ostringstream msg;
    msg << "Unknown error parsing " << fileName + "!";
    log.error(msg.str());
  } catch (const parse_error &e) {
    std::ostringstream msg;
    msg << "Parse error at file " << fileName << ':' << e.where.line << ':'
        << e.where.column << ": " << e.what;
    log.error(msg.str());
  }
  return false;
}

}
