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

  TallyState *activate(Node &leg, const Specificity &spec);
  TallyState *clone();

  bool fullyMatched() { return firstMatched && secondMatched; }
  Specificity specificity() { return firstMatch + secondMatch; }

public:
  TallyState(const AndTally &tally) :
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

  Node &node() { return *node_; }

  virtual void activate(Node &leg, const Specificity &spec,
      SearchState &searchState) = 0;
};

class OrTally : public Tally {
public:
  OrTally(Node &firstLeg, Node &secondLeg) : Tally(firstLeg, secondLeg) {}
  virtual void activate(Node &leg, const Specificity &spec,
      SearchState &searchState);
};

class AndTally : public Tally {
  friend class TallyState;
public:
  AndTally(Node &firstLeg, Node &secondLeg) : Tally(firstLeg, secondLeg) {}
  virtual void activate(Node &leg, const Specificity &spec,
      SearchState &searchState);
};

}

#endif /* CCS_DAG_TALLY_H_ */
