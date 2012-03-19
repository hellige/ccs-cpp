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
  Origin origin_; // TODO track!
  bool local_;
  bool override_;

  PropDef() : local_(false), override_(false) {}

  void setOrigin() { origin_.fileName = "OK!"; }
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

struct SelectorLeaf;

struct SelectorBranch {
  virtual ~SelectorBranch() {}
  virtual std::shared_ptr<BuildContext> traverse(
      std::shared_ptr<BuildContext> context,
      std::shared_ptr<BuildContext> baseContext) = 0;

  static SelectorBranch *descendant(SelectorLeaf *first);
  static SelectorBranch *conjunction(SelectorLeaf *first);
  static SelectorBranch *disjunction(SelectorLeaf *first);
};

struct SelectorLeaf {
  virtual ~SelectorLeaf() {};
  virtual Node &traverse(std::shared_ptr<BuildContext> context) = 0;
  virtual SelectorLeaf *descendant(SelectorLeaf *right) = 0;
  virtual SelectorLeaf *conjunction(SelectorLeaf *right) = 0;
  virtual SelectorLeaf *disjunction(SelectorLeaf *right) = 0;

  static SelectorLeaf *step(const Key &key);
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
