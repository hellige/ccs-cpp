#ifndef CCS_KEY_H_
#define CCS_KEY_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "dag/specificity.h"

namespace ccs {

class Key {
  std::map<std::string, std::set<std::string>> values_;
  Specificity specificity_;

public:
  Key() {}

  Key(const std::string &name, const std::vector<std::string> &values) {
    addName(name);
    for (auto it = values.begin(); it != values.end(); ++it)
      addValue(name, *it);
  }

  Key(const Key &) = default;
  Key &operator=(const Key &) = default;
  ~Key() = default;

  bool operator<(const Key &) const { return false; } // TODO

  void addName(const std::string &name) {
    if (values_.find(name) == values_.end()) {
      values_[name];
      specificity_.names++;
    }
  }

  bool addValue(const std::string &name, const std::string &value) {
    bool changed = false;
    if (values_.find(name) == values_.end()) {
      changed = true;
      specificity_.names++;
    }
    if (values_[name].insert(value).second) {
      changed = true;
      specificity_.values++;
    }
    return changed;
  }
};

}

#endif /* CCS_KEY_H_ */
