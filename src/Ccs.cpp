#include "ccs/Ccs.h"

#include "Node.h"

namespace ccs {

CcsDomain::CcsDomain() :
  root_(new Node()), nextPropertyNumber_(0) {}

CcsDomain::~CcsDomain() {
  delete root_;
}

}
