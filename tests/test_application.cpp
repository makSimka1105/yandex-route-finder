#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "api/api_client.hpp"
#include "cache/cache_manager.hpp"
#include "core/application.hpp"
#include "domain/errors.hpp"
#include "location/location_store.hpp"
#include "mock_http_client.hpp"

using namespace route_finder;

namespace {

std::string LoadMockJson(const std::string& filename) {
  std::string path = std::string(MOCK_JSONS "/") + filename;
  std::ifstream file(path);
  if (!file.is_open()) {
    return {};
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

}

class ApplicationContext : public ::testing::Test {
 protected:
  void SetUp() override {
    stations_json_ = LoadMockJson("stations_list_response.json");
    ASSERT_FALSE(stations_json_.empty());
    routes_json_ =
        R"({"search":{"from":{"title":"A"},"to":{"title":"B"}},"segments":[]})";

    mock_.ResetCallCount();
    mock_.SetStationsResponseRef(&stations_json_);
    mock_.SetRoutesResponseRef(&routes_json_);

    std::ostringstream dir;
    dir << "app_" << static_cast<const void*>(this);
    cache_root_ = std::filesystem::path(::testing::TempDir()) / dir.str();
    std::filesystem::create_directories(cache_root_);
    cache_ = std::make_unique<CacheManager>(cache_root_);

    api_ = std::make_unique<ApiClient>(&mock_, "test_key", nullptr);
    app_ = std::make_unique<Application>(*api_, *cache_, locations_);
  }

  std::string stations_json_;
  std::string routes_json_;
  MockHttpClient mock_;
  std::filesystem::path cache_root_;
  std::unique_ptr<CacheManager> cache_;
  std::unique_ptr<ApiClient> api_;
  LocationStore locations_;
  std::unique_ptr<Application> app_;
};

TEST_F(ApplicationContext, SyncStationsBuildsIndex) {
  ASSERT_TRUE(app_->SyncStations().has_value());
  EXPECT_TRUE(app_->HasStationsLoaded());
  EXPECT_NE(app_->ResolveRouteEndpoint("c2"), nullptr);
}

TEST_F(ApplicationContext, SecondSyncUsesStationsCache) {
  ASSERT_TRUE(app_->SyncStations().has_value());
  int after_first = mock_.GetCallCount();
  ASSERT_TRUE(app_->SyncStations().has_value());
  EXPECT_EQ(mock_.GetCallCount(), after_first);
}

TEST_F(ApplicationContext, SearchRoutesCachesRawResponse) {
  ASSERT_TRUE(app_->SyncStations().has_value());
  app_->SetRouteFrom("c2");
  app_->SetRouteTo("s9602497");

  ApiExpectedResult<std::vector<Route>> r1 = app_->SearchCurrentRoute();
  ASSERT_TRUE(r1.has_value());
  int after_first_search = mock_.GetCallCount();

  ApiExpectedResult<std::vector<Route>> r2 = app_->SearchCurrentRoute();
  ASSERT_TRUE(r2.has_value());
  EXPECT_EQ(mock_.GetCallCount(), after_first_search);
}

TEST_F(ApplicationContext, ResolveRouteEndpointEmpty) {
  ASSERT_TRUE(app_->SyncStations().has_value());
  EXPECT_EQ(app_->ResolveRouteEndpoint(""), nullptr);
}

TEST_F(ApplicationContext, FindLocationsRespectsTypeFilter) {
  ASSERT_TRUE(app_->SyncStations().has_value());
  auto cities = app_->FindLocations("Санкт", 10, Location::kCity);
  ASSERT_FALSE(cities.empty());
  for (const Location* p : cities) {
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->location_type, Location::kCity);
  }
}

TEST_F(ApplicationContext, RoutePrefsDateAndTransport) {
  app_->SetRouteDate("2031-06-15");
  EXPECT_EQ(app_->RouteDate(), "2031-06-15");
  EXPECT_EQ(app_->EffectiveRouteDate(), "2031-06-15");
  app_->ClearRouteDate();
  EXPECT_TRUE(app_->RouteDate().empty());
  EXPECT_FALSE(app_->EffectiveRouteDate().empty());

  app_->TransportSetAllowed({"train", "bus"});
  EXPECT_FALSE(app_->RoutePrefs().transport.allowed.empty());
  app_->TransportClear();
  EXPECT_TRUE(app_->RoutePrefs().transport.allowed.empty());

  EXPECT_TRUE(app_->RoutePrefs().allow_transfers);
  app_->SetAllowTransfers(false);
  EXPECT_FALSE(app_->RoutePrefs().allow_transfers);
  app_->ResetAllowTransfers();
  EXPECT_TRUE(app_->RoutePrefs().allow_transfers);
}

TEST(ApplicationNetworkFailure, SyncStationsOnNetworkError) {
  MockHttpClient mock;
  mock.SetResponseRef(nullptr);
  mock.response = std::unexpected(-1);
  CacheManager cache(std::filesystem::path(::testing::TempDir()) / "nw_netfail");
  ApiClient api(&mock, "k", nullptr);
  LocationStore loc;
  Application app(api, cache, loc);
  auto r = app.SyncStations();
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), ApiError::kNetworkError);
}

TEST(ApplicationParseFailure, SyncStationsInvalidJson) {
  static const std::string bad = "{";
  MockHttpClient mock;
  mock.SetStationsResponseRef(&bad);
  CacheManager cache(std::filesystem::path(::testing::TempDir()) / "pj_parsefail");
  ApiClient api(&mock, "k", nullptr);
  LocationStore loc;
  Application app(api, cache, loc);
  auto r = app.SyncStations();
  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), ApiError::kParseError);
}

TEST(ApiClientHttpLog, WritesOnFailure) {
  MockHttpClient mock;
  mock.SetResponseRef(nullptr);
  mock.response = std::unexpected(403);
  std::ostringstream log;
  ApiClient api(&mock, "key", &log);
  auto r = api.FetchStationsListRaw();
  ASSERT_FALSE(r.has_value());
  EXPECT_NE(log.str().find("403"), std::string::npos);
  EXPECT_NE(log.str().find("stations_list"), std::string::npos);
}

TEST(ApiClientFetchStationsList, ParsesWhenUsingHighLevelMethod) {
  std::string json = LoadMockJson("stations_list_response.json");
  ASSERT_FALSE(json.empty());
  MockHttpClient mock;
  mock.SetStationsResponseRef(&json);
  ApiClient api(&mock, "k", nullptr);
  auto r = api.FetchStationsList();
  ASSERT_TRUE(r.has_value());
  EXPECT_FALSE(r->empty());
}
