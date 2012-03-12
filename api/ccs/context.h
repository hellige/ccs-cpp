#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <memory>
#include <string>
#include <vector>

namespace ccs {

class CcsLogger;
class CcsProperty;
class Key;
class Node;
class SearchState;

class CcsContext {
  std::shared_ptr<SearchState> searchState;

  friend class CcsDomain;
  CcsContext(Node &root, CcsLogger &log);
  CcsContext(const CcsContext &parent, const Key &key);
  CcsContext(const CcsContext &parent, const std::string &name);
  CcsContext(const CcsContext &parent, const std::string &name,
      const std::vector<std::string> &values);

public:
  CcsContext(const CcsContext &that) = default;
  CcsContext &operator=(const CcsContext &that) = default;
  ~CcsContext() = default;

  // TODO Builder builder();

  CcsContext constrain(const std::string &name) const
    { return CcsContext(*this, name); }

  CcsContext constrain(const std::string &name,
      const std::vector<std::string> &values) const
    { return CcsContext(*this, name, values); }

  const CcsProperty &getProperty(const std::string &propertyName) const
    { return findProperty(propertyName, true); }

  const std::string &getString(const std::string &propertyName) const;

private:
  const CcsProperty &findProperty(const std::string &propertyName,
      bool locals) const;
};

struct no_such_property : public std::exception {
  std::string name;
  CcsContext context;

  no_such_property(const std::string &name, CcsContext context) throw() :
    name(name), context(context) {}
  virtual ~no_such_property() throw() {}

  virtual const char *what() const throw()
    { return "no such CCS property"; }
};

}

#endif /* CONTEXT_H_ */
