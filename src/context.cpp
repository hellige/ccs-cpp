#include "ccs/context.h"

#include <map>
#include <set>
#include <stdexcept>

#include "search_state.h"
#include "ccs/types.h"

namespace ccs {

struct MissingProp : public CcsProperty {
  virtual bool exists() const { return false; }
  virtual const std::string &value() const
    { throw std::runtime_error("called value() on MissingProp"); }
};

namespace { MissingProp Missing; }

CcsContext::CcsContext(Node &root, CcsLogger &log) :
    searchState(new SearchState(root, std::shared_ptr<SearchState>(), log)) {}

CcsContext::CcsContext(const CcsContext &parent, const Key &key) :
    searchState(SearchState::newChild(parent.searchState, key)) {}

CcsContext::CcsContext(const CcsContext &parent, const std::string &name) :
    searchState(SearchState::newChild(parent.searchState, Key(name, {}))) {}

CcsContext::CcsContext(const CcsContext &parent, const std::string &name,
    const std::vector<std::string> &values) :
    searchState(SearchState::newChild(parent.searchState,
        Key(name, values))) {}

const CcsProperty &CcsContext::findProperty(const std::string &propertyName,
    bool locals) const {
  const CcsProperty *prop = searchState->findProperty(propertyName, locals,
      true);
  if (!prop) prop = searchState->findProperty(propertyName, locals, false);
  if (prop) return *prop;
  return Missing;
}

const std::string &CcsContext::getString(const std::string &propertyName) const {
  const CcsProperty &prop(getProperty(propertyName));
  if (!prop.exists()) throw no_such_property(propertyName, *this);
  return prop.value();
}

}
