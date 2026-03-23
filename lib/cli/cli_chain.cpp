#include "cli_chain.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "cli_handler.hpp"
#include "core/application.hpp"
#include "handlers/date_handler.hpp"
#include "handlers/exit_handler.hpp"
#include "handlers/fallback_handler.hpp"
#include "handlers/find_locations_handler.hpp"
#include "handlers/from_handler.hpp"
#include "handlers/help_handler.hpp"
#include "handlers/search_execute_handler.hpp"
#include "handlers/show_route_handler.hpp"
#include "handlers/to_handler.hpp"
#include "handlers/transfers_handler.hpp"
#include "handlers/transport_handler.hpp"

namespace route_finder {

namespace {

std::vector<std::string> SplitTokens(const std::string& line) {
  std::vector<std::string> tokens;
  std::istringstream iss(line);
  std::string word;
  while (iss >> word) {
    tokens.push_back(word);
  }
  return tokens;
}

}

CliChain::CliChain(Application& app, std::ostream& out, std::ostream& err)
    : out_(out), err_(err) {
  head_ = std::make_unique<HelpHandler>(out);

  head_->SetNext(std::make_unique<ShowRouteHandler>(app, out, err))
      ->SetNext(std::make_unique<FromHandler>(app, out, err))
      ->SetNext(std::make_unique<ToHandler>(app, out, err))
      ->SetNext(std::make_unique<DateHandler>(app, out, err))
      ->SetNext(std::make_unique<TransfersHandler>(app, out, err))
      ->SetNext(std::make_unique<TransportHandler>(app, out, err))
      ->SetNext(std::make_unique<SearchExecuteHandler>(app, out, err))
      ->SetNext(std::make_unique<FindLocationsHandler>(app, out, err))
      ->SetNext(std::make_unique<ExitHandler>())
      ->SetNext(std::make_unique<FallbackHandler>(out, err));
}

int CliChain::Run() { return Run(std::cin); }

int CliChain::Run(std::istream& input) {
  std::string line;
  while (true) {
    out_ << "> ";
    if (!std::getline(input, line)) {
      return 0;
    }
    std::vector<std::string> tokens = SplitTokens(line);
    if (tokens.empty()) {
      continue;
    }
    CliExitCode res = head_->Handle(tokens);
    if (res == CliExitCode::kExitOk) {
      return 0;
    }
    if (res == CliExitCode::kExitError) {
      return 1;
    }
  }
}

}
