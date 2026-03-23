#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "cli/cli_handler.hpp"
#include "core/application.hpp"
#include "domain/models.hpp"

namespace route_finder {

class ToHandler : public ICliHandler {
 public:
  ToHandler(Application& app, std::ostream& out, std::ostream& err)
      : ICliHandler(), app_(app), out_(out), err_(err) {}

 protected:
  CliExitCode HandleCommand(const std::vector<std::string>& args) override {
    if (args.empty() || args[0] != "to") {
      return CliExitCode::kNotHandled;
    }
    if (args.size() < 2) {
      err_ << "usage: to <code | query>\n";
      return CliExitCode::kContinue;
    }
    const Location* loc = app_.ResolveRouteEndpoint(args[1]);
    if (loc == nullptr) {
      err_ << "to: no matching station or city for \"" << args[1] << "\"\n";
      return CliExitCode::kContinue;
    }
    app_.SetRouteTo(loc->code);
    out_ << "to = " << loc->title << " [" << loc->code << "]\n";
    return CliExitCode::kContinue;
  }

 private:
  Application& app_;
  std::ostream& out_;
  std::ostream& err_;
};

}
