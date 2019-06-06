#include "dag/property.h"

#include <cstdlib>
#include <sstream>

#include "ccs/context.h"

namespace ccs {

template <typename S, typename V>
S Value::accept(V &&visitor) const {
  switch (which_) {
    case String: return visitor(rawStringVal_);
    case Int: return visitor(rawPrimVal_.intVal);
    case Double: return visitor(rawPrimVal_.doubleVal);
    case Bool: return visitor(rawPrimVal_.boolVal);
  }
  std::ostringstream msg;
  msg << "Bad enum value " << which_;
  throw std::runtime_error(msg.str());
};

namespace {

template <typename S>
struct Caster {
  const Value &val;
  Caster(const Value &val) : val(val) {}
  template <typename T>
  S operator()(const T &v) const {
    (void)v;
    S s;
    if (!CcsContext::coerceString(val.asString(), s))
      throw bad_coercion(val.name(), val.asString());
    return s;
  }
  S operator()(const S &v) const { return v; }
};

struct ToString {
  std::string operator()(bool v) const { return v ? "true" : "false"; }
  std::string operator()(const StringVal &v) const { return v.str(); }
  template <typename T>
  std::string operator()(const T &v) const {
    std::ostringstream str;
    str << v;
    return str.str();
  }
};

}

void StringElem::interpolateInto(std::ostream &os) const {
  if (isInterpolant) {
    const char *val = getenv(value.c_str());
    if (val) os << val;
    return;
  }

  os << value;
}

std::string StringVal::str() const {
  std::ostringstream str;
  for (auto it = elements_.begin(); it != elements_.end(); ++it)
    it->interpolateInto(str);
  return str.str();
}

int Value::asInt() const
  { return accept<int>(Caster<int>(*this)); }
double Value::asDouble() const
  { return accept<double>(Caster<double>(*this)); }
bool Value::asBool() const
  { return accept<bool>(Caster<bool>(*this)); }
void Value::str()
  { strVal_ = accept<std::string>(ToString()); }

}
