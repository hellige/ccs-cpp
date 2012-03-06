#ifndef CCS_CCS_H_
#define CCS_CCS_H_

#include <memory>

namespace ccs {

class DagBuilder;
class ImportResolver;

class CcsDomain {
  std::unique_ptr<DagBuilder> dag_;

public:
  CcsDomain();
  ~CcsDomain();
  CcsDomain(const CcsDomain &) = delete;
  CcsDomain &operator=(const CcsDomain &) = delete;

  CcsDomain &loadCcsStream(std::istream &stream, const std::string &fileName,
      ImportResolver &importResolver);
};

}


#endif /* CCS_CCS_H_ */
