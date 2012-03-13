#include "search_state.h"

#include <algorithm>

#include "ccs/domain.h"
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
  for (auto it = priorState.nodes.crbegin(); it != priorState.nodes.crend();
        ++it)
      for (auto it2 = it->second.cbegin(); it2 != it->second.cend(); ++it2)
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
  for (auto it = nodes.crbegin(); it != nodes.crend(); ++it) {
    std::vector<Property *> values;
    for (auto it2 = it->second.cbegin(); it2 != it->second.cend(); ++it2) {
      auto valsAtNode = (*it2)->getProperty(propertyName, locals);
      for (auto it3 = valsAtNode.cbegin(); it3 != valsAtNode.cend(); ++it3)
        if ((*it3)->override() == override)
          values.push_back(*it3);
    }
    if (values.size() == 1)
      return values[0];
    else if (values.size() > 1) {
      std::sort(values.begin(), values.end(),
          [](const Property *l, const Property *r) {
        return l->propertyNumber < r->propertyNumber;
      });
      log.warn("Conflict detected for property: " + propertyName
         // + " in context [" + ccsContext.toString() + "]. " TODO log
         // + "Conflicting settings at: [" + origins(values) + "]"
          + ". Using most recent value.");
      return values.back();
    }
  }
  return NULL;
}

}
