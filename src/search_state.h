#ifndef SEARCH_STATE_H_
#define SEARCH_STATE_H_

#include <map>
#include <memory>
#include <set>

#include "dag/key.h"
#include "dag/specificity.h"

namespace ccs {

class AndTally;
class CcsLogger;
class CcsProperty;
class Node;
class TallyState;

class SearchState {
  std::shared_ptr<SearchState> parent;
  std::map<Specificity, std::set<Node*>> nodes; // TODO should it hold shared pointers or what?
  std::map<const AndTally *, TallyState *> tallyMap;
  CcsLogger &log;
  Key key;
  bool constraintsChanged;

  SearchState(const std::shared_ptr<SearchState> &parent,
      const Key &key, CcsLogger &log) :
        parent(parent),
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


  TallyState *getTallyState(const AndTally *tally);

  void setTallyState(const AndTally *tally, TallyState *state) {
    tallyMap[tally] = state;
  }
};

}

#endif /* SEARCH_STATE_H_ */
