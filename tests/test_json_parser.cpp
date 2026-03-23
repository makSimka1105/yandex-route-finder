#include <gtest/gtest.h>

#include <expected>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "api/api_client.hpp"
#include "domain/errors.hpp"
#include "domain/yandex_rasp.hpp"
#include "mock_http_client.hpp"
#include "models.hpp"

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

class RoutesSearchDirectContext : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    json_ = LoadMockJson("search_response.json");
    ASSERT_FALSE(json_.empty()) << "search_response.json missing or empty";
  }

  void SetUp() override {
    mock_.SetResponseRef(&json_);
    client_ = std::make_unique<ApiClient>(&mock_, "test_key");
    req_.from_code = "c2";
    req_.to_code = "c146";
  }

  inline static std::string json_;
  MockHttpClient mock_;
  std::unique_ptr<ApiClient> client_;
  SearchRequest req_;
};

TEST_F(RoutesSearchDirectContext, CountMatchesFile) {
  ApiExpectedResult<std::vector<Route>> result = client_->FetchRoutes(req_);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->size(), 34u);
}

TEST_F(RoutesSearchDirectContext, SearchMetaAndFirstTrainSegment) {
  ApiExpectedResult<std::vector<Route>> result = client_->FetchRoutes(req_);
  ASSERT_TRUE(result.has_value());
  ASSERT_FALSE(result->empty());

  const Route& route = result->front();
  EXPECT_EQ(route.departure, "Санкт-Петербург");
  EXPECT_EQ(route.arrival, "Симферополь");
  EXPECT_EQ(route.departure_code, "c2");
  EXPECT_EQ(route.arrival_code, "c146");
  ASSERT_EQ(route.segments.size(), 1u);

  const Segment& seg = route.segments.front();
  EXPECT_EQ(seg.transport_type, kTransportTrain);
  EXPECT_EQ(seg.carrier, "Гранд Сервис Экспресс (Таврия)");
  EXPECT_EQ(seg.from_station, "Санкт-Петербург (Московский вокзал)");
  EXPECT_EQ(seg.to_station, "Симферополь-Пасс.");
  EXPECT_EQ(seg.from_code, "s9602494");
  EXPECT_EQ(seg.to_code, "s9616627");
  EXPECT_EQ(seg.departure, "01:27:00");
  EXPECT_EQ(seg.arrival, "06:00:00");
}

TEST_F(RoutesSearchDirectContext, EachRouteIsSingleFlatSegment) {
  ApiExpectedResult<std::vector<Route>> result = client_->FetchRoutes(req_);
  ASSERT_TRUE(result.has_value());
  for (const Route& r : *result) {
    EXPECT_EQ(r.segments.size(), 1u);
  }
}

TEST_F(RoutesSearchDirectContext, NetworkError) {
  mock_.SetResponseRef(nullptr);
  mock_.response = std::unexpected(-1);
  ApiExpectedResult<std::vector<Route>> result = client_->FetchRoutes(req_);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ApiError::kNetworkError);
}

TEST_F(RoutesSearchDirectContext, InvalidJson) {
  mock_.SetResponseRef(nullptr);
  mock_.response = std::string("broken");
  ApiExpectedResult<std::vector<Route>> result = client_->FetchRoutes(req_);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ApiError::kParseError);
}

TEST_F(RoutesSearchDirectContext, OptionalTransportTypesInUrlWhenAllowedSet) {
  req_.prefs.transport.allowed = {kTransportBus, kTransportTrain};
  ApiExpectedResult<std::vector<Route>> result = client_->FetchRoutes(req_);
  ASSERT_TRUE(result.has_value());
  EXPECT_NE(mock_.last_requested_url.find("transport_types=bus,train"),
            std::string::npos);
}

TEST_F(RoutesSearchDirectContext, TransfersParamReflectsFlag) {
  req_.prefs.allow_transfers = false;
  (void)client_->FetchRoutes(req_);
  EXPECT_NE(mock_.last_requested_url.find("transfers=false"), std::string::npos);

  req_.prefs.allow_transfers = true;
  (void)client_->FetchRoutes(req_);
  EXPECT_NE(mock_.last_requested_url.find("transfers=true"), std::string::npos);
}

