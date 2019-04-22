#include "search_state.h"

#include <algorithm>

#include "ccs/domain.h"
#include "dag/key.h"
#include "dag/node.h"
#include "dag/specificity.h"
#include "dag/tally.h"

namespace ccs {

SearchState::SearchState(const std::shared_ptr<const SearchState> &parent,
    const Key &key) :
      parent(parent),
      log(parent->log),
      key(key),
      logAccesses(parent->logAccesses),
      constraintsChanged(false) {}

SearchState::SearchState(std::shared_ptr<const Node> &root,
    CcsLogger &log,
    bool logAccesses) :
      root(root), log(log), logAccesses(logAccesses) {
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


bool SearchState::extendWith(const SearchState &priorState) {
  constraintsChanged = false;
  for (auto it = priorState.nodes.cbegin(); it != priorState.nodes.cend(); ++it)
        it->first->getChildren(key, it->second, *this);
  return constraintsChanged;
}

const CcsProperty *SearchState::findProperty(const std::string &propertyName)
    const {
  const CcsProperty *prop = doSearch(propertyName);
  if (logAccesses) {
    std::ostringstream msg;
    if (prop) {
      msg << "Found property: " << propertyName
         << " = " << prop->strValue() << "\n";
      msg << "    at " << prop->origin() << " in context: [" << *this << "]";
    } else {
      msg << "Property not found: " << propertyName << "\n";
      msg << "    in context: [" << *this << "]";
    }
    log.info(msg.str());
  }
  return prop;
}

namespace {

void origins(std::ostream &str, const std::vector<const Property *> &values) {
  bool first = true;
  for (auto it = values.cbegin(); it != values.cend(); ++it) {
      if (!first) str << ", ";
      str << (*it)->origin();
      first = false;
  }
}

}

const CcsProperty *SearchState::doSearch(const std::string &propertyName)
    const {
  auto it = properties.find(propertyName);
  if (it == properties.end()) {
    if (parent) return parent->doSearch(propertyName);
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
  std::ostringstream msg;
  msg << "Conflict detected for property '" << propertyName
      << "' in context [" << *this << "]. "
      << "(Conflicting settings at: [";
  origins(msg, values);
  msg << "].) Using most recent value.";
  log.warn(msg.str());
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
