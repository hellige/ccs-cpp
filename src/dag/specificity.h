#pragma once

namespace ccs {

struct Specificity {
  unsigned names;
  unsigned values;

  Specificity() : names(0), values(0) {}
  Specificity(unsigned names, unsigned values) : names(names), values(values) {}

  bool operator<(const Specificity &s) const {
    if (values < s.values) return true;
    if (values == s.values && names < s.names) return true;
    return false;
  }

  Specificity operator+(const Specificity &that) const {
    return Specificity(names + that.names, values + that.values);
  }
};

}
