#ifndef BUILD_CONTEXT_H_
#define BUILD_CONTEXT_H_


namespace ccs {

class DagBuilder;
class Node;
namespace ast { class SelectorLeaf; }

namespace bc {

class BuildContext {
protected:
  DagBuilder &dag_;

  BuildContext(DagBuilder &dag) : dag_(dag) {}
  virtual ~BuildContext() {}

public:
  virtual Node &node() = 0;
  virtual Node &traverse(ast::SelectorLeaf *selector) = 0;

  BuildContext *descendant(Node &node);
  BuildContext *conjunction(Node &node, BuildContext &baseContext);
  BuildContext *disjunction(Node &node, BuildContext &baseContext);
};

class Descendant : public BuildContext {
  Node &node_;

public:
  Descendant(DagBuilder &dag, Node &node) :
    BuildContext(dag),
    node_(node) {}

  virtual Node &node() { return node_; }
  virtual Node &traverse(ast::SelectorLeaf *selector);
};

}}


#endif /* BUILD_CONTEXT_H_ */
