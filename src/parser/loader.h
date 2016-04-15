#ifndef CCS_PARSER_LOADER_H_
#define CCS_PARSER_LOADER_H_

#include <istream>
#include <string>

#include "parser/ast.h"
#include "parser/parser2.h"
#include "dag/dag_builder.h"

namespace ccs {

class Loader {
  CcsLogger &log;

public:
  Loader(CcsLogger &log) : log(log) {}

  CcsLogger &logger() { return log; }

  void loadCcsStream(std::istream &stream, const std::string &fileName,
      DagBuilder &dag, ImportResolver &importResolver) {
    ast::Nested ast;
    std::vector<std::string> inProgress;
    if (parseCcsStream(stream, fileName, importResolver, inProgress, ast)) {
      // everything parsed, no errors. now it's safe to modify the dag...
      ast.addTo(dag.buildContext(), dag.buildContext());
    }
    // otherwise, errors already reported, don't modify the dag...
  }

  bool parseCcsStream(std::istream &stream, const std::string &fileName,
      ImportResolver &importResolver, std::vector<std::string> &inProgress,
      ast::Nested &ast) {
    Parser2 parser(log);
    if (!parser.parseCcsStream(fileName, stream, ast)) return false;
    if (!ast.resolveImports(importResolver, *this, inProgress)) return false;
    return true;
  }
};


}



#endif /* CCS_PARSER_LOADER_H_ */
