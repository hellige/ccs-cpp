#ifndef CCS_DAG_DAG_BUILDER_H_
#define CCS_DAG_DAG_BUILDER_H_

#include <memory>

#include "dag/node.h"
#include "parser/build_context.h"

namespace ccs {

class DagBuilder {
  int nextProperty_;
  std::shared_ptr<Node> root_;
  std::shared_ptr<BuildContext> buildContext_;

public:
  DagBuilder(std::shared_ptr<CcsTracer> tracer) :
    nextProperty_(0),
    root_(new Node(std::move(tracer))),
    buildContext_(BuildContext::descendant(*this, *root_)) {}

  std::shared_ptr<const Node> root() { return root_; }
  BuildContext::P buildContext() { return buildContext_; }
  int nextProperty() { return nextProperty_++; }
};

}


#endif /* CCS_DAG_DAG_BUILDER_H_ */
