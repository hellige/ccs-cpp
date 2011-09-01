#ifndef CCS_CCS_H_
#define CCS_CCS_H_

namespace ccs {

class Node;

class CcsDomain {
  Node *root_;
  unsigned nextPropertyNumber_;

public:
  CcsDomain();
  ~CcsDomain();
  CcsDomain(const CcsDomain &) = delete;
  CcsDomain &operator=(const CcsDomain &) = delete;
};

}


#endif /* CCS_CCS_H_ */
