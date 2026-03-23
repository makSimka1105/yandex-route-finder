#pragma once

#include <cstddef>
#include <iostream>
#include <set>
#include <vector>
#include <string>

#include "cli/cli_handler.hpp"
#include "core/application.hpp"
#include "domain/yandex_rasp.hpp"

namespace route_finder {

class TransportHandler : public ICliHandler {
 public:
  TransportHandler(Application& app, std::ostream& out, std::ostream& err)
      : ICliHandler(), app_(app), out_(out), err_(err) {}

 protected:
  CliExitCode HandleCommand(const std::vector<std::string>& args) override {
    if (args.empty() || args[0] != "transport") {
      return CliExitCode::kNotHandled;
    }
    if (args.size() < 2) {
      err_ << "usage: transport clear | transport <type> [<type> ...]\n"
           << "allowed types: ";
      for (size_t i = 0; i < kKnownYandexTransportTypes.size(); ++i) {
        if (i != 0) {
          err_ << ' ';
        }
        err_ << kKnownYandexTransportTypes[i];
      }
      err_ << '\n';
      return CliExitCode::kContinue;
    }
    if (args[1] == "clear") {
      app_.TransportClear();
      out_ << "transport filter cleared (all types allowed)\n";
      return CliExitCode::kContinue;
    }
    std::set<std::string> types;
    for (size_t i = 1; i < args.size(); ++i) {
      types.insert(args[i]);
    }
    app_.TransportSetAllowed(types);
    out_ << "transport filter set (" << types.size() << " type(s))\n";
    return CliExitCode::kContinue;
  }

 private:
  Application& app_;
  std::ostream& out_;
  std::ostream& err_;
};

}
