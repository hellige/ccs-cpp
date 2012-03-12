#ifndef TYPES_H_
#define TYPES_H_

#include <stdexcept>
#include <string>

namespace ccs {

struct Origin {};
struct Value {};

struct CcsProperty {
  virtual ~CcsProperty() {}
  virtual bool exists() const = 0;
  virtual const std::string &value() const = 0;
};

}

#endif /* TYPES_H_ */
