#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "cli/cli_handler.hpp"
#include "core/application.hpp"

namespace route_finder {

class DateHandler : public ICliHandler {
 public:
  DateHandler(Application& app, std::ostream& out, std::ostream& err)
      : ICliHandler(), app_(app), out_(out), err_(err) {}

 protected:
  CliExitCode HandleCommand(const std::vector<std::string>& args) override {
    if (args.empty() || args[0] != "date") {
      return CliExitCode::kNotHandled;
    }
    if (args.size() < 2) {
      err_ << "usage: date <YYYY-MM-DD> | date clear\n";
      return CliExitCode::kContinue;
    }
    if (args[1] == "clear") {
      app_.ClearRouteDate();
      out_ << "date cleared\n";
      return CliExitCode::kContinue;
    }
    app_.SetRouteDate(args[1]);
    out_ << "date = " << app_.RouteDate() << '\n';
    return CliExitCode::kContinue;
  }

 private:
  Application& app_;
  std::ostream& out_;
  std::ostream& err_;
};

}
