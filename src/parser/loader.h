#ifndef LOADER_H_
#define LOADER_H_

#include <deque>
#include <istream>
#include <string>

#include "parser/ast.h"
#include "parser/parser.h"
#include "dag/dag_builder.h"

namespace ccs {

// TODO move
class ImportResolver;

class Loader {
public:
  void loadCcsStream(std::istream &stream, const std::string &fileName,
      DagBuilder &dag, ImportResolver &importResolver) {
    ast::Nested ast;
    std::deque<std::string> inProgress;
    if (parseCcsStream(stream, fileName, importResolver, inProgress, ast)) {
      // everything parsed, no errors. now it's safe to modify the dag...
      ast.addTo(dag.buildContext(), dag.buildContext());
    } else {
      // TODO how to report? already reported?
    }
  }

  bool parseCcsStream(std::istream &stream, const std::string &fileName,
      ImportResolver &importResolver, std::deque<std::string> &inProgress,
      ast::Nested &ast) {
    Parser parser;
    bool result = parser.parseCcsStream(stream, ast);

    if (!result) {
      // TODO how to report? already reported?
      //log.error("Errors parsing " + fileName + ":" + ErrorUtils.printParseErrors(result));
      return false;
    }

    if (!ast.resolveImports(importResolver, *this, inProgress)) return false;

    return true;
  }
};


}



#endif /* LOADER_H_ */
