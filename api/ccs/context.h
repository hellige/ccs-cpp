#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <memory>
#include <vector>

namespace ccs {

class CcsLogger;
class Key;
class Node;
class SearchState;

class CcsContext {
  std::unique_ptr<CcsContext> parent;
  std::shared_ptr<SearchState> searchState;

  friend class CcsDomain;
  CcsContext(Node &root, CcsLogger &log);
  CcsContext(const CcsContext &parent, const Key &key);
  CcsContext(const CcsContext &parent, const std::string &name);
  CcsContext(const CcsContext &parent, const std::string &name,
      const std::vector<std::string> &values);

  std::shared_ptr<SearchState> getSearchState(const Key &key);

public:
  CcsContext(const CcsContext &that);
  CcsContext &operator=(const CcsContext &that);

  // TODO Builder builder();

  CcsContext constrain(const std::string &name) const
    { return CcsContext(*this, name); }

  CcsContext constrain(const std::string &name,
      const std::vector<std::string> &values) const
    { return CcsContext(*this, name, values); }

  std::string getString(const std::string &propertyName) const;
};

}

#endif /* CONTEXT_H_ */
