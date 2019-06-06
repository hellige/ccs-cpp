#pragma once

#include <functional>
#include <memory>
#include <iosfwd>
#include <string>

#include "ccs/context.h"
#include "ccs/rule_builder.h"

namespace ccs {

class DagBuilder;

class CcsLogger {
public:
  virtual ~CcsLogger() {}
  virtual void info(const std::string &msg) = 0;
  virtual void warn(const std::string &msg) = 0;
  virtual void error(const std::string &msg) = 0;

  static std::shared_ptr<CcsLogger> makeStdErrLogger();
};

class CcsTracer {
public:
  virtual ~CcsTracer() {}
  virtual void onPropertyFound(
      const CcsContext &ccsContext,
      const std::string &propertyName,
      const CcsProperty &prop) = 0;
  virtual void onPropertyNotFound(
      const CcsContext &ccsContext,
      const std::string &propertyName) = 0;
  virtual void onConflict(
      const CcsContext &ccsContext,
      const std::string &propertyName,
      const std::vector<const CcsProperty *> values) = 0;
  virtual void onParseError(const std::string &msg) = 0;

  static std::shared_ptr<CcsTracer> makeLoggingTracer(
    std::shared_ptr<CcsLogger> logger, bool logAccesses = false);
};

class ImportResolver {
public:
  static ImportResolver &None;
  virtual ~ImportResolver() {}
  virtual bool resolve(const std::string &location,
      std::function<bool(std::istream &)> load) = 0;
};

class CcsDomain {
  std::unique_ptr<DagBuilder> dag;

public:
  explicit CcsDomain(bool logAccesses = false);
  explicit CcsDomain(std::shared_ptr<CcsLogger> log, bool logAccesses = false);
  explicit CcsDomain(std::shared_ptr<CcsTracer> tracer);
  ~CcsDomain();
  CcsDomain(const CcsDomain &) = delete;
  CcsDomain &operator=(const CcsDomain &) = delete;

  CcsDomain &loadCcsStream(std::istream &stream, const std::string &fileName,
      ImportResolver &importResolver);
  RuleBuilder ruleBuilder();

  void logRuleDag(std::ostream &os) const;

  CcsContext build();
};

}
