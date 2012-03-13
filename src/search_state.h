#ifndef SEARCH_STATE_H_
#define SEARCH_STATE_H_

#include <map>
#include <memory>
#include <set>

#include "dag/key.h"
#include "dag/specificity.h"

namespace ccs {

class CcsLogger;
class CcsProperty;
class Node;

struct TallyMap {
  TallyMap() {}
  // TODO this is NOT a copy constructor, it's a chaining constructor. ugly and dangerous.
  TallyMap(const TallyMap &tallyMap) {}
};

class SearchState {
  std::shared_ptr<SearchState> parent;
  std::map<Specificity, std::set<Node*>> nodes; // TODO should it hold shared pointers or what?
  TallyMap tallyMap;
  CcsLogger &log;
  Key key;
  bool constraintsChanged;

  SearchState(TallyMap &tallyMap, const std::shared_ptr<SearchState> &parent,
      const Key &key, CcsLogger &log) :
        parent(parent),
        tallyMap(tallyMap),
        log(log),
        key(key) {}

public:
  SearchState(Node &root, const std::shared_ptr<SearchState> &parent,
      CcsLogger &log);
  SearchState(const SearchState &) = delete;
  SearchState &operator=(const SearchState &) = delete;

  static std::shared_ptr<SearchState> newChild(
      const std::shared_ptr<SearchState> &parent, const Key &key);

  bool extendWith(const SearchState &priorState);

  const CcsProperty *findProperty(const std::string &propertyName, bool locals,
      bool override);
  const CcsProperty *doSearch(const std::string &propertyName, bool locals,
      bool override);

  void add(Specificity spec, Node *node)
    { nodes[spec].insert(node); }

  void constrain(const Key &constraints) {
    constraintsChanged |= key.addAll(constraints);
  }
};

}

#endif /* SEARCH_STATE_H_ */
