#ifndef CCS_DOMAIN_H_
#define CCS_DOMAIN_H_

#include <memory>
#include <istream>
#include <string>

#include "ccs/context.h"

namespace ccs {

class DagBuilder;

class CcsLogger {
public:
  static CcsLogger &StdErr;
  virtual ~CcsLogger() {}
  virtual void warn(const std::string &msg) = 0;
  virtual void error(const std::string &msg) = 0;
};

class ImportResolver {
public:
  static ImportResolver &None;
  virtual ~ImportResolver() {}
  virtual bool resolve(const std::string &location,
      std::function<bool(std::istream &)> load) = 0;
};

class CcsDomain {
  std::unique_ptr<DagBuilder> dag;
  CcsLogger &log;

public:
  CcsDomain();
  explicit CcsDomain(CcsLogger &log);
  ~CcsDomain();
  CcsDomain(const CcsDomain &) = delete;
  CcsDomain &operator=(const CcsDomain &) = delete;

  CcsDomain &loadCcsStream(std::istream &stream, const std::string &fileName,
      ImportResolver &importResolver);

  CcsContext build();
};

}


#endif /* CCS_DOMAIN_H_ */
