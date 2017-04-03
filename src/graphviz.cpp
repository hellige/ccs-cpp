#include "graphviz.h"

#include "dag/node.h"

#include <iostream>
#include <unordered_set>

namespace ccs {

typedef Dumper::Style Style;

namespace {

struct Streamer {
  const Style &style;
  bool root;
  Streamer(const Style &style, bool root=false) : style(style), root(root) {}
  friend std::ostream &operator<<(std::ostream &os, const Streamer &ss) {
    if (!ss.style.empty()) {
      bool first = true;
      if (!ss.root) os << '[';
      for (auto it = ss.style.cbegin(); it != ss.style.cend(); ++it) {
        if (!first && !ss.root) os << ", ";
        first = false;
        os << it->first;
        os << "=\"";
        for (auto cit = it->second.cbegin(); cit != it->second.cend(); ++cit) {
          switch (*cit) {
            default: os << *cit; break;
            case '"': os << "\\\""; break;
            case '\n': os << "\\n"; break;
            case '\t': os << "\\t"; break;
          }
        }
        os << '"';
        if (ss.root) os << ";";
      }
      if (!ss.root) os << ']';
    }
    return os;
  }
};

template <typename T>
struct NodeName {
  const T &t;
  NodeName(const T &t) : t(t) {}
  friend std::ostream &operator<<(std::ostream &os, const NodeName &nn) {
    return os << '"' << &nn.t << '"';
  }

};

template <typename T>
NodeName<T> nn(const T &t) { return NodeName<T>(t); }

void visit(const Dumper &dumper, std::ostream &os,
    std::unordered_set<const Node *> &visited, const Node &node) {
  if (visited.count(&node)) return;
  visited.insert(&node);
  os << nn(node) << ' ' << Streamer(dumper.nodeStyle(node)) << ";\n";
  const auto &cs = node.allChildren();
  for (auto it = cs.cbegin(); it != cs.cend(); ++it) {
    const auto &child = *it->second;
    os << nn(node) << "->" << nn(child) << ' '
        << Streamer(dumper.edgeStyle(it->first)) << ";\n";
    visit(dumper, os, visited, child);
  }

  const auto &ts = node.tallies<AndTally>();
  for (auto it = ts.cbegin(); it != ts.cend(); ++it) {
    const auto &tally = **it;
    os << nn(node) << "->" << nn(tally) << ' '
        << Streamer(dumper.tallyEdgeStyle()) << ";\n";
    if (!visited.count(&tally.node())) {
      os << nn(tally) << ' ' << Streamer(dumper.tallyStyle(tally)) << ";\n";
      os << nn(tally) << "->" << nn(tally.node()) << ' '
          << Streamer(dumper.tallyEdgeStyle()) << ";\n";
      visit(dumper, os, visited, tally.node());
    }
  }
  const auto &ts2 = node.tallies<OrTally>();
  for (auto it = ts2.cbegin(); it != ts2.cend(); ++it) {
    const auto &tally = **it;
    os << nn(node) << "->" << nn(tally) << ' '
        << Streamer(dumper.tallyEdgeStyle()) << ";\n";
    if (!visited.count(&tally.node())) {
      os << nn(tally) << ' ' << Streamer(dumper.tallyStyle(tally)) << ";\n";
      os << nn(tally) << "->" << nn(tally.node()) << ' '
          << Streamer(dumper.tallyEdgeStyle()) << ";\n";
      visit(dumper, os, visited, tally.node());
    }
  }

}

}

Style Dumper::graphStyle() const {
  return Style();
}

Style Dumper::tallyStyle(const Tally &tally) const {
  Style style;
  style["nodesep"] = "2.0";
  style["color"] = "blue";
  style["label"] = "";

  if (dynamic_cast<const AndTally *>(&tally)) {
    style["shape"] = "triangle";
  } else {
    style["shape"] = "invtriangle";
  }

  return style;
}

Style Dumper::nodeStyle(const Node &node) const {
  Style style;
  style["shape"] = "box";
  style["style"] = "rounded";
  style["nodesep"] = "2.0";

  std::ostringstream str;
  const auto &props = node.properties();
  for (auto it = props.cbegin(); it != props.cend(); ++it) {
    if (it->second.override()) str << "@override ";
    str << it->first << " = " << it->second.strValue() << "\n";
  }
  if (!node.allConstraints().empty())
    str << "@constrain " << node.allConstraints();
  style["label"] = str.str();
  style["fontsize"] = "9";
  return style;
}

Style Dumper::tallyEdgeStyle() const {
  Style style;
  style["style"] = "dashed";
  return style;
}

Style Dumper::edgeStyle(const Key &key) const {
  Style style;
  std::ostringstream str;
  str << key;
  style["label"] = str.str();
  style["fontsize"] = "9";
  return style;
}

std::ostream &operator<<(std::ostream &os, const Dumper &dagDumper) {
  std::unordered_set<const Node *> visited;

  os << "digraph {";
  os << "edge [arrowhead = none]";
  os << Streamer(dagDumper.graphStyle(), true);

  visit(dagDumper, os, visited, dagDumper.node_);

  os << "}";

  return os;
}

}
