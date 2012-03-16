#ifndef CCS_TYPES_H_
#define CCS_TYPES_H_

#include <string>

namespace ccs {

struct Origin {};

struct CcsProperty {
  virtual ~CcsProperty() {}
  virtual bool exists() const = 0;
  virtual const std::string &strValue() const = 0;
  virtual int intValue() const = 0;
  virtual double doubleValue() const = 0;
  virtual bool boolValue() const = 0;
};

}

#endif /* CCS_TYPES_H_ */
