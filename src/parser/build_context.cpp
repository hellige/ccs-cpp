#include "parser/build_context.h"

#include <algorithm>
#include <set>

#include "dag/dag_builder.h"
#include "dag/node.h"
#include "dag/tally.h"
#include "parser/ast.h"

namespace ccs {

class Descendant : public BuildContext {
  Node &node_;

public:
  Descendant(DagBuilder &dag, Node &node) :
    BuildContext(dag),
    node_(node) {}

  virtual Node &node() { return node_; }
  // this copy-into-new-shared-ptr thing is ugly, but enable_shared_from_this
  // is just as bad, or worse...
  virtual Node &traverse(ast::SelectorLeaf &selector)
    { return selector.traverse(std::make_shared<Descendant>(*this)); }
};

template <typename T>
class TallyBuildContext : public BuildContext {
  Node &firstNode_;
  BuildContext::P baseContext_;

public:
  TallyBuildContext(DagBuilder &dag, Node &node, BuildContext::P baseContext) :
    BuildContext(dag),
    firstNode_(node),
    baseContext_(baseContext) {}

  virtual Node &node() { return firstNode_; }

  virtual Node &traverse(ast::SelectorLeaf &selector) {
    Node &secondNode = selector.traverse(baseContext_);
    std::set<std::shared_ptr<Tally>> tallies;

    std::set_intersection(
        firstNode_.tallies().begin(), firstNode_.tallies().end(),
        secondNode.tallies().begin(), secondNode.tallies().end(),
        std::inserter(tallies, tallies.end()));

    // result will be either empty or have exactly one entry.

    if (tallies.empty()) {
      std::shared_ptr<Tally> tally = std::make_shared<T>(firstNode_,
          secondNode);
      firstNode_.addTally(tally);
      secondNode.addTally(tally);
      return tally->node();
    } else {
      return (*tallies.begin())->node();
    }
  }
};

BuildContext::P BuildContext::descendant(DagBuilder &dag, Node &node)
  { return std::make_shared<Descendant>(dag, node); }
BuildContext::P BuildContext::descendant(Node &node)
  { return std::make_shared<Descendant>(dag_, node); }
BuildContext::P BuildContext::conjunction(Node &node,
    BuildContext::P baseContext)
  { return std::make_shared<TallyBuildContext<AndTally>>(dag_, node,
      baseContext); }
BuildContext::P BuildContext::disjunction(Node &node,
    BuildContext::P baseContext)
  { return std::make_shared<TallyBuildContext<OrTally>>(dag_, node,
      baseContext); }

void BuildContext::addProperty(const ast::PropDef &propDef) {
  Value value(propDef.value_);
  value.setName(propDef.name_);
  node().addProperty(propDef.name_, Property(value,
      propDef.origin_, dag_.nextProperty(), propDef.override_));
}

}
