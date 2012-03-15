#ifndef CCS_TYPES_H_
#define CCS_TYPES_H_

#include <stdexcept>
#include <string>

namespace ccs {

struct Origin {};

struct CcsProperty {
  virtual ~CcsProperty() {}
  virtual bool exists() const = 0;
  virtual const std::string &value() const = 0;
};

}

#endif /* CCS_TYPES_H_ */
