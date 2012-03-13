#ifndef DOMAIN_H_
#define DOMAIN_H_

#include <memory>
#include <istream>
#include <string>

#include "ccs/context.h"

namespace ccs {

class DagBuilder;
class ImportResolver;

class CcsLogger {
public:
  static CcsLogger &StdErr;
  virtual ~CcsLogger() {}
  virtual void warn(const std::string &msg) = 0;
};

class CcsDomain {
  std::unique_ptr<DagBuilder> dag;
  CcsLogger &log;

public:
  CcsDomain();
  CcsDomain(CcsLogger &log);
  ~CcsDomain();
  CcsDomain(const CcsDomain &) = delete;
  CcsDomain &operator=(const CcsDomain &) = delete;

  CcsDomain &loadCcsStream(std::istream &stream, const std::string &fileName,
      ImportResolver &importResolver);

  CcsContext build();
};

}


#endif /* DOMAIN_H_ */
