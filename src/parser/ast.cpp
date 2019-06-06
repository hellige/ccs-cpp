#include "parser/ast.h"

#include "ccs/domain.h"
#include "dag/node.h"
#include "parser/build_context.h"
#include "parser/loader.h"

namespace ccs { namespace ast {

void Import::addTo(BuildContext::P buildContext, BuildContext::P baseContext) const
  { ast.addTo(buildContext, baseContext); }
void PropDef::addTo(BuildContext::P buildContext, BuildContext::P) const
  { buildContext->addProperty(*this); }
void Constraint::addTo(BuildContext::P buildContext, BuildContext::P) const
  { buildContext->node().addConstraint(key_); }

bool PropDef::resolveImports(ImportResolver &, Loader &,
    std::vector<std::string> &)
  { return true; }
bool Constraint::resolveImports(ImportResolver &, Loader &,
    std::vector<std::string> &)
  { return true; }

bool Import::resolveImports(ImportResolver &importResolver, Loader &loader,
    std::vector<std::string> &inProgress) {
  bool result = false;
  if (std::find(inProgress.begin(), inProgress.end(), location) !=
      inProgress.end()) {
    std::ostringstream msg;
    msg << "Circular import detected involving '" << location << "'";
    loader.tracer().onParseError(msg.str());
  } else {
    inProgress.push_back(location);
    result = importResolver.resolve(location,
        [&](std::istream &stream) {
      return loader.parseCcsStream(stream, location,
          importResolver, inProgress, ast);
    });
    inProgress.pop_back();
    if (!result) {
      std::ostringstream msg;
      msg << "Failed to resolve '" << location
        << "'! (User-provided resolver returned false.)";
      loader.tracer().onParseError(msg.str());
    }
  }
  return result;
}


void Nested::addTo(BuildContext::P buildContext, BuildContext::P baseContext) const {
  BuildContext::P bc = buildContext;
  if (selector_)
    bc = selector_->traverse(buildContext, baseContext);
  for (auto it = rules_.begin(); it != rules_.end(); ++it)
    (*it)->addTo(bc, baseContext);
}

bool Nested::resolveImports(ImportResolver &importResolver, Loader &loader,
    std::vector<std::string> &inProgress) {
  for (auto it = rules_.begin(); it != rules_.end(); ++it)
    if (!(*it)->resolveImports(importResolver, loader, inProgress))
      return false;
  return true;
}


struct Wrap : public SelectorLeaf {
  std::vector<SelectorBranch::P> branches;
  SelectorLeaf::P right;

  Wrap(SelectorBranch::P branch, SelectorLeaf::P leaf) : right(leaf)
    { branches.push_back(branch); }

  // left == this, always. guess we could just use enable_shared_from_this,
  // but that's just as ugly... spirit just doesn't like this way of doing
  // things... it works fine, though, and i don't want to invest a lot of
  // time in switching to some kind of value-types-in-a-variant design just
  // to play nicer with spirit. it's also worth something to stay roughly
  // in sync with the java version. anyway, maybe reconsider down the road.
  virtual P descendant(P left, P right)
    { push(SelectorBranch::descendant(this->right), right); return left; }
  virtual P conjunction(P left, P right)
    { push(SelectorBranch::conjunction(this->right), right); return left; }
  virtual P disjunction(P left, P right)
    { push(SelectorBranch::disjunction(this->right), right); return left; }

  void push(SelectorBranch::P newBranch, SelectorLeaf::P newRight) {
    branches.push_back(newBranch);
    right = newRight;
  }

  virtual Node &traverse(BuildContext::P context) {
    BuildContext::P tmp = context;
    for (auto it = branches.begin(); it != branches.end(); ++it)
      tmp = (*it)->traverse(tmp, context);
    return tmp->traverse(*right);
  }
};

struct Step : public SelectorLeaf {
  Key key_;

  Step(const Key &key) : key_(key) {}

  virtual P descendant(P left, P right)
    { return std::make_shared<Wrap>(SelectorBranch::descendant(left), right); }
  virtual P conjunction(P left, P right)
    { return std::make_shared<Wrap>(SelectorBranch::conjunction(left), right); }
  virtual P disjunction(P left, P right)
    { return std::make_shared<Wrap>(SelectorBranch::disjunction(left), right); }

  virtual Node &traverse(BuildContext::P context)
    { return context->node().addChild(key_); }
};

SelectorLeaf::P SelectorLeaf::step(const Key &key) {
  return std::make_shared<Step>(key);
}

class BranchImpl : public SelectorBranch {
  typedef std::function<BuildContext::P (SelectorLeaf &, BuildContext::P,
      BuildContext::P)> Traverse;
  SelectorLeaf::P first_;
  Traverse traverse_;

public:
  BranchImpl(SelectorLeaf::P first, Traverse traverse) :
    first_(first), traverse_(traverse) {}

  virtual BuildContext::P traverse(BuildContext::P context,
      BuildContext::P baseContext) {
    return traverse_(*first_, context, baseContext);
  }
};

SelectorBranch::P SelectorBranch::descendant(SelectorLeaf::P first) {
  return std::make_shared<BranchImpl>(first, [](SelectorLeaf &first,
              BuildContext::P context, BuildContext::P baseContext) {
    (void)baseContext;
    return context->descendant(context->traverse(first));
  });
}

SelectorBranch::P SelectorBranch::conjunction(SelectorLeaf::P first) {
  return std::make_shared<BranchImpl>(first, [](SelectorLeaf &first,
              BuildContext::P context, BuildContext::P baseContext) {
    return context->conjunction(context->traverse(first), baseContext);
  });
}

SelectorBranch::P SelectorBranch::disjunction(SelectorLeaf::P first) {
  return std::make_shared<BranchImpl>(first, [](SelectorLeaf &first,
              BuildContext::P context, BuildContext::P baseContext) {
    return context->disjunction(context->traverse(first), baseContext);
  });
}

}}
