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
  // TODO this is NOT a copy constructor, it's a chaining constructor. ugly.
  TallyMap(const TallyMap &tallyMap) {}
};

class SearchState {
  std::map<Specificity, std::set<Node*>> nodes; // TODO reverse order? should it hold shared pointers or what?
  TallyMap tallyMap;
  CcsContext ccsContext;
  CcsLogger &log;
  Key key;
  bool constraintsChanged;

  SearchState(TallyMap &tallyMap, CcsContext &ccsContext, const Key &key,
      CcsLogger &log) :
      tallyMap(tallyMap),
      ccsContext(ccsContext),
      log(log),
      key(key) {}

public:
  SearchState(Node &root, CcsContext &ccsContext, CcsLogger &log) :
      ccsContext(ccsContext), log(log) {
    std::set<Node *> s = { &root };
    nodes[Specificity()] = s;
  }

  SearchState(const SearchState &) = delete;
  SearchState &operator=(const SearchState &) = delete;

  std::shared_ptr<SearchState> newChild(CcsContext &ccsContext,
      const Key &key) {
    return std::shared_ptr<SearchState>(new SearchState(tallyMap, ccsContext,
        key, log));
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
    searchState(new SearchState(root, *this, log)) {}

CcsContext::CcsContext(const CcsContext &parent, const Key &key) :
    parent(new CcsContext(parent)),
    searchState(getSearchState(key)) {}

CcsContext::CcsContext(const CcsContext &parent, const std::string &name) :
    parent(new CcsContext(parent)),
    searchState(getSearchState(Key(name, {}))) {}

CcsContext::CcsContext(const CcsContext &parent, const std::string &name,
    const std::vector<std::string> &values) :
    parent(new CcsContext(parent)),
    searchState(getSearchState(Key(name, values))) {}

CcsContext::CcsContext(const CcsContext &that) :
    parent(that.parent ? new CcsContext(*that.parent) : NULL),
    searchState(that.searchState) {}

CcsContext &CcsContext::operator=(const CcsContext &that) {
  searchState = that.searchState;
  if (that.parent)
    parent.reset(new CcsContext(*that.parent));
  return *this;
}

std::string CcsContext::getString(const std::string &propertyName) const {
  return "TODO"; // TODO
}

std::shared_ptr<SearchState> CcsContext::getSearchState(const Key &key) {
    std::shared_ptr<SearchState> tmp =
        parent->searchState->newChild(*this, key);

    bool constraintsChanged;
    do {
        constraintsChanged = false;
        CcsContext *p = parent.get();
        while (p) {
            constraintsChanged |= tmp->extendWith(*p->searchState);
            p = p->parent.get();
        }
    } while (constraintsChanged);

    return tmp;
}


}
