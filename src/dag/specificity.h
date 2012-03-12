#ifndef CCS_SPECIFICITY_H_
#define CCS_SPECIFICITY_H_

namespace ccs {

struct Specificity {
  unsigned names;
  unsigned values;

  Specificity() : names(0), values(0) {}
  Specificity(unsigned names, unsigned values) : names(names), values(values) {}

  bool operator<(const Specificity &s) const {
    if (values < s.values) return true;
    if (names < s.names) return true;
    return false;
  }

  Specificity operator+(const Specificity &that) const {
    return Specificity(names + that.names, values + that.values);
  }
};

}

#endif /* CCS_SPECIFICITY_H_ */
