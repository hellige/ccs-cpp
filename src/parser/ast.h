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

typedef boost::variant<
    PropDef,
    Constraint,
    boost::recursive_wrapper<Import>,
    boost::recursive_wrapper<Nested>>
  AstRule;


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


struct Nested {
  std::shared_ptr<SelectorBranch> selector_;
  std::vector<AstRule> rules_;

  void addRule(const AstRule &rule) { rules_.push_back(rule); }
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
