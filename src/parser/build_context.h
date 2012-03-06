#ifndef BUILD_CONTEXT_H_
#define BUILD_CONTEXT_H_


namespace ccs {

class DagBuilder;
class Node;

namespace bc {

class BuildContext {
protected:
  DagBuilder &dag_;

  BuildContext(DagBuilder &dag) : dag_(dag) {}
};

class Descendant : public BuildContext {
  Node &node_;

public:
  Descendant(DagBuilder &dag, Node &node) :
    BuildContext(dag),
    node_(node) {}
};

}}


#endif /* BUILD_CONTEXT_H_ */
