#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "api/api_client.hpp"
#include "cache/cache_manager.hpp"
#include "cli/cli_chain.hpp"
#include "cli/cli_handler.hpp"
#include "cli/handlers/exit_handler.hpp"
#include "cli/handlers/help_handler.hpp"
#include "core/application.hpp"
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

TEST(CliHandler, HelpPassesUnmatchedToNext) {
  std::ostringstream out;
  HelpHandler help(out);
  help.SetNext(std::make_unique<ExitHandler>());
  EXPECT_EQ(help.Handle({"unknown"}), CliExitCode::kContinue);
}

TEST(CliHandler, HelpPrintsAndExitQuits) {
  std::ostringstream out;
  HelpHandler help(out);
  help.SetNext(std::make_unique<ExitHandler>());
  EXPECT_EQ(help.Handle({"help"}), CliExitCode::kContinue);
  EXPECT_NE(out.str().find("Commands:"), std::string::npos);
  EXPECT_EQ(help.Handle({"exit"}), CliExitCode::kExitOk);
}

class CliChainContext : public ::testing::Test {
 protected:
  void SetUp() override {
    stations_json_ = LoadMockJson("stations_list_response.json");
    ASSERT_FALSE(stations_json_.empty());
    routes_json_ =
        R"({"search":{"from":{"title":"A"},"to":{"title":"B"}},"segments":[]})";

    mock_.SetStationsResponseRef(&stations_json_);
    mock_.SetRoutesResponseRef(&routes_json_);

    std::ostringstream dir;
    dir << "cli_" << static_cast<const void*>(this);
    cache_root_ = std::filesystem::path(::testing::TempDir()) / dir.str();
    std::filesystem::create_directories(cache_root_);
    cache_ = std::make_unique<CacheManager>(cache_root_);

    api_ = std::make_unique<ApiClient>(&mock_, "test_key", nullptr);
    app_ = std::make_unique<Application>(*api_, *cache_, locations_);
    ASSERT_TRUE(app_->SyncStations().has_value());
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

TEST_F(CliChainContext, RunExercisesHandlers) {
  std::ostringstream out;
  std::ostringstream err;
  CliChain chain(*app_, out, err);

  std::istringstream in(
      "search\n"
      "help\n"
      "show\n"
      "from c2\n"
      "to s9602497\n"
      "date 2030-03-01\n"
      "show\n"
      "date clear\n"
      "transfers off\n"
      "transfers clear\n"
      "transport train\n"
      "transport clear\n"
      "search\n"
      "find Площадь\n"
      "find city Санкт\n"
      "find station Московский\n"
      "from zzznotfoundzzz\n"
      "transport\n"
      "date\n"
      "show bad\n"
      "exit\n");

  const int rc = chain.Run(in);

  EXPECT_EQ(rc, 0);
  EXPECT_NE(out.str().find("> "), std::string::npos);
  EXPECT_NE(err.str().find("set endpoints"), std::string::npos);
  EXPECT_NE(out.str().find("from ="), std::string::npos);
  EXPECT_NE(out.str().find("to ="), std::string::npos);
}

TEST(CliChainEmptyIndex, FindReportsEmpty) {
  MockHttpClient mock;
  std::filesystem::path root =
      std::filesystem::path(::testing::TempDir()) / "cli_empty_idx";
  std::filesystem::create_directories(root);
  CacheManager cache(root);
  ApiClient api(&mock, "k", nullptr);
  LocationStore loc;
  Application app(api, cache, loc);

  std::ostringstream out;
  std::ostringstream err;
  CliChain chain(app, out, err);
  std::istringstream in("find Москва\nexit\n");
  const int rc = chain.Run(in);

  EXPECT_EQ(rc, 0);
  EXPECT_NE(err.str().find("empty"), std::string::npos);
}

TEST(CliChainFallback, UnknownCommandPrintsHelp) {
  MockHttpClient mock;
  std::filesystem::path root =
      std::filesystem::path(::testing::TempDir()) / "cli_fb";
  std::filesystem::create_directories(root);
  CacheManager cache(root);
  ApiClient api(&mock, "k", nullptr);
  LocationStore loc;
  Application app(api, cache, loc);

  std::ostringstream out;
  std::ostringstream err;
  CliChain chain(app, out, err);
  std::istringstream in("nosuchcommand\nexit\n");
  chain.Run(in);

  EXPECT_NE(err.str().find("unknown command"), std::string::npos);
  EXPECT_NE(out.str().find("Commands:"), std::string::npos);
}
