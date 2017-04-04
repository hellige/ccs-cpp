#ifndef CCS_CONTEXT_H_
#define CCS_CONTEXT_H_

#include <iosfwd>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ccs/types.h"

namespace ccs {

class CcsLogger;
class CcsProperty;
class Key;
class Node;
class SearchState;

class CcsContext {
  std::shared_ptr<SearchState> searchState;

  friend class CcsDomain;
  CcsContext(std::shared_ptr<const Node> root, CcsLogger &log,
      bool logAccesses);
  CcsContext(const CcsContext &parent, const Key &key);
  CcsContext(const CcsContext &parent, const std::string &name);
  CcsContext(const CcsContext &parent, const std::string &name,
      const std::vector<std::string> &values);

public:
  CcsContext(const CcsContext &) = default;
  CcsContext &operator=(const CcsContext &) = default;
  ~CcsContext() = default;

  class Builder;

  void logRuleDag(std::ostream &os) const;

  Builder builder() const;

  CcsContext constrain(const std::string &name) const
    { return CcsContext(*this, name); }

  CcsContext constrain(const std::string &name,
      const std::vector<std::string> &values) const
    { return CcsContext(*this, name, values); }

  const CcsProperty &getProperty(const std::string &propertyName) const;

  const std::string &getString(const std::string &propertyName) const;
  const std::string &getString(const std::string &propertyName,
      const std::string &defaultVal) const;
  bool getInto(std::string &dest, const std::string &propertyName) const;

  int getInt(const std::string &propertyName) const;
  int getInt(const std::string &propertyName, int defaultVal) const;
  bool getInto(int &dest, const std::string &propertyName) const;

  double getDouble(const std::string &propertyName) const;
  double getDouble(const std::string &propertyName, double defaultVal) const;
  bool getInto(double &dest, const std::string &propertyName) const;

  bool getBool(const std::string &propertyName) const;
  bool getBool(const std::string &propertyName, bool defaultVal) const;
  bool getInto(bool &dest, const std::string &propertyName) const;

  // these depend on operator>> for T
  template <typename T>
  T get(const std::string &propertyName) const;
  template <typename T>
  T get(const std::string &propertyName, const T &defaultVal) const;
  template <typename T>
  bool getInto(T &dest, const std::string &propertyName) const;

  friend std::ostream &operator<<(std::ostream &, const CcsContext);

  template<class T>
  static bool coerceString(const std::string &s, T &dest);

private:
  static bool checkEmpty(std::istream &stream);
};


struct no_such_property : public virtual std::exception {
  std::string msg;
  CcsContext context;
  no_such_property(const std::string &name, CcsContext context);
  virtual ~no_such_property() throw() {}
  virtual const char *what() const throw()
    { return msg.c_str(); }
};

struct bad_coercion : public virtual std::exception {
  std::string msg;
  bad_coercion(const std::string &name, const std::string &value) :
    msg(std::string("property cannot be coerced to requested type: ") + name
        + " with value " + value) {}
  virtual ~bad_coercion() throw() {}
  virtual const char *what() const throw()
    { return msg.c_str(); }
};


template<class T>
bool CcsContext::coerceString(const std::string &s, T &dest) {
  std::istringstream ist(s);
  ist >> dest;
  return checkEmpty(ist);
}

template<>
inline bool CcsContext::coerceString<std::string>(const std::string &s,
    std::string &dest) {
  dest = s;
  return true;
}

template<>
inline bool CcsContext::coerceString<bool>(const std::string &s,
    bool &dest) {
  // convert from a string to a bool: value must be exactly "true" or "false",
  // for maximum consistency with ccs boolean literals.
  if (s == std::string("true")) {
    dest = true;
    return true;
  }
  if (s == std::string("false")) {
    dest = false;
    return true;
  }
  return false;
}

template <typename T>
T CcsContext::get(const std::string &propertyName) const {
  auto &val = getString(propertyName);
  T t;
  if (!coerceString(val, t)) throw bad_coercion(propertyName, val);
  return t;
}

template <typename T>
T CcsContext::get(const std::string &propertyName, const T &defaultVal) const {
  std::string str;
  if (!getInto(str, propertyName)) return defaultVal;
  T t;
  if (!coerceString(str, t)) return defaultVal;
  return t;
}

template <typename T>
bool CcsContext::getInto(T &dest, const std::string &propertyName) const {
  std::string str;
  if (!getInto(str, propertyName)) return false;
  return coerceString(str, dest);
}


class CcsContext::Builder {
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  explicit Builder(const CcsContext &context);
  Builder(const Builder &);
  Builder &operator=(const Builder &);
  ~Builder();

  CcsContext build() const;

  Builder &add(const std::string &name)
    { return add(name, {}); }
  Builder &add(const std::string &name,
      const std::vector<std::string> &values);
};

}

#endif /* CCS_CONTEXT_H_ */
