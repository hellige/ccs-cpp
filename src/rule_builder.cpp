#include "ccs/rule_builder.h"

#include "dag/dag_builder.h"
#include "parser/ast.h"

namespace ccs {

struct RuleBuilder::Impl : std::enable_shared_from_this<Impl> {
  ast::Nested ast;

  virtual ~Impl() {}

  void set(const std::string &name, const std::string &value) {
    ast::PropDef def;
    def.name_ = name;
    def.value_.setString(StringVal(value));
    ast.addRule(def);
  }

  void add(const ast::Nested &child) { ast.addRule(child); }
  std::shared_ptr<Impl> select(const std::string &name,
      const std::vector<std::string> &values);
  virtual std::shared_ptr<Impl> &pop() = 0;
};

struct RuleBuilder::Root : RuleBuilder::Impl {
  DagBuilder &dag;

  Root(DagBuilder &dag) : dag(dag) {}
  ~Root() { ast.addTo(dag.buildContext(), dag.buildContext()); }
  std::shared_ptr<Impl> &pop() { throw std::runtime_error("unmatched pop()!"); }
};


struct RuleBuilder::Child : RuleBuilder::Impl {
  std::shared_ptr<Impl> parent;

  Child(const std::shared_ptr<Impl> &parent, const std::string &name,
      const std::vector<std::string> &values) : parent(parent) {
    Key key(name, values);
    ast.selector_ = ast::SelectorBranch::disjunction(
        ast::SelectorLeaf::step(key));
  }

  ~Child() { parent->add(ast); }
  std::shared_ptr<Impl> &pop() { return parent; }
};

std::shared_ptr<RuleBuilder::Impl> RuleBuilder::Impl::select(
    const std::string &name, const std::vector<std::string> &values) {
  return std::shared_ptr<Impl>(new Child(shared_from_this(), name, values));
}

RuleBuilder::RuleBuilder(DagBuilder &dag) : impl(new Root(dag)) {}

RuleBuilder RuleBuilder::pop() {
  return RuleBuilder(impl->pop());
}

RuleBuilder RuleBuilder::set(const std::string &name,
    const std::string &value) {
  impl->set(name, value);
  return *this;
}

RuleBuilder RuleBuilder::select(const std::string &name) {
  return select(name, std::vector<std::string>());
}

RuleBuilder RuleBuilder::select(const std::string &name,
    const std::vector<std::string> &values) {
  return RuleBuilder(impl->select(name, values));
}

}
