#include "dag/key.h"

namespace ccs {

std::ostream &operator<<(std::ostream &out, const Key &key) {
  bool first = true;
  for (auto it = key.values_.cbegin(); it != key.values_.cend(); ++it) {
    if (!first) out << '/';
    out << it->first;
    for (auto it2 = it->second.cbegin(); it2 != it->second.cend(); ++it2) {
      // MMH the java version checks to see if *it2 is actually an ident, and
      // if not, quotes/escapes it. a nice touch, but maybe more trouble than
      // it's worth.
      //if (identRegex.matcher(*it2).matches())
      out << '.' << *it2;
      //else
      //  out << ".'" << v.replace("'", "\\'") << '\'';
    }
    first = false;
  }
  return out;
}

}
