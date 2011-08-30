#ifndef CCS_PARSER_H_
#define CCS_PARSER_H_

#include <string>

namespace ccs {

class Node;

class Parser {
public:
  bool parseString(std::string input);
  bool parseString(Node &root, std::string input);
};

}


#endif /* CCS_PARSER_H_ */
