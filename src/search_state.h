#ifndef CCS_SEARCH_STATE_H_
#define CCS_SEARCH_STATE_H_

#include <map>
#include <memory>
#include <ostream>
#include <set>

#include "dag/key.h"
#include "dag/specificity.h"

namespace ccs {

class AndTally;
class CcsLogger;
class CcsProperty;
class Node;
class TallyState;

class SearchState {
  // we need to be sure to retain a reference to the root of the dag. the
  // simplest way is to just make everything in 'nodes' shared_ptrs, but that
  // seems awfully heavy-handed. instead we'll just retain a direct reference
  // to the root in the root search state. the parent links are shared, so
  // this is sufficient.
  std::shared_ptr<const Node> root;
  std::shared_ptr<const SearchState> parent;
  std::map<Specificity, std::set<const Node *>> nodes;
  // the TallyStates here should rightly be unique_ptrs, but gcc 4.5 can't
  // support that in a map. bummer.
  std::map<const AndTally *, const TallyState *> tallyMap;
  CcsLogger &log;
  Key key;
  bool constraintsChanged;

  SearchState(const std::shared_ptr<const SearchState> &parent,
      const Key &key, CcsLogger &log);

public:
  SearchState(std::shared_ptr<const Node> &root,
      const std::shared_ptr<const SearchState> &parent,
      CcsLogger &log);
  SearchState(const SearchState &) = delete;
  SearchState &operator=(const SearchState &) = delete;
  ~SearchState();

  static std::shared_ptr<SearchState> newChild(
      const std::shared_ptr<const SearchState> &parent, const Key &key);

  bool extendWith(const SearchState &priorState);

  const CcsProperty *findProperty(const std::string &propertyName) const;

  void add(Specificity spec, Node *node)
    { nodes[spec].insert(node); }

  void constrain(const Key &constraints) {
    constraintsChanged |= key.addAll(constraints);
  }

  const TallyState *getTallyState(const AndTally *tally) const;
  void setTallyState(const AndTally *tally, const TallyState *state);

private:
  const CcsProperty *findProperty(const std::string &propertyName, bool locals,
      bool override) const;
  const CcsProperty *doSearch(const std::string &propertyName, bool locals,
      bool override) const;

  friend std::ostream &operator<<(std::ostream &, const SearchState &);
};

}

#endif /* CCS_SEARCH_STATE_H_ */
