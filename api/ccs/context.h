#ifndef CCS_CONTEXT_H_
#define CCS_CONTEXT_H_

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
  CcsContext(std::shared_ptr<const Node> root, CcsLogger &log);
  CcsContext(const CcsContext &parent, const Key &key);
  CcsContext(const CcsContext &parent, const std::string &name);
  CcsContext(const CcsContext &parent, const std::string &name,
      const std::vector<std::string> &values);

public:
  CcsContext(const CcsContext &that) = default;
  CcsContext &operator=(const CcsContext &that) = default;
  ~CcsContext() = default;

  class Builder;

  Builder builder() const;

  CcsContext constrain(const std::string &name) const
    { return CcsContext(*this, name); }

  CcsContext constrain(const std::string &name,
      const std::vector<std::string> &values) const
    { return CcsContext(*this, name, values); }

  const CcsProperty &getProperty(const std::string &propertyName) const
    { return findProperty(propertyName, true); }

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

private:
  const CcsProperty &findProperty(const std::string &propertyName,
      bool locals) const;
};

template <typename T>
T CcsContext::get(const std::string &propertyName) const {
  std::istringstream str(getString(propertyName));
  T t;
  str >> t;
  return t;
}

template <typename T>
T CcsContext::get(const std::string &propertyName, const T &defaultVal) const {
  std::string str;
  if (!getInto(str, propertyName)) return defaultVal;
  std::istringstream stream(str);
  T t;
  stream >> t;
  return t;
}

template <typename T>
bool CcsContext::getInto(T &dest, const std::string &propertyName) const {
  std::string str;
  if (!getInto(str, propertyName)) return false;
  std::istringstream stream(str);
  stream >> dest;
  return true;
}


class CcsContext::Builder {
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  Builder(const CcsContext &context);
  Builder(const Builder &);
  Builder &operator=(const Builder &);
  ~Builder();

  CcsContext build() const;

  Builder &add(const std::string &name)
    { return add(name, {}); }
  Builder &add(const std::string &name,
      const std::vector<std::string> &values);
};

struct no_such_property : public std::exception {
  std::string msg;
  CcsContext context;
  no_such_property(const std::string &name, CcsContext context) :
    msg("no such property: " + name), context(context) {}
  virtual ~no_such_property() throw() {}
  virtual const char *what() const throw()
    { return msg.c_str(); }
};

struct wrong_type : public std::exception {
  std::string msg;
  wrong_type(const std::string &name) :
    msg("property has wrong type: " + name) {}
  virtual ~wrong_type() throw() {}
  virtual const char *what() const throw()
    { return msg.c_str(); }
};

}

#endif /* CCS_CONTEXT_H_ */
