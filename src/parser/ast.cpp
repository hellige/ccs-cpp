#include "ast.h"

namespace ccs { namespace ast {

struct Wrap : public SelectorLeaf {
  std::vector<SelectorBranch *> branches;
  SelectorLeaf *right;

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
};

SelectorLeaf *SelectorLeaf::step(const Key &key) {
  return new Step(key);
}

struct BranchImpl : SelectorBranch {
  SelectorLeaf *first;
  std::function<void()> traverse;

  BranchImpl(SelectorLeaf *first, std::function<void()> traverse)
  : first(first), traverse(traverse) {}
};

SelectorBranch *SelectorBranch::descendant(SelectorLeaf *first) {
  return new BranchImpl(first, [](){});
}

SelectorBranch *SelectorBranch::conjunction(SelectorLeaf *first) {
  return new BranchImpl(first, [](){});
}

SelectorBranch *SelectorBranch::disjunction(SelectorLeaf *first) {
  return new BranchImpl(first, [](){});
}

}}
