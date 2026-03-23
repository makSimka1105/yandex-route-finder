#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "cli/cli_handler.hpp"
#include "help_handler.hpp"

namespace route_finder {

class FallbackHandler : public ICliHandler {
 public:
  FallbackHandler(std::ostream& out, std::ostream& err)
      : ICliHandler(), out_(out), err_(err) {}

 protected:
  CliExitCode HandleCommand(const std::vector<std::string>& args) override {
    if (!args.empty()) {
      err_ << "unknown command: " << args[0] << '\n';
    }
    HelpHandler::PrintHelp(out_);
    return CliExitCode::kContinue;
  }

 private:
  std::ostream& out_;
  std::ostream& err_;
};

}
