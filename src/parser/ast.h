#ifndef CCS_PARSER_AST_H_
#define CCS_PARSER_AST_H_

#include <boost/lexical_cast.hpp>
#include <boost/variant/recursive_variant.hpp>

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

struct PropDef {
  std::string name_;
  Value value_;
  Origin origin_;
  bool override_;

  PropDef() : override_(false) {}
};


struct Constraint {
  Key key_;
};

struct Import;
struct Nested;

struct AstRule {
typedef boost::variant<
    PropDef,
    Constraint,
    std::shared_ptr<Import>,
    std::shared_ptr<Nested>>
  Guts;

  Guts guts;

  AstRule() {}
  AstRule(const PropDef &p) : guts(p) {}
  AstRule(const Constraint &c) : guts(c) {}
  AstRule(const Import &i);
  AstRule(const Nested &n);

  AstRule(const AstRule &) = default;
  AstRule &operator=(const AstRule &) = default;
};


struct SelectorLeaf {
  typedef std::shared_ptr<SelectorLeaf> P;
  virtual ~SelectorLeaf() {};
  virtual Node &traverse(std::shared_ptr<BuildContext> context) = 0;

  static P desc(P left, P right)
    { return left->descendant(left, right); }
  static P conj(P left, P right)
    { return left->conjunction(left, right); }
  static P disj(P left, P right)
    { return left->disjunction(left, right); }
  static P step(const Key &key);

private:
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


struct Nested : boost::static_visitor<void> {
  std::shared_ptr<SelectorBranch> selector_;
  std::vector<AstRule> rules_;

  void operator()(const Import &i) { rules_.push_back(AstRule(i)); }
  void operator()(const PropDef &p) { rules_.push_back(AstRule(p)); }
  void operator()(const Constraint &c) { rules_.push_back(AstRule(c)); }

  void addRule(const boost::variant<ccs::ast::Import, ccs::ast::Constraint,
       ccs::ast::PropDef> &rule) {
    boost::apply_visitor(*this, rule);
  }

  void addNested(const Nested &rule) { rules_.push_back(AstRule(rule)); }
  void addTo(std::shared_ptr<BuildContext> buildContext,
      std::shared_ptr<BuildContext> baseContext);
  bool resolveImports(ImportResolver &importResolver, Loader &loader,
      std::vector<std::string> &inProgress);
};

struct Import {
  std::string location;
  Nested ast;
};

}}


#endif /* CCS_PARSER_AST_H_ */
