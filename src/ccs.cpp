#include "ccs/ccs.h"

#include "dag/dag_builder.h"
#include "parser/loader.h"

namespace ccs {

CcsDomain::CcsDomain() :
  dag_(new DagBuilder()) {}

CcsDomain::~CcsDomain() {}

CcsDomain &CcsDomain::loadCcsStream(std::istream &stream,
    const std::string &fileName, ImportResolver &importResolver) {
  Loader loader;
  loader.loadCcsStream(stream, fileName, *dag_, importResolver);
  return *this;
}


}
