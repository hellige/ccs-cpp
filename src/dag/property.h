#pragma once

#include <cstdint>
#include <iosfwd>
#include <vector>

#include "ccs/types.h"

namespace ccs {

struct StringElem {
  std::string value;
  bool isInterpolant;

  explicit StringElem(std::string value, bool isInterpolant = false) :
    value(std::move(value)), isInterpolant(isInterpolant) {}

  void interpolateInto(std::ostream &os) const;
};

struct StringVal {
  std::vector<StringElem> elements_;

  StringVal() {}
  explicit StringVal(const std::string &str) {
    elements_.emplace_back(str);
  }
  std::string str() const;

  bool interpolation() const {
    if (elements_.size() > 1) return true;
    if (elements_.front().isInterpolant) return true;
    return false;
  }
};

class Value {
  enum Which { String, Int, Double, Bool };
  Which which_;
  StringVal rawStringVal_;
  union {
    int64_t intVal;
    double doubleVal;
    bool boolVal;
  } rawPrimVal_;
  std::string strVal_;
  std::string name_;

public:
  Value() : which_(String), rawPrimVal_{0} {}
  void setString(const StringVal &val)
    { rawStringVal_ = val; which_ = String; str(); }
  void setInt(int64_t val)
    { rawPrimVal_.intVal = val; which_ = Int; str(); }
  void setBool(bool val)
    { rawPrimVal_.boolVal = val; which_ = Bool; str(); }
  void setDouble(double val)
    { rawPrimVal_.doubleVal = val; which_ = Double; str(); }
  void setName(const std::string &name) { name_ = name; }

  const std::string &name() const { return name_; }
  const std::string &asString() const { return strVal_; }
  int asInt() const;
  double asDouble() const;
  bool asBool() const;

private:
  void str();
  template <typename S, typename V>
  S accept(V &&visitor) const;
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
