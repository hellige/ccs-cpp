#ifndef NODE_H_
#define NODE_H_

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "Key.h"

namespace ccs {

class Property {};

class Node {
  std::unordered_map<Key, std::shared_ptr<Node>> children;
  std::unordered_map<std::string, std::vector<Property>> props;
  std::unordered_map<std::string, std::vector<Property>> localProps;

public:
  Node ();
};

} /* namespace ccs */

#endif /* NODE_H_ */
