#ifndef CCS_SPECIFICITY_H_
#define CCS_SPECIFICITY_H_

namespace ccs {

struct Specificity {
  unsigned names;
  unsigned values;

  bool operator<(const Specificity &s) const {
    if (values < s.values) return true;
    if (names < s.names) return true;
    return false;
  }
};

}

#endif /* CCS_SPECIFICITY_H_ */
