#ifndef CCS_PARSER_PARSER_H_
#define CCS_PARSER_PARSER_H_

#include <string>
#include <istream>
#include <memory>

#include "ccs/domain.h"
#include "parser/ast.h"

namespace ccs {

class Node;

class Parser {
  class Impl;
  CcsLogger &log;
  std::unique_ptr<Impl> impl;

public:
  Parser(CcsLogger &log);
  ~Parser();

  bool parseCcsStream(const std::string &fileName, std::istream &stream,
      ast::Nested &ast);
};

}


#endif /* CCS_PARSER_PARSER_H_ */
