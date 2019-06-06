#pragma once

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
