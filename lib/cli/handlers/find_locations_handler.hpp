#pragma once

#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "cli/cli_handler.hpp"
#include "core/application.hpp"
#include "domain/models.hpp"

namespace route_finder {

class FindLocationsHandler : public ICliHandler {
 public:
  FindLocationsHandler(Application& app, std::ostream& out, std::ostream& err)
      : ICliHandler(), app_(app), out_(out), err_(err) {}

 protected:
  CliExitCode HandleCommand(const std::vector<std::string>& args) override {
    if (args.empty() || args[0] != "find") {
      return CliExitCode::kNotHandled;
    }
    if (args.size() < 2) {
      err_ << "usage: find <query>\n"
           << "       find city <query>\n"
           << "       find station <query>\n";
      return CliExitCode::kContinue;
    }

    std::optional<Location::LocationType> filter;
    size_t query_begin = 1;
    if (args[1] == "city") {
      filter = Location::kCity;
      query_begin = 2;
    } else if (args[1] == "station") {
      filter = Location::kStation;
      query_begin = 2;
    }

    if (args.size() <= query_begin) {
      err_ << "usage: find <query> | find city <query> | find station <query>\n";
      return CliExitCode::kContinue;
    }

    std::string query;
    for (size_t j = query_begin; j < args.size(); ++j) {
      if (j > query_begin) {
        query += ' ';
      }
      query += args[j];
    }

    if (!app_.HasStationsLoaded()) {
      err_ << "station index is empty (stations were not loaded)\n";
      return CliExitCode::kContinue;
    }

    std::vector<const Location*> hits = app_.FindLocations(query, 20, filter);
    if (hits.empty()) {
      out_ << "no matches\n";
      return CliExitCode::kContinue;
    }
    for (const Location* loc : hits) {
      if (!loc) {
        continue;
      }
      out_ << loc->title << " [" << loc->code << "]\n";
    }
    return CliExitCode::kContinue;
  }

 private:
  Application& app_;
  std::ostream& out_;
  std::ostream& err_;
};

}
