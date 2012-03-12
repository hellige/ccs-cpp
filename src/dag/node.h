#ifndef CCS_NODE_H_
#define CCS_NODE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "search_state.h"
#include "ccs/types.h"
#include "dag/key.h"

namespace ccs {

class Property : public CcsProperty {
  std::string value_;
  Origin origin;
  unsigned propertyNumber;
  bool override_;

public:
  Property(const std::string &value, const Origin &origin,
      unsigned propertyNumber, bool override) :
        value_(value), origin(origin), propertyNumber(propertyNumber),
        override_(override) {}

  virtual bool exists() const { return true; }
  virtual const std::string &value() const { return value_; }
  bool override() const { return override_; }
};

class Node {
  std::map<Key, std::shared_ptr<Node>> children;
  std::multimap<std::string, Property> props;
  std::multimap<std::string, Property> localProps;
  Key constraints;

public:
  Node() {}
  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;

  Node &addChild(const Key &key) {
    return *(*children.insert(std::make_pair(key, std::make_shared<Node>()))
        .first).second;
  }

  void getChildren(const Key &key, const Specificity &spec,
      SearchState &searchState) {
    for (auto it = children.begin(); it != children.end(); ++it) {
      if (it->first.matches(key))
        it->second->activate(spec + it->first.specificity(), searchState);
    }
  }

  std::vector<Property *> getProperty(const std::string &name, bool locals) {
    std::pair<std::multimap<std::string, Property>::iterator,
      std::multimap<std::string, Property>::iterator> range(props.end(),
          props.end());
    if (locals) range = localProps.equal_range(name);
    if (range.first == range.second) range = props.equal_range(name);
    std::vector<Property *> result;
    for (; range.first != range.second; ++range.first)
      result.push_back(&range.first->second);
    return result;
  }

  void activate(const Specificity &spec, SearchState &searchState) {
    searchState.add(spec, this);
    searchState.constrain(constraints);
  // TODO for (Tally tally : this.tallies) tally.activate(this, spec, searchState);
  }

  void addConstraint(const Key &key) {
    constraints.addAll(key);
  }

  void addProperty(const std::string &name, const Property &value,
      bool isLocal) {
    std::multimap<std::string, Property> &theProps = isLocal ?
        localProps : props;
    theProps.insert(std::pair<std::string, Property>(name, value));
  }
};

}

#endif /* CCS_NODE_H_ */
