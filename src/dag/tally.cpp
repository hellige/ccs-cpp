#include "dag/tally.h"

#include "dag/node.h"

namespace ccs {

TallyState *TallyState::clone() const {
  TallyState *next = new TallyState(tally);
  next->firstMatched = firstMatched;
  next->firstMatch = firstMatch;
  next->secondMatched = secondMatched;
  next->secondMatch = secondMatch;
  return next;
}

TallyState *TallyState::activate(const Node &leg,
    const Specificity &spec) const {
  // NB reference equality in the below...
  TallyState *next = clone();
  if (&tally.firstLeg_ == &leg) {
    next->firstMatched = true;
    if (next->firstMatch < spec) next->firstMatch = spec;
  }
  if (&tally.secondLeg_ == &leg) {
    next->secondMatched = true;
    if (next->secondMatch < spec) next->secondMatch = spec;
  }
  return next;
}

Tally::Tally(Node &firstLeg, Node &secondLeg) :
    node_(new Node()),
    firstLeg_(firstLeg),
    secondLeg_(secondLeg) {}

Tally::~Tally() {}

void AndTally::activate(const Node &leg, const Specificity &spec,
    SearchState &searchState) const {
  const TallyState *state = searchState.getTallyState(this)->activate(leg, spec);
  searchState.setTallyState(this, state);
  // seems like this could lead to spurious warnings, but see comment below...
  if (state->fullyMatched())
    node_->activate(state->specificity(), searchState);
}

void OrTally::activate(const Node &, const Specificity &spec,
    SearchState &searchState) const {
  // no state for or-joins, just re-activate node with the current specificity
  // it seems that this may allow spurious warnings, if multiple legs of the
  // disjunction match with same specificity. but this is detected in
  // SearchState, where we keep a *set* of nodes for each specificity, rather
  // than, for example, a *list*.
  node_->activate(spec, searchState);
}


}
