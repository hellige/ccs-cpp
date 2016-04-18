#include "parser/parser2.h"

#include <iostream>
#include <istream>
#include <vector>

namespace ccs {

namespace {

// TODO collapse IDENT and STRING?

struct Token {
  enum Type {
    EOS,
    LPAREN, RPAREN, LBRACE, RBRACE, SEMI, COLON, COMMA, DOT, GT, EQ, SLASH,
    CONSTRAIN, CONTEXT, IMPORT, OVERRIDE,
    INT, DOUBLE, IDENT, STRING
  };

  Type type;
  std::string value;
  StringVal stringValue;
  int64_t intValue; // only valid in case of ints...
  double doubleValue; // only valid in case of numbers...

  Token() : type(EOS), intValue(0), doubleValue(0) {}
  Token(Type type) : type(type), intValue(0), doubleValue(0) {}
  Token(Type type, char first) : type(type), intValue(0), doubleValue(0) {
    value += first;
  }
  Token(Type type, const std::string value)
  : type(type), value(value), intValue(0), doubleValue(0) {}

  void append(char c) { value += c; }
};

class Buf {
  static constexpr size_t SIZE = 100; // TODO stupid size.
  std::istream &stream_;
  char buf_[SIZE];
  int pos_;
  int lim_;

public:
  explicit Buf(std::istream &stream) : stream_(stream), pos_(0), lim_(0) {
    stream_.unsetf(std::ios::skipws);
  }

  Buf(const Buf &) = delete;
  const Buf &operator=(const Buf &) = delete;

  int get() {
    if (!ensure(1)) return EOF;
    return buf_[pos_++];
  }

  int peek() {
    if (!ensure(1)) return EOF;
    return buf_[pos_];
  }

private:
  bool ensure(int n) {
    if (pos_ + n <= lim_) return true;

    auto delta = lim_ - pos_;
    memmove(buf_, buf_ + pos_, lim_ - pos_);
    pos_ = 0;
    lim_ = delta;
    stream_.read(buf_ + lim_, SIZE - lim_);
    auto read = stream_.gcount();
    lim_ += read;
    return pos_ + n <= lim_;
  }
};

// TODO position info

class Lexer {
  Buf stream_;
  Token next_;

public:
  Lexer(std::istream &stream) : stream_(stream) {
    next_ = nextToken();
  }

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

    switch (c) {
    case EOF: return Token::EOS;
    case '(': return Token::LPAREN;
    case ')': return Token::RPAREN;
    case '{': return Token::LBRACE;
    case '}': return Token::RBRACE;
    case ';': return Token::SEMI;
    case ':': return Token::COLON;
    case ',': return Token::COMMA;
    case '.': return Token::DOT;
    case '>': return Token::GT;
    case '=': return Token::EQ;
    case '/': return Token::SLASH;
    case '@': {
      Token tok = ident(c);
      if (tok.value == "@constrain") tok.type = Token::CONSTRAIN;
      else if (tok.value == "@context") tok.type = Token::CONTEXT;
      else if (tok.value == "@import") tok.type = Token::IMPORT;
      else if (tok.value == "@override") tok.type = Token::OVERRIDE;
      else throw std::runtime_error("TODO 1"); // TODO ERROR!
      return tok;
    }
    case '\'': return string('\'');
    case '"': return string('"');
    }

    if (c == '-' || std::isdigit(c)) return number(c);

    if (identInitChar(c)) return ident(c);

    throw std::runtime_error("TODO 2"); // TODO ERROR!
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
      if (c == EOF) throw std::runtime_error("Unterminated multi-line comment");
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

