#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace route_finder {

enum class CliExitCode {
  kNotHandled,
  kContinue,
  kExitOk,
  kExitError,
};

class ICliHandler {
 public:
  ICliHandler() = default;
  virtual ~ICliHandler() = default;

  ICliHandler* SetNext(std::unique_ptr<ICliHandler> next) {
    next_ = std::move(next);
    return next_.get();
  }

  CliExitCode Handle(const std::vector<std::string>& args) {
    CliExitCode result = HandleCommand(args);
    if (result == CliExitCode::kNotHandled) {
      if (next_) {
        return next_->Handle(args);
      }
      return CliExitCode::kContinue;
    }
    return result;
  }

 protected:
  virtual CliExitCode HandleCommand(const std::vector<std::string>& args) = 0;

 private:
  std::unique_ptr<ICliHandler> next_;
};

}
