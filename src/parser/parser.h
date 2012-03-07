#ifndef CCS_PARSER_PARSER_H_
#define CCS_PARSER_PARSER_H_

#include <string>
#include <istream>

#include "parser/ast.h"

namespace ccs {

class Node;

class Parser {
public:
  bool parseCcsStream(std::istream &stream, ast::Nested &ast);
};

}


#endif /* CCS_PARSER_PARSER_H_ */
