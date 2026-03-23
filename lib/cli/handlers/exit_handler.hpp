#pragma once

#include <string>
#include <vector>

#include "cli/cli_handler.hpp"

namespace route_finder {

class ExitHandler : public ICliHandler {
 public:
  ExitHandler() = default;

 protected:
  CliExitCode HandleCommand(const std::vector<std::string>& args) override {
    if (!args.empty() && (args[0] == "exit" || args[0] == "quit")) {
      return CliExitCode::kExitOk;
    }
    return CliExitCode::kNotHandled;
  }
};

}
