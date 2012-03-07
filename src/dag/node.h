#ifndef CCS_NODE_H_
#define CCS_NODE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ccs/types.h"
#include "dag/key.h"

namespace ccs {

class SearchState;

struct Value {};
struct Property {
  Value value;
  Origin origin;
  unsigned propertyNumber;
};

class Node {
  std::map<Key, std::shared_ptr<Node>> children;
  std::multimap<std::string, Property> props;
  std::multimap<std::string, Property> localProps;

public:
  Node () {}

  Node &addChild(const Key &key) {
    return *(*children.insert(std::make_pair(key, std::make_shared<Node>()))
        .first).second;
  }

  void getChildren(const Key &key, const Specificity &spec,
      SearchState &searchState) {
    // TODO
//    for (Map.Entry<Key, Node> entry : children.entrySet())
//      if (entry.getKey().matches(key))
//        entry.getValue().activate(spec.add(entry.getKey().getSpecificity()), searchState);
  }
};

} /* namespace ccs */

#endif /* CCS_NODE_H_ */