  Token string(char first) {
    StringVal result;
    std::string current;
    while (stream_.peek() != first) {
      switch (stream_.peek()) {
      case EOF:
        throw std::runtime_error("BAD EOF 1"); // TODO error
      case '$': {
        stream_.get();
        if (stream_.peek() != '{')
          throw std::runtime_error("EXPECTED {"); // TODO error
        stream_.get();
        if (!current.empty()) result.elements_.push_back(current);
        current = "";
        Interpolant interpolant;
        while (stream_.peek() != '}') {
          if (!interpolantChar(stream_.peek()))
              throw std::runtime_error("BAD INTERPOLANT CHAR"); // TODO error
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
          case EOF: throw std::runtime_error("BAD EOF 2"); // TODO error
          case '$': current += '$'; break;
          case '\'': current += '\''; break;
          case '"': current += '"'; break;
          case '\\': current += '\\'; break;
          case 't': current += '\t'; break;
          case 'n': current += '\n'; break;
          case 'r': current += '\r'; break;
          case '\n': break; // escaped newline: ignore
          default: std::cerr << "ESC " << escape << std::endl; throw std::runtime_error("BAD ESCAPE"); // TODO error
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
    auto tok = Token(Token::STRING);
    tok.stringValue = result;
    return tok;
  }

  bool numberChar(int c) {
    if ('0' <= c && c <= '9') return true;
    if (c == '-' || c == '.') return true;
    if (c == 'e' || c == 'E') return true;
    if (identInitChar(c)) return true; // TODO this isn't really right, see below...
    return false;
  }

  /*
   * TODO three types:
   *  INT/DOUBLE num: -? 0123 (. 0123)? (eE -? 0123 (. 0123)?)?
   *  IDENT? numIdent: num [a-zA-Z_$]+
   */
  Token number(char first) {
    if (first == '0' and stream_.peek() == 'x') {
      stream_.get();
      return hexLiteral();
    }

    Token token(Token::DOUBLE, first);
    while (numberChar(stream_.peek())) token.append(stream_.get());
    try {
      token.doubleValue = boost::lexical_cast<double>(token.value);
    } catch (const std::bad_cast &e) {
      throw std::runtime_error("BAD DOUBLE"); // TODO error
    }
    if (round(token.doubleValue) == token.doubleValue) {
      token.type = Token::INT;
      token.intValue = round(token.doubleValue);
    }
    return token;
  }

  int hexChar(int c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return 10 + c - 'a';
    if ('A' <= c && c <= 'F') return 10 + c - 'A';
    return -1;
  }

  Token hexLiteral() {
    Token token(Token::INT);
    for (int n = hexChar(stream_.peek()); n != -1;
        n = hexChar(stream_.peek())) {
      token.intValue = token.intValue * 16 + n;
      token.append(stream_.get());
    }
    token.doubleValue = token.intValue;
    return token;
  }

  Token ident(char first) {
    Token token(Token::IDENT, first);
    while (identChar(stream_.peek())) token.append(stream_.get());
    return token;
  }
};

}

class ParserImpl {
  Lexer lex_;
  Token cur_;
  Token last_;

public:
  ParserImpl(const std::string &fileName, std::istream &stream) : lex_(stream) {
    (void)fileName; // TODO
  }

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
      throw std::runtime_error("TODO 3"); // TODO error
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
        throw std::runtime_error("TODO 6"); // TODO error!
      advanceIf(Token::SEMI);
    } else if (advanceIf(Token::LBRACE)) {
      while (!advanceIf(Token::RBRACE)) parseRule(nested);
    } else {
      throw std::runtime_error("TODO 7"); // TODO error!
    }

    ast.addRule(nested);
  }

  bool parsePrimRule(ast::Nested &ast) {
    switch (cur_.type) {
    case Token::IMPORT:
      advance();
      expect(Token::STRING);
      if (last_.stringValue.interpolation())
        throw std::runtime_error("INTERPOLATION NOT ALLOWED HERE");
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
    prop.name_ = parseIdent();
    prop.override_ = override;
    // TODO track origins!
    expect(Token::EQ);

    switch (cur_.type) {
    case Token::INT:
      prop.value_.setInt(cur_.intValue); break;
    case Token::DOUBLE:
      prop.value_.setDouble(cur_.doubleValue); break;
    case Token::STRING:
      prop.value_.setString(cur_.stringValue); break;
    case Token::IDENT:
      // TODO deal with num+literal stuff?
      if (cur_.value == "true") prop.value_.setBool(true);
      else if (cur_.value == "false") prop.value_.setBool(false);
      else throw std::runtime_error("TODO 8"); // TODO error
      break;
    default:
      throw std::runtime_error("TODO 4"); // TODO error
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
      auto name = parseIdent();
      key.addName(name);
      while (advanceIf(Token::DOT)) key.addValue(name, parseIdent());
    } while (advanceIf(Token::SLASH));
    return key;
  }

  std::string parseIdent() {
    if (advanceIf(Token::IDENT)) return last_.value;
    if (advanceIf(Token::STRING)) {
      if (last_.stringValue.interpolation())
        throw std::runtime_error("INTERPOLATION NOT ALLOWED HERE");
      return last_.stringValue.str();
    }
    throw std::runtime_error("TODO 5"); // TODO error
  }
};

Parser2::Parser2(CcsLogger &log) : log(log) {}
Parser2::~Parser2() {}

bool Parser2::parseCcsStream(const std::string &fileName, std::istream &stream,
    ast::Nested &ast) {
  try {
    ParserImpl p(fileName, stream);
    if (p.parseRuleset(ast))
      return true;
    std::ostringstream msg;
    msg << "Unknown error parsing " << fileName + "!";
    log.error(msg.str());
  } catch (const std::runtime_error &e) {
//    const classic::file_position_base<std::string> &pos =
//        e.first.get_position();
    std::ostringstream msg;
//    msg << "Parse error at file " << pos.file << ':' << pos.line << ':'
//        << pos.column << ": '" << e.first.get_currentline() << "'";
    msg << "Parse error: " << e.what();
    log.error(msg.str());
  }
  return false;
}

}
