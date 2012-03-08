#include "ccs/context.h"

#include <map>
#include <set>

#include "ccs/domain.h"
#include "dag/key.h"
#include "dag/node.h"
#include "dag/specificity.h"

namespace ccs {

struct TallyMap {
  TallyMap() {}
  // TODO this is NOT a copy constructor, it's a chaining constructor. ugly and dangerous.
  TallyMap(const TallyMap &tallyMap) {}
};

class SearchState {
  std::shared_ptr<SearchState> parent;
  std::map<Specificity, std::set<Node*>> nodes; // TODO reverse order? should it hold shared pointers or what?
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
      CcsLogger &log) :
        parent(parent), log(log) {
    std::set<Node *> s = { &root };
    nodes[Specificity()] = s;
  }

  SearchState(const SearchState &) = delete;
  SearchState &operator=(const SearchState &) = delete;

  static std::shared_ptr<SearchState> newChild(
      const std::shared_ptr<SearchState> &parent, const Key &key) {
    std::shared_ptr<SearchState> searchState(new SearchState(parent->tallyMap,
        parent, key, parent->log));

    bool constraintsChanged;
    do {
      constraintsChanged = false;
      SearchState *p = parent.get();
      while (p) {
        constraintsChanged |= searchState->extendWith(*p);
        p = p->parent.get();
      }
    } while (constraintsChanged);

    return searchState;
  }

  bool extendWith(const SearchState &priorState) {
    constraintsChanged = false;
    for (auto it = priorState.nodes.begin(); it != priorState.nodes.end(); ++it)
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
            (*it2)->getChildren(key, it->first, *this);
    return constraintsChanged;
  }
};

CcsContext::CcsContext(Node &root, CcsLogger &log) :
    searchState(new SearchState(root, NULL, log)) {}

CcsContext::CcsContext(const CcsContext &parent, const Key &key) :
    searchState(SearchState::newChild(parent.searchState, key)) {}

CcsContext::CcsContext(const CcsContext &parent, const std::string &name) :
    searchState(SearchState::newChild(parent.searchState, Key(name, {}))) {}

CcsContext::CcsContext(const CcsContext &parent, const std::string &name,
    const std::vector<std::string> &values) :
    searchState(SearchState::newChild(parent.searchState,
        Key(name, values))) {}

std::string CcsContext::getString(const std::string &propertyName) const {
  return "TODO"; // TODO
}

}
