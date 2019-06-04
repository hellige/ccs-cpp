#include "ccs/domain.h"

#include <iostream>

#include "graphviz.h"
#include "ccs/context.h"
#include "dag/dag_builder.h"
#include "parser/loader.h"

namespace ccs {

namespace {

void origins(std::ostream &str, const std::vector<const CcsProperty *> &values) {
  bool first = true;
  for (auto it = values.cbegin(); it != values.cend(); ++it) {
      if (!first) str << ", ";
      str << (*it)->origin();
      first = false;
  }
}

class LoggingTracer : public CcsTracer {
  std::shared_ptr<CcsLogger> logger;
  bool logAccesses;

public:
  LoggingTracer(std::shared_ptr<CcsLogger> logger, bool logAccesses) :
    logger(std::move(logger)), logAccesses(logAccesses) {}

  virtual void onPropertyFound(
      const CcsContext &ccsContext,
      const std::string &propertyName,
      const CcsProperty &prop) {
    if (logAccesses) {
      std::ostringstream msg;
      msg << "Found property: " << propertyName
        << " = " << prop.strValue() << "\n";
      msg << "    at " << prop.origin() << " in context: [" << ccsContext << "]";
      logger->info(msg.str());
    }
  }

  virtual void onPropertyNotFound(
      const CcsContext &ccsContext,
      const std::string &propertyName) {
    if (logAccesses) {
      std::ostringstream msg;
      msg << "Property not found: " << propertyName << "\n";
      msg << "    in context: [" << ccsContext << "]";
      logger->info(msg.str());
    }
  }

  virtual void onConflict(
      const CcsContext &ccsContext,
      const std::string &propertyName,
      const std::vector<const CcsProperty *> values) {
    std::ostringstream msg;
    msg << "Conflict detected for property '" << propertyName
        << "' in context [" << ccsContext << "]. "
        << "(Conflicting settings at: [";
    origins(msg, values);
    msg << "].) Using most recent value.";
    logger->warn(msg.str());
  }

  virtual void onParseError(const std::string &msg) {
    logger->error(msg);
  }
};

struct StdErrLogger : public CcsLogger {
  virtual void info(const std::string &msg)
    { std::cerr << "INFO: " << msg << std::endl; }
  virtual void warn(const std::string &msg)
    { std::cerr << "WARN: " << msg << std::endl; }
  virtual void error(const std::string &msg)
    { std::cerr << "ERROR: " << msg << std::endl; }
};

struct NoImportResolver : public ImportResolver {
  virtual bool resolve(const std::string &,
      std::function<bool(std::istream &)>) {
    return false;
  }
};

NoImportResolver NoImportResolver;

}

std::shared_ptr<CcsLogger> CcsLogger::makeStdErrLogger() {
  return std::make_shared<StdErrLogger>();
}

std::shared_ptr<CcsTracer> CcsTracer::makeLoggingTracer(
  std::shared_ptr<CcsLogger> logger, bool logAccesses) {
  return std::make_shared<LoggingTracer>(std::move(logger), logAccesses);
}

ImportResolver &ImportResolver::None = NoImportResolver;

CcsDomain::CcsDomain(std::shared_ptr<CcsTracer> tracer) :
  dag(new DagBuilder(std::move(tracer))) {}

CcsDomain::CcsDomain(bool logAccesses) :
  dag(new DagBuilder(CcsTracer::makeLoggingTracer(
    CcsLogger::makeStdErrLogger(), logAccesses))) {}

CcsDomain::CcsDomain(std::shared_ptr<CcsLogger> log, bool logAccesses) :
  dag(new DagBuilder(CcsTracer::makeLoggingTracer(
    std::move(log), logAccesses))) {}

CcsDomain::~CcsDomain() {}

CcsDomain &CcsDomain::loadCcsStream(std::istream &stream,
    const std::string &fileName, ImportResolver &importResolver) {
  Loader loader(dag->root()->tracer());
  loader.loadCcsStream(stream, fileName, *dag, importResolver);
  return *this;
}

RuleBuilder CcsDomain::ruleBuilder() {
  return RuleBuilder(*dag);
}

CcsContext CcsDomain::build() {
  return CcsContext(dag->root());
}

void CcsDomain::logRuleDag(std::ostream &os) const {
  os << Dumper(*dag->root());
}

}
