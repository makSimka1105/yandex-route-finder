#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "cli/cli_handler.hpp"
#include "core/application.hpp"

namespace route_finder {

class TransfersHandler : public ICliHandler {
 public:
  TransfersHandler(Application& app, std::ostream& out, std::ostream& err)
      : ICliHandler(), app_(app), out_(out), err_(err) {}

 protected:
  CliExitCode HandleCommand(const std::vector<std::string>& args) override {
    if (args.empty() || args[0] != "transfers") {
      return CliExitCode::kNotHandled;
    }
    if (args.size() < 2) {
      err_ << "usage: transfers on | transfers off | transfers clear\n";
      return CliExitCode::kContinue;
    }
    const std::string& a = args[1];
    if (a == "on") {
      app_.SetAllowTransfers(true);
      out_ << "transfers: on\n";
      return CliExitCode::kContinue;
    }
    if (a == "off") {
      app_.SetAllowTransfers(false);
      out_ << "transfers: off\n";
      return CliExitCode::kContinue;
    }
    if (a == "clear") {
      app_.ResetAllowTransfers();
      out_ << "transfers reset to default (on)\n";
      return CliExitCode::kContinue;
    }
    err_ << "usage: transfers on | transfers off | transfers clear\n";
    return CliExitCode::kContinue;
  }

 private:
  Application& app_;
  std::ostream& out_;
  std::ostream& err_;
};

}  // namespace route_finder
