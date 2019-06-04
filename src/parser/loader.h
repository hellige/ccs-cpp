#ifndef CCS_PARSER_LOADER_H_
#define CCS_PARSER_LOADER_H_

#include <istream>
#include <string>

#include "parser/ast.h"
#include "parser/parser.h"
#include "dag/dag_builder.h"

namespace ccs {

class Loader {
  CcsTracer &trace;

public:
  Loader(CcsTracer &trace) : trace(trace) {}

  CcsTracer &tracer() { return trace; }

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
    Parser parser(trace);
    if (!parser.parseCcsStream(fileName, stream, ast)) return false;
    if (!ast.resolveImports(importResolver, *this, inProgress)) return false;
    return true;
  }
};


}



#endif /* CCS_PARSER_LOADER_H_ */
