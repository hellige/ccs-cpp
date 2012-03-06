#ifndef CCS_PARSER_AST_H_
#define CCS_PARSER_AST_H_

#include <boost/lexical_cast.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <memory>
#include <string>

#include "Key.h"
#include "Node.h"

namespace ccs { namespace ast {

// TODO desctructors or shared_ptrs...


struct Import {
  std::string location_;
};


struct Value {
  std::string strVal_; // TODO generalize?

  Value &operator=(const Value &that) {
    strVal_ = that.strVal_;
    return *this;
  }

  template <typename T>
  Value &operator=(const T &t) {
    strVal_ = boost::lexical_cast<std::string>(t);
    return *this;
  }
};

struct PropDef {
  std::string name_;
  Value value_;
  Origin origin_; // TODO track!
  bool local_;
  bool override_;
};


struct Constraint {
  Key key_;
};

struct Nested;

typedef boost::variant<
    boost::blank, // TODO
    Import,
    PropDef,
    Constraint,
    boost::recursive_wrapper<Nested>>
  AstRule;

struct SelectorLeaf;

struct SelectorBranch {
  static SelectorBranch *descendant(SelectorLeaf *first);
  static SelectorBranch *conjunction(SelectorLeaf *first);
  static SelectorBranch *disjunction(SelectorLeaf *first);
};

struct SelectorLeaf {
  virtual ~SelectorLeaf() {};
  virtual SelectorLeaf *descendant(SelectorLeaf *right) = 0;
  virtual SelectorLeaf *conjunction(SelectorLeaf *right) = 0;
  virtual SelectorLeaf *disjunction(SelectorLeaf *right) = 0;

  static SelectorLeaf *step(const Key &key);
};

struct Nested {
  SelectorBranch *selector_;
  std::vector<AstRule> rules_;
};

}}


#endif /* CCS_PARSER_AST_H_ */
