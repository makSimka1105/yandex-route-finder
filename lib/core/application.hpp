#pragma once

#include <cstddef>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "api/api_client.hpp"
#include "cache/cache_manager.hpp"
#include "domain/errors.hpp"
#include "domain/models.hpp"
#include "location/location_store.hpp"
#include "parsers/routes/routes_parser.hpp"
#include "parsers/stations/stations_parser.hpp"

namespace route_finder {

class Application {
 public:
  Application(ApiClient& api, CacheManager& cache, LocationStore& locations);

  ApiExpectedResult<void> SyncStations();
  ApiExpectedResult<std::vector<Route>> SearchRoutes(const SearchRequest& req);
  ApiExpectedResult<std::vector<Route>> SearchCurrentRoute();

  std::vector<const Location*> FindLocations(
      const std::string& query, size_t max_results = 20,
      std::optional<Location::LocationType> type = std::nullopt);

  bool HasStationsLoaded() const;

  void SetRouteFrom(const std::string& code);
  void SetRouteTo(const std::string& code);

  const Location* ResolveRouteEndpoint(const std::string& query) const;
  void SetRouteDate(const std::string& date);
  void ClearRouteDate();
  void TransportClear();
  void TransportSetAllowed(const std::set<std::string>& types);

  void SetAllowTransfers(bool allow);
  void ResetAllowTransfers();

  const std::string& RouteFrom() const { return route_from_; }
  const std::string& RouteTo() const { return route_to_; }
  const std::string& RouteDate() const { return route_date_; }
  std::string EffectiveRouteDate() const;
  const SearchPreferences& RoutePrefs() const { return route_prefs_; }

 private:
  SearchRequest BuildRouteRequest() const;

  ApiClient& api_;
  CacheManager& cache_;
  LocationStore& locations_;
  StationsParser stations_parser_;
  ResponseParser routes_parser_;

  std::string route_from_;
  std::string route_to_;
  std::string route_date_;
  SearchPreferences route_prefs_;
};

}
