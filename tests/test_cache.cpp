#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <thread>

#include "cache/cache_manager.hpp"
#include "cache/routes_cache.hpp"
#include "cache/stations_cache.hpp"
#include "domain/models.hpp"

namespace fs = std::filesystem;
using namespace route_finder;

namespace {

fs::path UniqueTempSubdir(const char* tag) {
  static int seq = 0;
  const fs::path p = fs::path(::testing::TempDir()) /
                     (std::string(tag) + "_" + std::to_string(seq++));
  std::error_code ec;
  fs::remove_all(p, ec);
  return p;
}

SearchRequest MakeRoute2030(const char* from_code, const char* to_code) {
  SearchRequest r;
  r.from_code = from_code;
  r.to_code = to_code;
  r.date = "2030-01-01";
  r.prefs.allow_transfers = false;
  return r;
}

void PutEmptySegmentsRoute(FileRoutesCache& cache, const SearchRequest& req) {
  cache.Put(req, R"({"segments":[]})");
}

}

TEST(StationsCache, GetMissingReturnsNullopt) {
  fs::path root = UniqueTempSubdir("st_empty");
  fs::create_directories(root);
  FileStationsCache cache(root / "stations.json");
  EXPECT_FALSE(cache.Get().has_value());
}

TEST(StationsCache, PutThenGetRoundTrip) {
  fs::path root = UniqueTempSubdir("st_round");
  fs::create_directories(root);
  const fs::path file = root / "stations.json";
  FileStationsCache cache(file, std::chrono::hours(24));
  const std::string payload = R"({"countries":[]})";
  cache.Put(payload);
  std::optional<std::string> got = cache.Get();
  ASSERT_TRUE(got.has_value());
  EXPECT_EQ(*got, payload);
}

TEST(StationsCache, ExpiredEntryIgnored) {
  fs::path root = UniqueTempSubdir("st_ttl");
  fs::create_directories(root);
  const fs::path file = root / "stations.json";
  FileStationsCache cache(file, std::chrono::seconds(0));
  cache.Put("{}");
  ASSERT_TRUE(cache.Get().has_value());
  std::this_thread::sleep_for(std::chrono::seconds(2));
  EXPECT_FALSE(cache.Get().has_value());
}

TEST(RoutesCache, GetPutAndSecondGetUsesCache) {
  fs::path dir = UniqueTempSubdir("rt_basic");
  fs::create_directories(dir);
  FileRoutesCache cache(dir, 20, std::chrono::hours(24));

  SearchRequest req;
  req.from_code = "c1";
  req.to_code = "c2";
  req.date = "2030-01-01";
  req.prefs.allow_transfers = false;

  EXPECT_FALSE(cache.Get(req).has_value());
  const std::string body = R"({"segments":[]})";
  cache.Put(req, body);
  std::optional<std::string> got = cache.Get(req);
  ASSERT_TRUE(got.has_value());
  EXPECT_EQ(*got, body);
}

TEST(RoutesCache, EvictsOldestWhenOverMaxEntries) {
  fs::path dir = UniqueTempSubdir("rt_lru");
  fs::create_directories(dir);
  FileRoutesCache cache(dir, 2, std::chrono::hours(24));

  PutEmptySegmentsRoute(cache, MakeRoute2030("aa", "bb"));
  PutEmptySegmentsRoute(cache, MakeRoute2030("cc", "dd"));
  PutEmptySegmentsRoute(cache, MakeRoute2030("ee", "ff"));

  EXPECT_FALSE(cache.Get(MakeRoute2030("aa", "bb")).has_value());
  ASSERT_TRUE(cache.Get(MakeRoute2030("cc", "dd")).has_value());
  ASSERT_TRUE(cache.Get(MakeRoute2030("ee", "ff")).has_value());
}

TEST(CacheManager, DelegatesStationsAndRoutes) {
  fs::path root = UniqueTempSubdir("cm_full");
  fs::create_directories(root);
  CacheManager mgr(root);

  EXPECT_FALSE(mgr.GetStations().has_value());
  mgr.PutStations("[]");
  ASSERT_TRUE(mgr.GetStations().has_value());

  SearchRequest req;
  req.from_code = "x";
  req.to_code = "y";
  req.date = "2030-02-02";
  req.prefs.allow_transfers = true;
  EXPECT_FALSE(mgr.GetRoutes(req).has_value());
  mgr.PutRoutes(req, "{}");
  ASSERT_TRUE(mgr.GetRoutes(req).has_value());
}
