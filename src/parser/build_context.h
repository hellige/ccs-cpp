#ifndef CCS_PARSER_BUILD_CONTEXT_H_
#define CCS_PARSER_BUILD_CONTEXT_H_

#include <memory>

#include "ccs/types.h"

namespace ccs {

class DagBuilder;
class Node;
namespace ast { class PropDef; }
namespace ast { class SelectorLeaf; }

class BuildContext {
protected:
  DagBuilder &dag_;

  BuildContext(DagBuilder &dag) : dag_(dag) {}

public:
  typedef std::shared_ptr<BuildContext> P;
  virtual ~BuildContext() {}

  virtual Node &node() = 0;
  virtual Node &traverse(ast::SelectorLeaf *selector) = 0;

  static BuildContext::P descendant(DagBuilder &dag, Node &root);
  BuildContext::P descendant(Node &node);
  BuildContext::P conjunction(Node &node, BuildContext::P baseContext);
  BuildContext::P disjunction(Node &node, BuildContext::P baseContext);
  void addProperty(const ast::PropDef &propDef);
};

}


#endif /* CCS_PARSER_BUILD_CONTEXT_H_ */
