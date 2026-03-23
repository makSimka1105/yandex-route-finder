#include "stations_parser.hpp"

#include <expected>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

#include "errors.hpp"
#include "location_handlers.hpp"
#include "models.hpp"
#include "nlohmann/json_fwd.hpp"

namespace route_finder {

StationsParser::StationsParser() {
  location_chain_ = std::make_unique<TitleExtractor>();
  location_chain_->SetNext(std::make_unique<CodeExtractor>())
                 ->SetNext(std::make_unique<TransportExtractor>())
                 ->SetNext(std::make_unique<StationTypeExtractor>());
}

ApiExpectedResult<std::vector<Location>> StationsParser::Parse(
    std::string raw_string) {
  ApiExpectedResult<nlohmann::json> json_result =
      TryParseJson(std::move(raw_string));
  if (!json_result) {
    return std::unexpected(json_result.error());
  }
  return ExtractLocations(*json_result);
}

ApiExpectedResult<nlohmann::json> StationsParser::TryParseJson(
    std::string raw) {
  try {
    return nlohmann::json::parse(raw);
  } catch (const nlohmann::json::parse_error&) {
    return std::unexpected(ApiError::kParseError);
  }
}

std::vector<Location> StationsParser::ExtractLocations(
    const nlohmann::json& json) {
  std::vector<Location> locations;
  if (!json.contains("countries") || !json["countries"].is_array()) {
    return locations;
  }

  for (const auto& country : json["countries"]) {
    if (!country.contains("regions") || !country["regions"].is_array()) {
      continue;
    }

    for (const auto& region : country["regions"]) {
      if (!region.contains("settlements") || !region["settlements"].is_array()) {
        continue;
      }

      for (const auto& settlement : region["settlements"]) {
        Location location{.location_type = Location::kCity};

        location_chain_->Handle(settlement, location);

        if (location.title.empty() || location.code.empty()) {
          continue;
        }

        std::string city_title = location.title;
        locations.push_back(std::move(location));

        if (!settlement.contains("stations") ||
            !settlement["stations"].is_array()) {
          continue;
        }

        for (const auto& station : settlement["stations"]) {
          Location station_entry{.location_type = Location::kStation,
                                 .group_city = city_title};
          location_chain_->Handle(station, station_entry);
          if (!station_entry.title.empty() && !station_entry.code.empty()) {
            locations.push_back(std::move(station_entry));
          }
        }
      }
    }
  }

  return locations;
}

}
