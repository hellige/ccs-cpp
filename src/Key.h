#ifndef CCS_KEY_H_
#define CCS_KEY_H_

#include <map>
#include <set>
#include <string>

#include "Specificity.h"

namespace ccs {

class Key {
  std::string element_;
  std::map<std::string, std::string> attributes_;
  std::set<std::string> classes_;
  Specificity specificity_;

  std::string id_;
  bool root_;
  bool directChild_;

public:
  Key();

  bool operator<(const Key &) const { return false; }
};

} /* namespace ccs */

#endif /* CCS_KEY_H_ */