class RoutesSearchTransfersContext : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    json_ = LoadMockJson("search_transfers_response.json");
    ASSERT_FALSE(json_.empty()) << "search_transfers_response.json missing";
  }

  void SetUp() override {
    mock_.SetResponseRef(&json_);
    client_ = std::make_unique<ApiClient>(&mock_, "test_key");
    req_.from_code = "c98";
    req_.to_code = "c146";
    req_.prefs.allow_transfers = true;
  }

  inline static std::string json_;
  MockHttpClient mock_;
  std::unique_ptr<ApiClient> client_;
  SearchRequest req_;
};

TEST_F(RoutesSearchTransfersContext, CompositeRoutesFromFile) {
  ApiExpectedResult<std::vector<Route>> result = client_->FetchRoutes(req_);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->size(), 6u);

  const Route& route = result->front();
  EXPECT_EQ(route.departure, "Кельн");
  EXPECT_EQ(route.arrival, "Симферополь");
  EXPECT_EQ(route.departure_code, "c98");
  EXPECT_EQ(route.arrival_code, "c146");
  ASSERT_EQ(route.segments.size(), 3u);

  EXPECT_EQ(route.segments[0].transport_type, kTransportPlane);
  EXPECT_EQ(route.segments[0].thread_number, "4M 402");
  EXPECT_EQ(route.segments[0].carrier, "LATAM Argentina");
  EXPECT_EQ(route.segments[0].from_station, "Кельн");
  EXPECT_EQ(route.segments[0].to_station, "Анталья");
  EXPECT_EQ(route.segments[0].from_code, "c98");
  EXPECT_EQ(route.segments[0].to_code, "c11511");

  EXPECT_EQ(route.segments[1].transport_type, kTransportPlane);
  EXPECT_EQ(route.segments[1].thread_number, "A4 3052");
  EXPECT_EQ(route.segments[1].carrier, "Азимут");
  EXPECT_EQ(route.segments[1].from_code, "c11511");
  EXPECT_EQ(route.segments[1].to_code, "c35");

  EXPECT_EQ(route.segments[2].transport_type, kTransportBus);
  EXPECT_EQ(route.segments[2].carrier, "Краснодар ИП Селезень И.А.");
  EXPECT_TRUE(route.segments[2].thread_number.empty());
  EXPECT_EQ(route.segments[2].from_station, "Краснодар");
  EXPECT_EQ(route.segments[2].to_station, "Симферополь");
  EXPECT_EQ(route.segments[2].from_code, "c35");
  EXPECT_EQ(route.segments[2].to_code, "c146");
}

class StationsListContext : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    json_ = LoadMockJson("stations_list_response.json");
    ASSERT_FALSE(json_.empty()) << "stations_list_response.json missing";
  }

  void SetUp() override {
    mock_.SetResponseRef(&json_);
    client_ = std::make_unique<ApiClient>(&mock_, "test_key");
  }

  inline static std::string json_;
  MockHttpClient mock_;
  std::unique_ptr<ApiClient> client_;
};

TEST_F(StationsListContext, Count) {
  ApiExpectedResult<std::vector<Location>> result = client_->FetchStationsList();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->size(), 9u);
}

TEST_F(StationsListContext, EntryFields) {
  ApiExpectedResult<std::vector<Location>> result = client_->FetchStationsList();
  ASSERT_TRUE(result.has_value());
  const std::vector<Location>& entries = *result;

  EXPECT_EQ(entries[0].title, "Санкт-Петербург");
  EXPECT_EQ(entries[0].location_type, Location::kCity);

  EXPECT_EQ(entries[1].title, "Московский вокзал");
  EXPECT_EQ(entries[1].location_type, Location::kStation);
  ASSERT_TRUE(entries[1].transport_type.has_value());
  EXPECT_EQ(entries[1].group_city, "Санкт-Петербург");
}

TEST_F(StationsListContext, NetworkError) {
  mock_.SetResponseRef(nullptr);
  mock_.response = std::unexpected(-1);
  ApiExpectedResult<std::vector<Location>> result = client_->FetchStationsList();
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ApiError::kNetworkError);
}
