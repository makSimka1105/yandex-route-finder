#pragma once

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>

#include "domain/models.hpp"
#include "routes_cache.hpp"
#include "stations_cache.hpp"

namespace route_finder {

class CacheManager {
 public:
  explicit CacheManager(std::filesystem::path cache_root = "./cache_files")
      : stations_cache_(cache_root / "stations.json", std::chrono::hours(24 * 7)),
        routes_cache_(cache_root / "routes", 20, std::chrono::hours(24)) {}

  std::optional<std::string> GetStations() const { return stations_cache_.Get(); }

  void PutStations(const std::string& raw_json) { stations_cache_.Put(raw_json); }

  std::optional<std::string> GetRoutes(const SearchRequest& req) {
    return routes_cache_.Get(req);
  }

  void PutRoutes(const SearchRequest& req, const std::string& raw_json) {
    routes_cache_.Put(req, raw_json);
  }

 private:
  FileStationsCache stations_cache_;
  FileRoutesCache routes_cache_;
};

}
