#ifndef CCS_PARSER_PARSER2_H_
#define CCS_PARSER_PARSER2_H_

#include <string>
#include <istream>

#include "ccs/domain.h"
#include "parser/ast.h"

namespace ccs {

class Node;

class Parser2 {
  CcsLogger &log;

public:
  Parser2(CcsLogger &log);
  ~Parser2();

  bool parseCcsStream(const std::string &fileName, std::istream &stream,
      ast::Nested &ast);
};

}


#endif /* CCS_PARSER_PARSER2_H_ */
