#include "search_state.h"

#include <algorithm>
#include <ostream>
#include <sstream>

#include "ccs/domain.h"
#include "dag/key.h"
#include "dag/node.h"
#include "dag/specificity.h"
#include "dag/tally.h"

namespace ccs {

SearchState::SearchState(const std::shared_ptr<const SearchState> &parent,
    const Key &key) :
      parent(parent),
      tracer(parent->tracer),
      key(key),
      constraintsChanged(false) {}

SearchState::SearchState(std::shared_ptr<const Node> &root) :
      root(root), tracer(root->tracer()) {
  constraintsChanged = false;
  root->activate(Specificity(), *this);
  while (constraintsChanged) {
    constraintsChanged = false;
    root->getChildren(key, Specificity(), *this);
  }
}

SearchState::~SearchState() {
  for (auto it = tallyMap.begin(); it != tallyMap.end(); ++it)
    delete it->second;
}

std::shared_ptr<SearchState> SearchState::newChild(
    const std::shared_ptr<const SearchState> &parent, const Key &key) {
  std::shared_ptr<SearchState> searchState(new SearchState(parent, key));

  bool constraintsChanged;
  do {
    constraintsChanged = false;
    const SearchState *p = parent.get();
    while (p) {
      constraintsChanged |= searchState->extendWith(*p);
      p = p->parent.get();
    }
  } while (constraintsChanged);

  return searchState;
}

void SearchState::logRuleDag(std::ostream &os) const {
  if (parent)
    parent->logRuleDag(os);
  else
    os << Dumper(*root);
}

bool SearchState::extendWith(const SearchState &priorState) {
  constraintsChanged = false;
  for (auto it = priorState.nodes.cbegin(); it != priorState.nodes.cend(); ++it)
        it->first->getChildren(key, it->second, *this);
  return constraintsChanged;
}

const CcsProperty *SearchState::findProperty(const CcsContext &context,
    const std::string &propertyName) const {
  const CcsProperty *prop = doSearch(context, propertyName);
  if (prop) {
    tracer.onPropertyFound(context, propertyName, *prop);
  } else {
    tracer.onPropertyNotFound(context, propertyName);
  }
  return prop;
}

const CcsProperty *SearchState::doSearch(const CcsContext &context,
    const std::string &propertyName) const {
  auto it = properties.find(propertyName);
  if (it == properties.end()) {
    if (parent) return parent->doSearch(context, propertyName);
    return NULL;
  }

  if (it->second.values.size() == 1)
    return *it->second.values.begin();

  // it->second.values.size() > 1
  std::vector<const Property *> values(it->second.values.begin(),
      it->second.values.end());
  std::sort(values.begin(), values.end(),
      [](const Property *l, const Property *r) {
    return l->propertyNumber() < r->propertyNumber();
  });

  std::vector<const CcsProperty *> baseValues(values.begin(), values.end());
  tracer.onConflict(context, propertyName, baseValues);
  return values.back();
}

const TallyState *SearchState::getTallyState(const AndTally *tally) const {
  auto it = tallyMap.find(tally);
  if (it != tallyMap.end()) return it->second;
  if (parent) return parent->getTallyState(tally);
  return tally->emptyState();
}

void SearchState::setTallyState(const AndTally *tally,
    const TallyState *state) {
  const TallyState *&loc = tallyMap[tally];
  if (loc) delete loc;
  loc = state;
}

void SearchState::append(std::ostream &out, bool isPrefix) const {
  if (parent) {
    parent->append(out, isPrefix || !key.empty());
  } else if (key.empty() && !isPrefix) {
    out << "<root>";
    return;
  }

  if (!key.empty()) {
    out << key;
    if (isPrefix) out << " > ";
  }
}

std::ostream &operator<<(std::ostream &out, const SearchState &state) {
  state.append(out, false);
  return out;
}

}
