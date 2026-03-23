#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "cli/cli_handler.hpp"
#include "core/application.hpp"
#include "models.hpp"

namespace route_finder {

class ShowRouteHandler : public ICliHandler {
 public:
  ShowRouteHandler(Application& app, std::ostream& out, std::ostream& err)
      : ICliHandler(), app_(app), out_(out), err_(err) {}

 protected:
  CliExitCode HandleCommand(const std::vector<std::string>& args) override {
    if (args.empty() || args[0] != "show") {
      return CliExitCode::kNotHandled;
    }
    if (args.size() != 1) {
      err_ << "usage: show\n";
      return CliExitCode::kContinue;
    }
    out_ << "from: " << (app_.RouteFrom().empty() ? "(not set)" : app_.RouteFrom())
         << "\nto: " << (app_.RouteTo().empty() ? "(not set)" : app_.RouteTo())
         << "\ndate: ";
    if (app_.RouteDate().empty()) {
      out_ << "(not set; search uses today: " << app_.EffectiveRouteDate() << ")\n";
    } else {
      out_ << app_.RouteDate() << '\n';
    }
    const SearchPreferences& p = app_.RoutePrefs();
    if (p.transport.allowed.empty()) {
      out_ << "transport: (all)\n";
    } else {
      out_ << "transport:";
      for (const std::string& t : p.transport.allowed) {
        out_ << ' ' << t;
      }
      out_ << '\n';
    }
    out_ << "transfers: " << (p.allow_transfers ? "on" : "off") << '\n';
    return CliExitCode::kContinue;
  }

 private:
  Application& app_;
  std::ostream& out_;
  std::ostream& err_;
};

}
