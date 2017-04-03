#ifndef CCS_GRAPHVIZ_H_
#define CCS_GRAPHVIZ_H_

#include <iosfwd>
#include <map>
#include <string>

namespace ccs {

class Key;
class Node;
class Tally;

class Dumper {
  const Node &node_;

public:
  explicit Dumper(const Node &node) : node_(node) {}
  virtual ~Dumper() {}

  typedef std::map<std::string, std::string> Style;
  Style graphStyle() const;
  Style nodeStyle(const Node &node) const;
  Style edgeStyle(const Key &key) const;
  Style tallyStyle(const Tally &tally) const;
  Style tallyEdgeStyle() const;

  friend std::ostream &operator<<(std::ostream &os, const Dumper &dumper);
};

}

#endif
