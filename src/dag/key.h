#ifndef CCS_KEY_H_
#define CCS_KEY_H_

#include <map>
#include <set>
#include <string>

#include "dag/specificity.h"

namespace ccs {

class Key {
  std::map<std::string, std::set<std::string>> values_;
  Specificity specificity_;

public:
  bool operator<(const Key &) const { return false; } // TODO

  void addName(const std::string &name) {
    if (values_.find(name) == values_.end()) {
      values_[name];
      specificity_.elementNames++;
    }
  }

  bool addValue(const std::string &name, const std::string &value) {
    bool changed = false;
    if (values_.find(name) == values_.end()) {
      changed = true;
      specificity_.elementNames++;
    }
    if (values_[name].insert(value).second) {
      changed = true;
      specificity_.classSelectors++;
    }
    return changed;
  }
};

} /* namespace ccs */

#endif /* CCS_KEY_H_ */
