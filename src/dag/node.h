#ifndef CCS_DAG_NODE_H_
#define CCS_DAG_NODE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "search_state.h"
#include "ccs/types.h"
#include "dag/key.h"
#include "dag/property.h"
#include "dag/tally.h"

namespace ccs {

class Dumper;

template<typename T>
struct identity { typedef T type; };

class Node {
  friend class Dumper;
  std::map<Key, std::shared_ptr<Node>> children;
  std::multimap<std::string, Property> props;
  std::set<std::shared_ptr<AndTally>> andTallies_;
  std::set<std::shared_ptr<OrTally>> orTallies_;
  Key constraints;

public:
  Node() {}
  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;

  const std::map<Key, std::shared_ptr<Node>> &allChildren() const
      { return children; }

  template<typename T>
  const std::set<std::shared_ptr<T>> &tallies() const {
    return tallies(identity<T>());
  }

  const std::set<std::shared_ptr<AndTally>> &tallies(identity<AndTally>) const
    { return andTallies_; }
  const std::set<std::shared_ptr<OrTally>> &tallies(identity<OrTally>) const
    { return orTallies_; }

  const std::multimap<std::string, Property> &properties() const
      { return props; }
  const Key &allConstraints() const { return constraints; }

  void addTally(std::shared_ptr<AndTally> tally) { andTallies_.insert(tally); }
  void addTally(std::shared_ptr<OrTally> tally) { orTallies_.insert(tally); }

  Node &addChild(const Key &key) {
    return *(*children.insert(std::make_pair(key, std::make_shared<Node>()))
        .first).second;
  }

  void getChildren(const Key &key, const Specificity &spec,
      SearchState &searchState) const {
    for (auto it = children.cbegin(); it != children.cend(); ++it) {
      if (it->first.matches(key))
        it->second->activate(spec + it->first.specificity(), searchState);
    }
  }

  std::vector<const Property *> getProperty(const std::string &name) const {
    std::pair<std::multimap<std::string, Property>::const_iterator,
      std::multimap<std::string, Property>::const_iterator> range(
          props.equal_range(name));
    std::vector<const Property *> result;
    for (; range.first != range.second; ++range.first)
      result.push_back(&range.first->second);
    return result;
  }

  void activate(const Specificity &spec, SearchState &searchState) const {
    searchState.constrain(constraints);
    if (searchState.add(spec, this)) {
      for (auto it = props.begin(); it != props.end(); ++it)
        searchState.cacheProperty(it->first, spec, &it->second);
      for (auto it = andTallies_.begin(); it != andTallies_.end(); ++it)
        (*it)->activate(*this, spec, searchState);
      for (auto it = orTallies_.begin(); it != orTallies_.end(); ++it)
        (*it)->activate(*this, spec, searchState);
    }
  }

  void addConstraint(const Key &key) {
    constraints.addAll(key);
  }

  void addProperty(const std::string &name, const Property &value) {
    props.insert(std::pair<std::string, Property>(name, value));
  }
};


}

#endif /* CCS_DAG_NODE_H_ */
