#include "parser/ast.h"

#include <boost/variant.hpp>

#include "ccs/domain.h"
#include "dag/node.h"
#include "parser/build_context.h"
#include "parser/loader.h"

namespace ccs { namespace ast {

struct AstVisitor : public boost::static_visitor<void> {
  BuildContext::P buildContext;
  BuildContext::P baseContext;

  AstVisitor(BuildContext::P buildContext, BuildContext::P baseContext) :
    buildContext(buildContext), baseContext(baseContext) {}
  void operator()(Import &import) const
    { import.ast.addTo(buildContext, baseContext); }
  void operator()(PropDef &propDef) const
    { buildContext->addProperty(propDef); }
  void operator()(Constraint &constraint) const
    { buildContext->node().addConstraint(constraint.key_); }
  void operator()(Nested &nested) const
    { nested.addTo(buildContext, baseContext); }
};

struct ImportVisitor : public boost::static_visitor<bool> {
  ImportResolver &resolver;
  Loader &loader;
  std::vector<std::string> &inProgress;

  ImportVisitor(ImportResolver &importResolver, Loader &loader,
      std::vector<std::string> &inProgress) :
    resolver(importResolver), loader(loader), inProgress(inProgress) {}
  bool operator()(PropDef &propDef) const
    { return true; }
  bool operator()(Constraint &constraint) const
    { return true; }
  bool operator()(Nested &nested) const
    { return nested.resolveImports(resolver, loader, inProgress); }

  bool operator()(Import &import) const {
    bool result = false;
    if (std::find(inProgress.begin(), inProgress.end(), import.location) !=
        inProgress.end()) {
      std::ostringstream msg;
      msg << "Circular import detected involving '" << import.location << "'";
      loader.logger().error(msg.str());
    } else {
      inProgress.push_back(import.location);
      // TODO
//      std::istream stream(resolver.resolve(import.location));
      std::istream *stream;
      Nested ast;
      result = loader.parseCcsStream(*stream, import.location, resolver,
          inProgress, ast);
      inProgress.pop_back();
    }
    return result;
  }
};

void Nested::addTo(BuildContext::P buildContext, BuildContext::P baseContext) {
  BuildContext::P bc = buildContext;
  if (selector_)
    bc = selector_->traverse(buildContext, baseContext);
  for (auto it = rules_.begin(); it != rules_.end(); ++it)
    boost::apply_visitor(AstVisitor(bc, baseContext), *it);
}

bool Nested::resolveImports(ImportResolver &importResolver, Loader &loader,
    std::vector<std::string> &inProgress) {
  for (auto it = rules_.begin(); it != rules_.end(); ++it)
    if (!boost::apply_visitor(ImportVisitor(importResolver, loader, inProgress),
        *it))
      return false;
  return true;
}


struct Wrap : public SelectorLeaf {
  std::vector<SelectorBranch *> branches;
  SelectorLeaf *right;

  virtual ~Wrap() {
    for (auto it = branches.begin(); it != branches.end(); ++it) delete *it;
    delete right;
  }

  Wrap(SelectorBranch *branch, SelectorLeaf *leaf) : right(leaf)
    { branches.push_back(branch); }

  virtual SelectorLeaf *descendant(SelectorLeaf *right)
    { return push(SelectorBranch::descendant(this->right), right); }
  virtual SelectorLeaf *conjunction(SelectorLeaf *right)
    { return push(SelectorBranch::conjunction(this->right), right); }
  virtual SelectorLeaf *disjunction(SelectorLeaf *right)
    { return push(SelectorBranch::disjunction(this->right), right); }

  SelectorLeaf *push(SelectorBranch *newBranch, SelectorLeaf *newRight) {
    branches.push_back(newBranch);
    right = newRight;
    return this;
  }

  virtual Node &traverse(BuildContext::P context) {
    BuildContext::P tmp = context;
    for (auto it = branches.begin(); it != branches.end(); ++it)
      tmp = (*it)->traverse(tmp, context);
    return tmp->traverse(right);
  }
};

struct Step : public SelectorLeaf {
  Key key_;

  Step(const Key &key) : key_(key) {}

  virtual SelectorLeaf *descendant(SelectorLeaf *right)
    { return new Wrap(SelectorBranch::descendant(this), right); }
  virtual SelectorLeaf *conjunction(SelectorLeaf *right)
    { return new Wrap(SelectorBranch::conjunction(this), right); }
  virtual SelectorLeaf *disjunction(SelectorLeaf *right)
    { return new Wrap(SelectorBranch::disjunction(this), right); }

  virtual Node &traverse(BuildContext::P context)
    { return context->node().addChild(key_); }
};

SelectorLeaf *SelectorLeaf::step(const Key &key) {
  return new Step(key);
}

class BranchImpl : public SelectorBranch {
  typedef std::function<BuildContext::P (SelectorLeaf *, BuildContext::P,
      BuildContext::P)> Traverse;
  std::unique_ptr<SelectorLeaf> first_;
  Traverse traverse_;

public:
  BranchImpl(SelectorLeaf *first, Traverse traverse) :
    first_(first), traverse_(traverse) {}

  virtual BuildContext::P traverse(BuildContext::P context,
      BuildContext::P baseContext) {
    return traverse_(first_.get(), context, baseContext);
  }
};

SelectorBranch *SelectorBranch::descendant(SelectorLeaf *first) {
  return new BranchImpl(first, [](SelectorLeaf *first, BuildContext::P context,
      BuildContext::P baseContext) {
    return context->descendant(context->traverse(first));
  });
}

SelectorBranch *SelectorBranch::conjunction(SelectorLeaf *first) {
  return new BranchImpl(first, [](SelectorLeaf *first, BuildContext::P context,
      BuildContext::P baseContext) {
    return context->conjunction(context->traverse(first), baseContext);
  });
}

SelectorBranch *SelectorBranch::disjunction(SelectorLeaf *first) {
  return new BranchImpl(first, [](SelectorLeaf *first, BuildContext::P context,
      BuildContext::P baseContext) {
    return context->disjunction(context->traverse(first), baseContext);
  });
}

}}
