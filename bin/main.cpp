#include <iostream>
#include <string>

#include "api/api_client.hpp"
#include "cache/cache_manager.hpp"
#include "cli_chain.hpp"
#include "core/application.hpp"
#include "domain/errors.hpp"
#include "location/location_store.hpp"
#include "net/cpr_http_client.hpp"

int main(int argc, char** argv) {
  using namespace route_finder;

  if (argc < 2) {
    std::cerr << "usage: route_finder <api_key>\n";
    return 1;
  }

  std::string api_key(argv[1]);

  CprHttpClient http;
  ApiClient api(&http, api_key, &std::cerr);
  CacheManager cache;
  LocationStore locations;
  Application app(api, cache, locations);

  ApiExpectedResult<void> synced = app.SyncStations();
  if (!synced) {
    std::cerr << "stations sync failed: " << ApiErrorMessage(synced.error()) << '\n';
    return 1;
  }

  CliChain chain(app, std::cout, std::cerr);
  return chain.Run();
}
