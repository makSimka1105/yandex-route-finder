#pragma once

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace route_finder {

struct Location {
  enum LocationType { kCity, kStation };
  std::string title;
  std::string code;
  LocationType location_type = LocationType::kCity;
  std::string station_type;
  std::optional<std::string> transport_type;
  std::optional<std::string> group_city;
};

struct TransportPreferences {
  std::set<std::string> allowed;

  bool IsAllowed(const std::string& transport_type) const {
    return allowed.empty() || allowed.contains(transport_type);
  }
};

struct SearchPreferences {
  TransportPreferences transport;
  bool allow_transfers = true;
};

struct SearchRequest {
  std::string from_code;
  std::string to_code;
  std::string date;
  SearchPreferences prefs;
};

struct Segment {
  std::string from_station;
  std::string to_station;
  std::string from_code;
  std::string to_code;
  std::string transport_type;
  std::string departure;
  std::string arrival;
  std::string carrier;
  std::string thread_number;
};

struct Route {
  std::string departure;
  std::string arrival;
  std::string departure_code;
  std::string arrival_code;
  std::vector<Segment> segments;
};

}