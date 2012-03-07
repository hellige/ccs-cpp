#include "ccs/domain.h"

#include "ccs/context.h"
#include "dag/dag_builder.h"
#include "parser/loader.h"

namespace ccs {

namespace {
CcsLogger StdErrLogger;
}

CcsLogger &CcsLogger::StdErr = StdErrLogger;

CcsDomain::CcsDomain() :
  dag(new DagBuilder()), log(CcsLogger::StdErr) {}

CcsDomain::CcsDomain(CcsLogger &log) :
  dag(new DagBuilder()), log(log) {}

CcsDomain::~CcsDomain() {}

CcsDomain &CcsDomain::loadCcsStream(std::istream &stream,
    const std::string &fileName, ImportResolver &importResolver) {
  Loader loader;
  loader.loadCcsStream(stream, fileName, *dag, importResolver);
  return *this;
}

CcsContext CcsDomain::build() {
  return CcsContext(dag->root(), log);
}

}
