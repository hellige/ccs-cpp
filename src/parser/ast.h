#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ccs/types.h"
#include "dag/key.h"
#include "dag/property.h"

namespace ccs {

class Loader;
class ImportResolver;
class Node;
class BuildContext;

namespace ast {

struct AstRule {
  virtual ~AstRule() {}
  virtual void addTo(std::shared_ptr<BuildContext> buildContext,
    std::shared_ptr<BuildContext> baseContext) const = 0;
  virtual bool resolveImports(ImportResolver &importResolver, Loader &loader,
    std::vector<std::string> &inProgress) = 0;

};

struct PropDef : AstRule {
  std::string name_;
  Value value_;
  Origin origin_;
  bool override_;

  PropDef() : override_(false) {}
  void addTo(std::shared_ptr<BuildContext> buildContext,
    std::shared_ptr<BuildContext> baseContext) const override;
  bool resolveImports(ImportResolver &importResolver, Loader &loader,
    std::vector<std::string> &inProgress) override;
};

struct Constraint : AstRule {
  Key key_;
  explicit Constraint(const Key &key) : key_(key) {}
  void addTo(std::shared_ptr<BuildContext> buildContext,
    std::shared_ptr<BuildContext> baseContext) const override;
  bool resolveImports(ImportResolver &importResolver, Loader &loader,
    std::vector<std::string> &inProgress) override;
};

struct SelectorLeaf {
  typedef std::shared_ptr<SelectorLeaf> P;
  virtual ~SelectorLeaf() {};
  virtual Node &traverse(std::shared_ptr<BuildContext> context) = 0;

  static P step(const Key &key);

  virtual P descendant(P left, P right) = 0;
  virtual P conjunction(P left, P right) = 0;
  virtual P disjunction(P left, P right) = 0;
};

struct SelectorBranch {
  typedef std::shared_ptr<SelectorBranch> P;
  virtual ~SelectorBranch() {}
  virtual std::shared_ptr<BuildContext> traverse(
      std::shared_ptr<BuildContext> context,
      std::shared_ptr<BuildContext> baseContext) = 0;

  static P descendant(SelectorLeaf::P first);
  static P conjunction(SelectorLeaf::P first);
  static P disjunction(SelectorLeaf::P first);
};

struct Nested : AstRule {
  std::shared_ptr<SelectorBranch> selector_;
  std::vector<std::unique_ptr<AstRule>> rules_;

  void addRule(std::unique_ptr<AstRule> rule) {
    rules_.push_back(std::move(rule));
  }
  void addTo(std::shared_ptr<BuildContext> buildContext,
      std::shared_ptr<BuildContext> baseContext) const override;
  bool resolveImports(ImportResolver &importResolver, Loader &loader,
      std::vector<std::string> &inProgress) override;
};

struct Import : AstRule {
  std::string location;
  Nested ast;

  explicit Import(const std::string &location) : location(location) {}
  void addTo(std::shared_ptr<BuildContext> buildContext,
    std::shared_ptr<BuildContext> baseContext) const override;
  bool resolveImports(ImportResolver &importResolver, Loader &loader,
    std::vector<std::string> &inProgress) override;
};

}}
