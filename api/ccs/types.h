#ifndef CCS_TYPES_H_
#define CCS_TYPES_H_

#include <ostream>
#include <string>

namespace ccs {

struct Origin {
  std::string fileName;
  unsigned line;

  Origin() : fileName("<unknown>"), line(0) {}
  Origin(const std::string &fileName, unsigned line) :
    fileName(fileName), line(line) {}
};

static inline std::ostream &operator<<(std::ostream &str, const Origin &origin) {
  str << origin.fileName << ':' << origin.line;
  return str;
}

struct CcsProperty {
  virtual ~CcsProperty() {}
  virtual bool exists() const = 0;
  virtual Origin origin() const = 0;
  virtual const std::string &strValue() const = 0;
  virtual int intValue() const = 0;
  virtual double doubleValue() const = 0;
  virtual bool boolValue() const = 0;
};

}

#endif /* CCS_TYPES_H_ */
