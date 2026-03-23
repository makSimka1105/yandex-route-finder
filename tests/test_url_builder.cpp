#include <gtest/gtest.h>
#include <string>

#include "api/url_builder.hpp"

using namespace route_finder;

TEST(ApiUrlBuilder, BasicUrl) {
  std::string url = ApiUrlBuilder()
                        .SetBase("https://api.rasp.yandex-net.ru/v3.0")
                        .SetEndpoint("search")
                        .AddParam("apikey", "test_key")
                        .AddParam("from", "c2")
                        .AddParam("to", "c150")
                        .Build();

  EXPECT_TRUE(url.starts_with("https://api.rasp.yandex-net.ru/v3.0/search/"));
  EXPECT_NE(url.find("apikey=test_key"), std::string::npos);
  EXPECT_NE(url.find("from=c2"), std::string::npos);
  EXPECT_NE(url.find("to=c150"), std::string::npos);
}

TEST(ApiUrlBuilder, ParamsSeparatedByAmpersand) {
  std::string url = ApiUrlBuilder()
                        .SetBase("https://example.com")
                        .SetEndpoint("api")
                        .AddParam("a", "1")
                        .AddParam("b", "2")
                        .AddParam("c", "3")
                        .Build();

  EXPECT_NE(url.find("a=1&b=2&c=3"), std::string::npos);
}

TEST(ApiUrlBuilder, NoParams) {
  std::string url = ApiUrlBuilder()
                        .SetBase("https://example.com")
                        .SetEndpoint("test")
                        .Build();

  EXPECT_EQ(url, "https://example.com/test/");
}

TEST(ApiUrlBuilder, StationsListEndpoint) {
  std::string url = ApiUrlBuilder()
                        .SetBase("https://api.rasp.yandex-net.ru/v3.0")
                        .SetEndpoint("stations_list")
                        .AddParam("apikey", "key123")
                        .AddParam("format", "json")
                        .AddParam("lang", "ru_RU")
                        .Build();

  EXPECT_TRUE(url.starts_with(
      "https://api.rasp.yandex-net.ru/v3.0/stations_list/"));
  EXPECT_NE(url.find("format=json"), std::string::npos);
  EXPECT_NE(url.find("lang=ru_RU"), std::string::npos);
}
