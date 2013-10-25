#ifndef CCS_DAG_KEY_H_
#define CCS_DAG_KEY_H_

#include <algorithm>
#include <map>
#include <ostream>
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

  const Specificity &specificity() const { return specificity_; }

  bool empty() const { return values_.empty(); }

  bool operator<(const Key &that) const
    { return values_ < that.values_; }

  bool addName(const std::string &name) {
    if (values_.find(name) == values_.end()) {
      values_[name];
      specificity_.names++;
      return true;
    }
    return false;
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

  bool addAll(const Key &key) {
    bool changed = false;
    for (auto it = key.values_.cbegin(); it != key.values_.cend(); ++it) {
      changed |= addName(it->first);
      for (auto it2 = it->second.cbegin(); it2 != it->second.cend(); ++it2)
        changed |= addValue(it->first, *it2);
    }
    return changed;
  }

  /*
   * treating this as a pattern, see whether it matches the given specific
   * key. this is asymmetric because the given key can have unmatched (extra)
   * names/values, but the current object must fully match the key. wildcards
   * also match on the current object, but not on the given key.
   * returns true if this object, as a pattern, matches the given key.
   */
  bool matches(Key k) const {
    for (auto it = values_.cbegin(); it != values_.cend(); ++it) {
      auto valSet = k.values_.find(it->first);
      if (valSet == k.values_.cend()) return false;
      if (!std::includes(valSet->second.cbegin(), valSet->second.cend(),
          it->second.cbegin(), it->second.cend()))
        return false;
    }
    return true;
  }

  friend std::ostream &operator<<(std::ostream &, const Key &);
};

std::ostream &operator<<(std::ostream &out, const Key &key);

}

#endif /* CCS_DAG_KEY_H_ */
