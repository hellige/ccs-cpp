#include "parser/build_context.h"

#include "dag/dag_builder.h"
#include "dag/node.h"
#include "parser/ast.h"

namespace ccs { namespace bc {

BuildContext *BuildContext::descendant(Node &node)
  { return new Descendant(dag_, node); }
BuildContext *BuildContext::conjunction(Node &node, BuildContext &baseContext)
  { return new Descendant(dag_, node); } // TODO
BuildContext *BuildContext::disjunction(Node &node, BuildContext &baseContext)
  { return new Descendant(dag_, node); } // TODO

void BuildContext::addProperty(const ast::PropDef &propDef) {
  node().addProperty(propDef.name_, Property(propDef.value_.strVal_,
      propDef.origin_, dag_.nextProperty(), propDef.override_), propDef.local_);
}

Node &Descendant::traverse(ast::SelectorLeaf *selector) {
  return selector->traverse(*this);
}

}}
