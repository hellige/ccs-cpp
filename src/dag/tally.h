#ifndef CCS_DAG_TALLY_H_
#define CCS_DAG_TALLY_H_

#include <memory>

#include "dag/specificity.h"

namespace ccs {

class AndTally;
class Node;
class SearchState;

class TallyState {
  friend class AndTally;
  const AndTally &tally;
  bool firstMatched;
  bool secondMatched;
  Specificity firstMatch;
  Specificity secondMatch;

  TallyState *activate(const Node &leg, const Specificity &spec) const;
  TallyState *clone() const;

  bool fullyMatched() const { return firstMatched && secondMatched; }
  Specificity specificity() const { return firstMatch + secondMatch; }

public:
  explicit TallyState(const AndTally &tally) :
    tally(tally), firstMatched(false), secondMatched(false) {}
  TallyState(const TallyState &) = delete;
  TallyState &operator=(const TallyState &) = delete;
};

class Tally {
protected:
  std::unique_ptr<Node> node_;
  Node &firstLeg_;
  Node &secondLeg_;

public:
  Tally(Node &firstLeg, Node &secondLeg);
  virtual ~Tally();

  Tally(const Tally &) = delete;
  Tally &operator=(const Tally &) = delete;

  const Node &node() const { return *node_; }
  Node &node() { return *node_; }

  virtual void activate(const Node &leg, const Specificity &spec,
      SearchState &searchState) const = 0;
};

class OrTally : public Tally {
public:
  OrTally(Node &firstLeg, Node &secondLeg) : Tally(firstLeg, secondLeg) {}
  virtual void activate(const Node &leg, const Specificity &spec,
      SearchState &searchState) const;
};

class AndTally : public Tally {
  friend class TallyState;
  TallyState emptyState_;

public:
  AndTally(Node &firstLeg, Node &secondLeg) :
    Tally(firstLeg, secondLeg), emptyState_(*this) {}
  virtual void activate(const Node &leg, const Specificity &spec,
      SearchState &searchState) const;
  const TallyState *emptyState() const { return &emptyState_; }
};

}

#endif /* CCS_DAG_TALLY_H_ */
