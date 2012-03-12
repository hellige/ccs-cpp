#include "search_state.h"

#include "dag/key.h"
#include "dag/node.h"
#include "dag/specificity.h"

namespace ccs {

SearchState::SearchState(Node &root, const std::shared_ptr<SearchState> &parent,
    CcsLogger &log) :
      parent(parent), log(log) {
  std::set<Node *> s = { &root };
  nodes[Specificity()] = s;
}

std::shared_ptr<SearchState> SearchState::newChild(
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


bool SearchState::extendWith(const SearchState &priorState) {
  constraintsChanged = false;
  for (auto it = priorState.nodes.begin(); it != priorState.nodes.end(); ++it)
      for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
          (*it2)->getChildren(key, it->first, *this);
  return constraintsChanged;
}

const CcsProperty *SearchState::findProperty(const std::string &propertyName,
    bool locals, bool override) {
  // first, look in nodes newly matched by this pattern...
  const CcsProperty *prop = doSearch(propertyName, locals, override);
  if (prop) return prop;

  // if not, then inherit...
  if (parent) return parent->findProperty(propertyName, false, override);

  return NULL;
}

const CcsProperty *SearchState::doSearch(const std::string &propertyName,
    bool locals, bool override) {
  for (auto it = nodes.cbegin(); it != nodes.cend(); ++it) {
    std::vector<CcsProperty *> values;
    for (auto it2 = it->second.cbegin(); it2 != it->second.cend(); ++it2) {
      auto valsAtNode = (*it2)->getProperty(propertyName, locals);
      for (auto it3 = valsAtNode.begin(); it3 != valsAtNode.end(); ++it3)
        if ((*it3)->override() == override)
          values.push_back(*it3);
    }
    if (values.size() == 1)
      return values[0];
    else if (values.size() > 1) {
      // TODO
//      Collections.sort(values, PROP_COMPARATOR);
//      log.warn("Conflict detected for property: " + propertyName
//          + " in context [" + ccsContext.toString() + "]. "
//          + "Conflicting settings at: [" + origins(values) + "]. "
//          + "Using most recent value.");
      return values.back();
    }
  }
  return NULL;
}

}
