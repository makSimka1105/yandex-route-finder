#include "application.hpp"

#include <cstddef>
#include <ctime>
#include <expected>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include "api_client.hpp"
#include "cache_manager.hpp"
#include "errors.hpp"
#include "location_store.hpp"
#include "models.hpp"
#include <vector>

namespace route_finder {
namespace {

std::string TodayLocalDateString() {
  std::time_t t = std::time(nullptr);
  char buf[16];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
  return buf;
}

}

std::string Application::EffectiveRouteDate() const {
  if (!route_date_.empty()) {
    return route_date_;
  }
  return TodayLocalDateString();
}

Application::Application(ApiClient& api, CacheManager& cache, LocationStore& locations)
    : api_(api), cache_(cache), locations_(locations) {}

ApiExpectedResult<void> Application::SyncStations() {
  std::string json;
  std::optional<std::string> cached_stations = cache_.GetStations();
  if (cached_stations) {
    json = std::move(*cached_stations);
  } else {
    ApiExpectedResult<std::string> fetched = api_.FetchStationsListRaw();
    if (!fetched) {
      return std::unexpected(fetched.error());
    }
    cache_.PutStations(*fetched);
    json = std::move(*fetched);
  }

  ApiExpectedResult<std::vector<Location>> parsed =
      stations_parser_.Parse(std::move(json));
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  locations_.Build(std::move(*parsed));
  return {};
}

ApiExpectedResult<std::vector<Route>> Application::SearchRoutes(const SearchRequest& req) {
  std::optional<std::string> cached_routes = cache_.GetRoutes(req);
  if (cached_routes) {
    return routes_parser_.Parse(std::move(*cached_routes));
  }
  ApiExpectedResult<std::string> raw_routes = api_.FetchRoutesRaw(req);
  if (!raw_routes) {
    return std::unexpected(raw_routes.error());
  }
  cache_.PutRoutes(req, *raw_routes);
  return routes_parser_.Parse(std::move(*raw_routes));
}

SearchRequest Application::BuildRouteRequest() const {
  SearchRequest req;
  req.from_code = route_from_;
  req.to_code = route_to_;
  req.date = EffectiveRouteDate();
  req.prefs = route_prefs_;
  return req;
}

ApiExpectedResult<std::vector<Route>> Application::SearchCurrentRoute() {
  return SearchRoutes(BuildRouteRequest());
}

std::vector<const Location*> Application::FindLocations(
    const std::string& query, size_t max_results,
    std::optional<Location::LocationType> type) {
  return locations_.FindTopN(query, max_results, type);
}

bool Application::HasStationsLoaded() const { return !locations_.Empty(); }

const Location* Application::ResolveRouteEndpoint(const std::string& query) const {
  if (query.empty()) {
    return nullptr;
  }
  if (const Location* by_code = locations_.FindByCode(query)) {
    return by_code;
  }
  return locations_.FindBest(query);
}

void Application::SetRouteFrom(const std::string& code) { route_from_ = code; }

void Application::SetRouteTo(const std::string& code) { route_to_ = code; }

void Application::SetRouteDate(const std::string& date) { route_date_ = date; }

void Application::ClearRouteDate() { route_date_.clear(); }

void Application::TransportClear() { route_prefs_.transport.allowed.clear(); }

void Application::TransportSetAllowed(const std::set<std::string>& types) {
  route_prefs_.transport.allowed = types;
}

void Application::SetAllowTransfers(bool allow) {
  route_prefs_.allow_transfers = allow;
}

void Application::ResetAllowTransfers() { route_prefs_.allow_transfers = true; }

}
