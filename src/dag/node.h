#ifndef CCS_NODE_H_
#define CCS_NODE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "dag/key.h"

namespace ccs {

struct Value {};
struct Origin {};
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
  Node ();
};

} /* namespace ccs */

#endif /* CCS_NODE_H_ */
