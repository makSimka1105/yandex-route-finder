#include "routes_parser.hpp"

#include <expected>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "errors.hpp"
#include "models.hpp"
#include "nlohmann/json_fwd.hpp"
#include "segment_handlers.hpp"

namespace route_finder {
namespace {

bool ItemHasDetails(const nlohmann::json& segment_item) {
  return segment_item.contains("details") &&
         segment_item["details"].is_array() && !segment_item["details"].empty();
}

bool DetailIsTransferWaitBlock(const nlohmann::json& detail) {
  return detail.contains("is_transfer") && detail["is_transfer"].is_boolean() &&
         detail["is_transfer"].get<bool>();
}

bool DetailHasRideBranch(const nlohmann::json& detail) {
  return detail.contains("thread") && detail["thread"].is_object();
}

void ReadSearchMeta(const nlohmann::json& root, std::string& from_title,
                    std::string& to_title, std::string& from_code,
                    std::string& to_code) {
  if (!root.contains("search") || !root["search"].is_object()) {
    return;
  }
  const nlohmann::json& search = root["search"];
  if (search.contains("from") && search["from"].is_object()) {
    const nlohmann::json& from = search["from"];
    if (from.contains("title")) {
      from_title = from["title"].get<std::string>();
    }
    if (from.contains("code") && from["code"].is_string()) {
      from_code = from["code"].get<std::string>();
    }
  }
  if (search.contains("to") && search["to"].is_object()) {
    const nlohmann::json& to = search["to"];
    if (to.contains("title")) {
      to_title = to["title"].get<std::string>();
    }
    if (to.contains("code") && to["code"].is_string()) {
      to_code = to["code"].get<std::string>();
    }
  }
}

}

ResponseParser::ResponseParser() {
  segment_chain_ = std::make_unique<StationExtractor>();
  segment_chain_->SetNext(std::make_unique<ThreadExtractor>())
      ->SetNext(std::make_unique<TimeExtractor>());
}

ApiExpectedResult<std::vector<Route>> ResponseParser::Parse(
    std::string raw_string) {
  ApiExpectedResult<nlohmann::json> json_result =
      TryParseJson(std::move(raw_string));
  if (!json_result) {
    return std::unexpected(json_result.error());
  }
  return ExtractRoutes(*json_result);
}

ApiExpectedResult<nlohmann::json> ResponseParser::TryParseJson(
    std::string raw) {
  try {
    return nlohmann::json::parse(raw);
  } catch (const nlohmann::json::parse_error&) {
    return std::unexpected(ApiError::kParseError);
  }
}

std::vector<Route> ResponseParser::ExtractRoutes(const nlohmann::json& json) {
  std::vector<Route> routes;
  if (!json.contains("segments") || !json["segments"].is_array()) {
    return routes;
  }

  std::string departure_city;
  std::string arrival_city;
  std::string departure_code;
  std::string arrival_code;
  ReadSearchMeta(json, departure_city, arrival_city, departure_code,
                 arrival_code);

  for (const auto& segment_json : json["segments"]) {
    Route route;
    route.departure = departure_city;
    route.arrival = arrival_city;
    route.departure_code = departure_code;
    route.arrival_code = arrival_code;

    if (ItemHasDetails(segment_json)) {
      AppendBranchesFromDetails(segment_json, route);
      if (!route.segments.empty()) {
        routes.push_back(std::move(route));
      }
      continue;
    }

    Segment seg;
    segment_chain_->Handle(segment_json, seg);
    route.segments.push_back(std::move(seg));
    routes.push_back(std::move(route));
  }

  return routes;
}

void ResponseParser::AppendBranchesFromDetails(const nlohmann::json& item,
                                               Route& route) {
  for (const auto& detail : item["details"]) {
    if (DetailIsTransferWaitBlock(detail)) {
      continue;
    }
    if (!DetailHasRideBranch(detail)) {
      continue;
    }
    Segment seg;
    segment_chain_->Handle(detail, seg);
    route.segments.push_back(std::move(seg));
  }
}

}
