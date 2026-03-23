#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "cli/cli_handler.hpp"
#include "core/application.hpp"
#include "domain/models.hpp"

namespace route_finder {

class FromHandler : public ICliHandler {
 public:
  FromHandler(Application& app, std::ostream& out, std::ostream& err)
      : ICliHandler(), app_(app), out_(out), err_(err) {}

 protected:
  CliExitCode HandleCommand(const std::vector<std::string>& args) override {
    if (args.empty() || args[0] != "from") {
      return CliExitCode::kNotHandled;
    }
    if (args.size() < 2) {
      err_ << "usage: from <code | query>\n";
      return CliExitCode::kContinue;
    }
    const Location* loc = app_.ResolveRouteEndpoint(args[1]);
    if (loc == nullptr) {
      err_ << "from: no matching station or city for \"" << args[1] << "\"\n";
      return CliExitCode::kContinue;
    }
    app_.SetRouteFrom(loc->code);
    out_ << "from = " << loc->title << " [" << loc->code << "]\n";
    return CliExitCode::kContinue;
  }

 private:
  Application& app_;
  std::ostream& out_;
  std::ostream& err_;
};

}
