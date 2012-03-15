#ifndef BUILD_CONTEXT_H_
#define BUILD_CONTEXT_H_

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
  virtual ~BuildContext() {}

  virtual Node &node() = 0;
  virtual Node &traverse(ast::SelectorLeaf *selector) = 0;

  static BuildContext *descendant(DagBuilder &dag, Node &root);
  BuildContext *descendant(Node &node);
  BuildContext *conjunction(Node &node, BuildContext &baseContext);
  BuildContext *disjunction(Node &node, BuildContext &baseContext);
  void addProperty(const ast::PropDef &propDef);
};

}


#endif /* BUILD_CONTEXT_H_ */
