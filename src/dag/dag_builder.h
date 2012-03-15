#ifndef DAG_BUILDER_H_
#define DAG_BUILDER_H_

#include <memory>

#include "dag/node.h"
#include "parser/build_context.h"

namespace ccs {

class DagBuilder {
  int nextProperty_;
  std::shared_ptr<Node> root_;
  std::shared_ptr<BuildContext> buildContext_;

public:
  DagBuilder() :
    nextProperty_(0),
    root_(new Node()),
    buildContext_(BuildContext::descendant(*this, *root_)) {}

  Node &root() { return *root_; }
  BuildContext &buildContext() { return *buildContext_; }
  int nextProperty() { return nextProperty_++; }
};

}


#endif /* DAG_BUILDER_H_ */
