#ifndef CCS_DAG_PROPERTY_H_
#define CCS_DAG_PROPERTY_H_

#include <cstdint>

#include <boost/variant.hpp>

#include "ccs/types.h"

namespace ccs {

struct Value {
  typedef boost::variant <bool, int64_t, double, std::string> V;
  V val_;
  std::string strVal_;
  std::string name_;

  Value() : val_(false) {}
  void setString(const std::string &val) { val_ = val; str(); }
  void setInt(int64_t val) { val_ = val; str(); }
  void setBool(bool val) { val_ = val; str(); }
  void setDouble(double val) { val_ = val; str(); }
  void setName(const std::string &name) { name_ = name; }

  const std::string &asString() const { return strVal_; }
  int asInt() const;
  double asDouble() const;
  bool asBool() const;
  void str();
};

class Property : public CcsProperty {
  const Value value_;
  Origin origin_;
  unsigned propertyNumber_;
  bool override_;

public:
  Property(const Value &value, const Origin &origin,
      unsigned propertyNumber, bool override) :
        value_(value), origin_(origin), propertyNumber_(propertyNumber),
        override_(override) {}

  virtual bool exists() const { return true; }
  virtual Origin origin() const { return origin_; }
  virtual const std::string &strValue() const { return value_.asString(); }
  virtual int intValue() const { return value_.asInt(); }
  virtual double doubleValue() const { return value_.asDouble(); }
  virtual bool boolValue() const { return value_.asBool(); }
  bool override() const { return override_; }
  unsigned propertyNumber() const { return propertyNumber_; }
};

}

#endif /* CCS_DAG_PROPERTY_H_ */
