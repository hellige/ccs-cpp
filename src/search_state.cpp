#include "search_state.h"

#include <algorithm>
#include <sstream>

#include "ccs/domain.h"
#include "dag/key.h"
#include "dag/node.h"
#include "dag/specificity.h"
#include "dag/tally.h"

namespace ccs {

SearchState::SearchState(const std::shared_ptr<const SearchState> &parent,
    const Key &key, CcsLogger &log) :
      parent(parent),
      log(log),
      key(key) {}

SearchState::SearchState(std::shared_ptr<const Node> &root,
    const std::shared_ptr<const SearchState> &parent,
    CcsLogger &log) :
      root(root), parent(parent), log(log) {
  std::set<const Node *> s = { root.get() };
  nodes[Specificity()] = s;
}

SearchState::~SearchState() {
  for (auto it = tallyMap.begin(); it != tallyMap.end(); ++it)
    delete it->second;
}

std::shared_ptr<SearchState> SearchState::newChild(
    const std::shared_ptr<const SearchState> &parent, const Key &key) {
  std::shared_ptr<SearchState> searchState(new SearchState(parent, key,
      parent->log));

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
  for (auto it = priorState.nodes.crbegin(); it != priorState.nodes.crend();
        ++it)
      for (auto it2 = it->second.cbegin(); it2 != it->second.cend(); ++it2)
          (*it2)->getChildren(key, it->first, *this);
  return constraintsChanged;
}

const CcsProperty *SearchState::findProperty(const std::string &propertyName)
    const {
  const CcsProperty *prop = findProperty(propertyName, true, true);
  if (!prop) prop = findProperty(propertyName, true, false);
  return prop;
}


const CcsProperty *SearchState::findProperty(const std::string &propertyName,
    bool locals, bool override) const {
  // first, look in nodes newly matched by this pattern...
  const CcsProperty *prop = doSearch(propertyName, locals, override);
  if (prop) return prop;

  // if not, then inherit...
  if (parent) return parent->findProperty(propertyName, false, override);

  return NULL;
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

const CcsProperty *SearchState::doSearch(const std::string &propertyName,
    bool locals, bool override) const {
  for (auto it = nodes.crbegin(); it != nodes.crend(); ++it) {
    std::vector<const Property *> values;
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
  }
  return NULL;
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

std::ostream &operator<<(std::ostream &out, const SearchState &state) {
  if (state.parent) {
    if (state.parent->parent) {
      out << *state.parent << " > " << state.key;
    } else {
      out << state.key;
    }
  } else {
    out << "<root>";
  }
  return out;
}

}
