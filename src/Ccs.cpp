#include "ccs/Ccs.h"

#include "Node.h"

namespace ccs {

CcsDomain::CcsDomain() {
  root_ = new Node();
}

CcsDomain::~CcsDomain() {
  delete root_;
}

}
