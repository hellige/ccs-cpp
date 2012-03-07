#include "parser/build_context.h"

#include "parser/ast.h"

namespace ccs { namespace bc {

BuildContext *BuildContext::descendant(Node &node)
  { return new Descendant(dag_, node); }
BuildContext *BuildContext::conjunction(Node &node, BuildContext &baseContext)
  { return new Descendant(dag_, node); } // TODO
BuildContext *BuildContext::disjunction(Node &node, BuildContext &baseContext)
  { return new Descendant(dag_, node); } // TODO

Node &Descendant::traverse(ast::SelectorLeaf *selector) {
  return selector->traverse(*this);
}

}}
