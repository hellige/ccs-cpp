#ifndef CCS_KEY_H_
#define CCS_KEY_H_

namespace ccs {

class Key {
public:
  Key ();

  bool operator<(const Key &) const { return false; }
};

} /* namespace ccs */

#endif /* CCS_KEY_H_ */
