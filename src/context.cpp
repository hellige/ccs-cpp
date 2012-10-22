#include "ccs/context.h"

#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

#include "search_state.h"
#include "ccs/types.h"

namespace ccs {

struct MissingProp : public CcsProperty {
  virtual bool exists() const { return false; }
  virtual Origin origin() const
    { throw std::runtime_error("called origin() on MissingProp"); }
  virtual const std::string &strValue() const
    { throw std::runtime_error("called strValue() on MissingProp"); }
  virtual int intValue() const
    { throw std::runtime_error("called intValue() on MissingProp"); }
  virtual double doubleValue() const
    { throw std::runtime_error("called doubleValue() on MissingProp"); }
  virtual bool boolValue() const
    { throw std::runtime_error("called boolValue() on MissingProp"); }
};

namespace { MissingProp Missing; }

CcsContext::CcsContext(std::shared_ptr<const Node> root, CcsLogger &log,
    bool logAccesses)
  : searchState(new SearchState(root, log, logAccesses)) {}

CcsContext::CcsContext(const CcsContext &parent, const Key &key)
  : searchState(SearchState::newChild(parent.searchState, key)) {}

CcsContext::CcsContext(const CcsContext &parent, const std::string &name)
  : searchState(SearchState::newChild(parent.searchState, Key(name, {}))) {}

CcsContext::CcsContext(const CcsContext &parent, const std::string &name,
    const std::vector<std::string> &values)
  : searchState(SearchState::newChild(parent.searchState, Key(name, values))) {}

CcsContext::Builder CcsContext::builder() const { return Builder(*this); }

const CcsProperty &CcsContext::getProperty(const std::string &propertyName)
    const {
  const CcsProperty *prop = searchState->findProperty(propertyName);
  if (prop) return *prop;
  return Missing;
}

const std::string &CcsContext::getString(const std::string &propertyName)
    const {
  const CcsProperty &prop(getProperty(propertyName));
  if (!prop.exists()) throw no_such_property(propertyName, *this);
  return prop.strValue();
}

const std::string &CcsContext::getString(const std::string &propertyName,
    const std::string &defaultVal) const {
  const CcsProperty *prop(searchState->findProperty(propertyName, defaultVal));
  if (!prop) return defaultVal;
  return prop->strValue();
}

bool CcsContext::getInto(std::string &dest, const std::string &propertyName)
    const {
  const CcsProperty &prop(getProperty(propertyName));
  if (!prop.exists()) return false;
  dest = prop.strValue();
  return true;
}

int CcsContext::getInt(const std::string &propertyName) const {
  const CcsProperty &prop(getProperty(propertyName));
  if (!prop.exists()) throw no_such_property(propertyName, *this);
  return prop.intValue();
}

int CcsContext::getInt(const std::string &propertyName, int defaultVal) const {
  const CcsProperty *prop(searchState->findProperty(propertyName, defaultVal));
  if (!prop) return defaultVal;
  return prop->intValue();
}

bool CcsContext::getInto(int &dest, const std::string &propertyName)
    const {
  const CcsProperty &prop(getProperty(propertyName));
  if (!prop.exists()) return false;
  dest = prop.intValue();
  return true;
}

double CcsContext::getDouble(const std::string &propertyName) const {
  const CcsProperty &prop(getProperty(propertyName));
  if (!prop.exists()) throw no_such_property(propertyName, *this);
  return prop.doubleValue();
}

double CcsContext::getDouble(const std::string &propertyName,
    double defaultVal) const {
  const CcsProperty *prop(searchState->findProperty(propertyName, defaultVal));
  if (!prop) return defaultVal;
  return prop->doubleValue();
}

bool CcsContext::getInto(double &dest, const std::string &propertyName)
    const {
  const CcsProperty &prop(getProperty(propertyName));
  if (!prop.exists()) return false;
  dest = prop.doubleValue();
  return true;
}

bool CcsContext::getBool(const std::string &propertyName) const {
  const CcsProperty &prop(getProperty(propertyName));
  if (!prop.exists()) throw no_such_property(propertyName, *this);
  return prop.boolValue();
}

bool CcsContext::getBool(const std::string &propertyName, bool defaultVal)
    const {
  const CcsProperty *prop(searchState->findProperty(propertyName, defaultVal));
  if (!prop) return defaultVal;
  return prop->boolValue();
}

bool CcsContext::getInto(bool &dest, const std::string &propertyName)
    const {
  const CcsProperty &prop(getProperty(propertyName));
  if (!prop.exists()) return false;
  dest = prop.boolValue();
  return true;
}

std::ostream &operator<<(std::ostream &str, const CcsContext ctx) {
  return str << *ctx.searchState;
}


struct CcsContext::Builder::Impl {
  CcsContext context;
  Key key;
  Impl(const CcsContext &context) : context(context) {}
};

CcsContext::Builder::Builder(const CcsContext &context) :
    impl(new Impl(context)) {}
CcsContext::Builder::Builder(const Builder &that) :
    impl(new Impl(*that.impl)) {}
CcsContext::Builder &CcsContext::Builder::operator=(
    const CcsContext::Builder &that) {
  impl.reset(new Impl(*that.impl));
  return *this;
}

CcsContext::Builder::~Builder() {}

CcsContext CcsContext::Builder::build() const
  { return CcsContext(impl->context, impl->key); }

CcsContext::Builder &CcsContext::Builder::add(const std::string &name,
    const std::vector<std::string> &values) {
  impl->key.addName(name);
  for (auto it = values.cbegin(); it != values.cend(); ++it)
    impl->key.addValue(name, *it);
  return *this;
}

no_such_property::no_such_property(const std::string &name, CcsContext context)
  : context(context) {
  std::ostringstream str;
  str << "no such property: '" << name << "' (in context: " << context << ")";
  msg = str.str();
}

}
