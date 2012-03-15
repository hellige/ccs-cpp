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
  virtual Node &traverse(ast::SelectorLeaf *selector)
    { return selector->traverse(*this); }
};

template <typename T>
class TallyBuildContext : public BuildContext {
  Node &firstNode_;
  BuildContext &baseContext_;

public:
  TallyBuildContext(DagBuilder &dag, Node &node, BuildContext &baseContext) :
    BuildContext(dag),
    firstNode_(node),
    baseContext_(baseContext) {}

  virtual Node &node() { return firstNode_; }

  virtual Node &traverse(ast::SelectorLeaf *selector) {
    Node &secondNode = selector->traverse(baseContext_);
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

BuildContext *BuildContext::descendant(DagBuilder &dag, Node &node)
  { return new Descendant(dag, node); }
BuildContext *BuildContext::descendant(Node &node)
  { return new Descendant(dag_, node); }
BuildContext *BuildContext::conjunction(Node &node, BuildContext &baseContext)
  { return new TallyBuildContext<AndTally>(dag_, node, baseContext); }
BuildContext *BuildContext::disjunction(Node &node, BuildContext &baseContext)
  { return new TallyBuildContext<OrTally>(dag_, node, baseContext); }

void BuildContext::addProperty(const ast::PropDef &propDef) {
  node().addProperty(propDef.name_, Property(propDef.value_.strVal_,
      propDef.origin_, dag_.nextProperty(), propDef.override_), propDef.local_);
}

}
