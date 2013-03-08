#include "dag/property.h"

#include <cstdlib>
#include <sstream>

#include <boost/variant/static_visitor.hpp>

#include "ccs/context.h"

namespace ccs {

namespace {

template <typename S>
struct Caster : public boost::static_visitor<S> {
  const Value &val;
  Caster(const Value &val) : val(val) {}
  template <typename T>
  S operator()(const T &v) const {
    S s;
    if (!CcsContext::coerceString(val.strVal_, s))
      throw bad_coercion(val.name_, val.strVal_);
    return s;
  }
  S operator()(const S &v) const { return v; }
};

struct ToString : public boost::static_visitor<std::string> {
  std::string operator()(bool v) const { return v ? "true" : "false"; }
  std::string operator()(const StringVal &v) const { return v.str(); }
  template <typename T>
  std::string operator()(const T &v) const {
    std::ostringstream str;
    str << v;
    return str.str();
  }
};

struct Interpolate : public boost::static_visitor<std::string> {
  std::string operator()(const std::string &str) const { return str; }
  std::string operator()(const Interpolant &interp) const {
    const char *val = getenv(interp.name.c_str());
    if (val) return std::string(val);
    return "";
  }
};

}

std::string StringVal::str() const {
  std::ostringstream str;
  for (auto it = elements_.begin(); it != elements_.end(); ++it)
    str << boost::apply_visitor(Interpolate(), *it);
  return str.str();
}


int Value::asInt() const
  { return boost::apply_visitor(Caster<int64_t>(*this), val_); }
double Value::asDouble() const
  { return boost::apply_visitor(Caster<double>(*this), val_); }
bool Value::asBool() const
  { return boost::apply_visitor(Caster<bool>(*this), val_); }
void Value::str()
  { strVal_ = boost::apply_visitor(ToString(), val_); }

}
