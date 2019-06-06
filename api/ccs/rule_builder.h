#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ccs {

class DagBuilder;

class RuleBuilder {
  friend class CcsDomain;
  class Impl;
  class Root;
  class Child;
  std::shared_ptr<Impl> impl;

  RuleBuilder(DagBuilder &dag);
  RuleBuilder(const std::shared_ptr<Impl> &impl) : impl(impl) {}

public:
  RuleBuilder pop();
  RuleBuilder set(const std::string &name, const std::string &value);

  RuleBuilder select(const std::string &name);
  RuleBuilder select(const std::string &name,
      const std::vector<std::string> &values);
};

}
