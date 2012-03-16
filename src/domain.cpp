#include "ccs/domain.h"

#include <iostream>

#include "ccs/context.h"
#include "dag/dag_builder.h"
#include "parser/loader.h"

namespace ccs {

namespace {

struct StdErrLogger : public CcsLogger {
  virtual void warn(const std::string &msg)
    { std::cerr << "WARN: " << msg << std::endl; }
  virtual void error(const std::string &msg)
    { std::cerr << "ERROR: " << msg << std::endl; }
};

StdErrLogger StdErrLogger;

struct NoImportResolver : public ImportResolver {
  virtual bool resolve(const std::string &location,
      std::function<bool(std::istream &)> load) {
    return false;
  }
};

NoImportResolver NoImportResolver;

}

CcsLogger &CcsLogger::StdErr = StdErrLogger;
ImportResolver &ImportResolver::None = NoImportResolver;

CcsDomain::CcsDomain() :
  dag(new DagBuilder()), log(CcsLogger::StdErr) {}

CcsDomain::CcsDomain(CcsLogger &log) :
  dag(new DagBuilder()), log(log) {}

CcsDomain::~CcsDomain() {}

CcsDomain &CcsDomain::loadCcsStream(std::istream &stream,
    const std::string &fileName, ImportResolver &importResolver) {
  Loader loader(log);
  loader.loadCcsStream(stream, fileName, *dag, importResolver);
  return *this;
}

CcsContext CcsDomain::build() {
  return CcsContext(dag->root(), log);
}

}
