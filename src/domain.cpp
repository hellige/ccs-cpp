#include "ccs/domain.h"

#include <iostream>

#include "graphviz.h"
#include "ccs/context.h"
#include "dag/dag_builder.h"
#include "parser/loader.h"

namespace ccs {

namespace {

struct StdErrLogger : public CcsLogger {
  virtual void info(const std::string &msg)
    { std::cerr << "INFO: " << msg << std::endl; }
  virtual void warn(const std::string &msg)
    { std::cerr << "WARN: " << msg << std::endl; }
  virtual void error(const std::string &msg)
    { std::cerr << "ERROR: " << msg << std::endl; }
};

StdErrLogger StdErrLogger;

struct NoImportResolver : public ImportResolver {
  virtual bool resolve(const std::string &,
      std::function<bool(std::istream &)>) {
    return false;
  }
};

NoImportResolver NoImportResolver;

}

CcsLogger &CcsLogger::StdErr = StdErrLogger;
ImportResolver &ImportResolver::None = NoImportResolver;

CcsDomain::CcsDomain(bool logAccesses) :
  dag(new DagBuilder()), log(CcsLogger::StdErr), logAccesses(logAccesses) {}

CcsDomain::CcsDomain(CcsLogger &log, bool logAccesses) :
  dag(new DagBuilder()), log(log), logAccesses(logAccesses) {}

CcsDomain::~CcsDomain() {}

CcsDomain &CcsDomain::loadCcsStream(std::istream &stream,
    const std::string &fileName, ImportResolver &importResolver) {
  Loader loader(log);
  loader.loadCcsStream(stream, fileName, *dag, importResolver);
  return *this;
}

RuleBuilder CcsDomain::ruleBuilder() {
  return RuleBuilder(*dag);
}

CcsContext CcsDomain::build() {
  return CcsContext(dag->root(), log, logAccesses);
}

void CcsDomain::logRuleDag(std::ostream &os) const {
  os << Dumper(*dag->root());
}

}
