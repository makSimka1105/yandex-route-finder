#pragma once

#include <iostream>
#include <vector>

#include "cli/cli_handler.hpp"

namespace route_finder {

class HelpHandler : public ICliHandler {
 public:
  explicit HelpHandler(std::ostream& out) : ICliHandler(), out_(out) {}

 protected:
  CliExitCode HandleCommand(const std::vector<std::string>& args) override {
    if (args.empty() || args[0] != "help") {
      return CliExitCode::kNotHandled;
    }
    PrintHelp(out_);
    return CliExitCode::kContinue;
  }

 public:
  static void PrintHelp(std::ostream& out) {
    out << "Commands:\n"
        << "  help\n"
        << "  show\n"
        << "  from <code | query>\n"
        << "  to <code | query>\n"
        << "  date <YYYY-MM-DD> | date clear  (if unset, search uses today)\n"
        << "  transfers on | transfers off | transfers clear  (default on)\n"
        << "  transport clear | transport <type>\n"
        << "  search\n"
        << "  find <query> | find city <query> | find station <query>\n"
        << "  exit | quit\n";
  }

 private:
  std::ostream& out_;
};

}  // namespace route_finder
