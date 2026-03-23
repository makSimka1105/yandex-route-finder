#pragma once

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include "cli/cli_handler.hpp"
#include "core/application.hpp"
#include "domain/errors.hpp"
#include "domain/models.hpp"

namespace route_finder {
namespace {

std::string StationLabel(std::string title, const std::string& code) {
  if (!code.empty()) {
    if (title.empty()) {
      return '[' + code + ']';
    }
    title += " [";
    title += code;
    title += ']';
  }
  return title;
}

}

class SearchExecuteHandler : public ICliHandler {
 public:
  SearchExecuteHandler(Application& app, std::ostream& out, std::ostream& err)
      : ICliHandler(), app_(app), out_(out), err_(err) {}

 protected:
  CliExitCode HandleCommand(const std::vector<std::string>& args) override {
    if (args.empty() || args[0] != "search") {
      return CliExitCode::kNotHandled;
    }
    if (args.size() != 1) {
      err_ << "usage: search  (specify from and to)\n";
      return CliExitCode::kContinue;
    }
    if (app_.RouteFrom().empty() || app_.RouteTo().empty()) {
      err_ << "set endpoints: from <code | query> and to <code | query> \n";
      return CliExitCode::kContinue;
    }
    ApiExpectedResult<std::vector<Route>> routes = app_.SearchCurrentRoute();
    if (!routes) {
      err_ << "search failed: " << ApiErrorMessage(routes.error()) << '\n';
      return CliExitCode::kContinue;
    }
    size_t i = 0;
    for (const Route& rt : *routes) {
      out_ << "route " << (++i) << ": "
           << StationLabel(rt.departure, rt.departure_code) << " .. "
           << StationLabel(rt.arrival, rt.arrival_code) << '\n';
      for (const Segment& seg : rt.segments) {
        out_ << "  " << StationLabel(seg.from_station, seg.from_code) << " -> "
             << StationLabel(seg.to_station, seg.to_code) << " ("
             << seg.transport_type << ") " << seg.departure << " .. "
             << seg.arrival << '\n';
      }
    }
    if (i == 0) {
      out_ << "no routes returned\n";
    }
    return CliExitCode::kContinue;
  }

 private:
  Application& app_;
  std::ostream& out_;
  std::ostream& err_;
};

}
