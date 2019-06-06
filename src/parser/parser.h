#pragma once

#include <string>
#include <istream>

#include "ccs/domain.h"
#include "parser/ast.h"

namespace ccs {

class Node;

class Parser {
  CcsTracer &tracer;

public:
  Parser(CcsTracer &tracer);
  ~Parser();

  bool parseCcsStream(const std::string &fileName, std::istream &stream,
      ast::Nested &ast);
};

}
