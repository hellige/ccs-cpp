#ifndef CCS_SEARCH_STATE_H_
#define CCS_SEARCH_STATE_H_

#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <set>

#include "ccs/domain.h"
#include "dag/key.h"
#include "dag/property.h"
#include "dag/specificity.h"
#include "graphviz.h"

namespace ccs {

class AndTally;
class CcsProperty;
class Node;
class TallyState;

struct PropertySetting {
    Specificity spec;
    bool override;
    std::set<const Property *> values;

    PropertySetting(Specificity spec, const Property *value)
    : spec(spec),
      override(value->override()) {
      values.insert(value);
    }

    bool better(const PropertySetting &that) const {
      if (override && !that.override) return true;
      if (!override && that.override) return false;
      return that.spec < spec;
    }
};

class SearchState {
  // we need to be sure to retain a reference to the root of the dag. the
  // simplest way is to just make everything in 'nodes' shared_ptrs, but that
  // seems awfully heavy-handed. instead we'll just retain a direct reference
  // to the root in the root search state. the parent links are shared, so
  // this is sufficient.
  std::shared_ptr<const Node> root;
  std::shared_ptr<const SearchState> parent;
  std::map<const Node *, Specificity> nodes;
  // the TallyStates here should rightly be unique_ptrs, but gcc 4.5 can't
  // support that in a map. bummer.
  std::map<const AndTally *, const TallyState *> tallyMap;
  // cache of properties newly set in this context
  std::map<std::string, PropertySetting> properties;
  CcsLogger &log;
  Key key;
  bool logAccesses;
  bool constraintsChanged;

  SearchState(const std::shared_ptr<const SearchState> &parent, const Key &key);

public:
  SearchState(std::shared_ptr<const Node> &root,
      CcsLogger &log, bool logAccesses);
  SearchState(const SearchState &) = delete;
  SearchState &operator=(const SearchState &) = delete;
  ~SearchState();

  static std::shared_ptr<SearchState> newChild(
      const std::shared_ptr<const SearchState> &parent, const Key &key);

  void logRuleDag(std::ostream &os) const {
    if (parent)
      parent->logRuleDag(os);
    else
      os << Dumper(*root);
  }

  bool extendWith(const SearchState &priorState);

  const CcsProperty *findProperty(const std::string &propertyName) const;

  bool add(Specificity spec, const Node *node) {
    auto pr = nodes.insert(std::make_pair(node, spec));

    if (pr.second) return true;

    auto &it = pr.first;
    if (it->second < spec) {
      it->second = spec;
      return true;
    }

    return false;
  }

  void constrain(const Key &constraints)
    { constraintsChanged |= key.addAll(constraints); }

  void cacheProperty(const std::string &propertyName,
      Specificity spec, const Property *property) {
    auto it = properties.find(propertyName);

    PropertySetting newSetting(spec, property);

    if (it == properties.end()) {
      // we don't have a local setting for this yet.
      auto parentProperty = parent ? parent->checkCache(propertyName) : NULL;
      if (parentProperty) {
        if (parentProperty->better(newSetting))
          // parent copy found, parent property better, leave local cache empty.
          return;

        // copy parent property into local cache. this is done solely to
        // support conflict detection.
        it = properties.insert(std::make_pair(propertyName, *parentProperty))
            .first;
      }
    }

    if (it == properties.end()) {
      properties.insert(std::make_pair(propertyName, newSetting));
    } else if (newSetting.better(it->second)) {
      // new property better than local cache. replace.
      it->second = newSetting;
    } else if (it->second.better(newSetting)) {
      // ignore
    } else {
      // new property has same specificity/override as existing... append.
      it->second.values.insert(property);
    }
  }

  const PropertySetting *checkCache(const std::string &propertyName) const {
    auto it = properties.find(propertyName);
    if (it != properties.end()) return &it->second;
    if (!parent) return NULL;
    return parent->checkCache(propertyName);
  }

  template <typename T>
  const CcsProperty *findProperty(const std::string &propertyName,
      T defaultVal) {
    const CcsProperty *prop = doSearch(propertyName);
    if (logAccesses) {
      std::ostringstream msg;
      if (prop) {
        msg << "Found property: " << propertyName << " = "
          << prop->strValue() << "\n";
      } else {
        msg << "Property not found: " << propertyName << ". Default = "
          << defaultVal << "\n";
      }
      msg << "    in context: [" << *this << "]";
      log.info(msg.str());
    }
    return prop;
  }

  const TallyState *getTallyState(const AndTally *tally) const;
  void setTallyState(const AndTally *tally, const TallyState *state);

private:
  const CcsProperty *doSearch(const std::string &propertyName) const;

  friend std::ostream &operator<<(std::ostream &, const SearchState &);
  void append(std::ostream &out, bool isPrefix) const;
};

}

#endif /* CCS_SEARCH_STATE_H_ */
