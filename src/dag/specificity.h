#ifndef CCS_SPECIFICITY_H_
#define CCS_SPECIFICITY_H_

namespace ccs {

struct Specificity {
  int idSelectors;
  int classSelectors;
  int elementNames;
};

} /* namespace ccs */

#endif /* CCS_SPECIFICITY_H_ */
