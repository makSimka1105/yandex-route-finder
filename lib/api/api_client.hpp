#pragma once

#include <expected>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "domain/errors.hpp"
#include "domain/models.hpp"
#include "domain/yandex_rasp.hpp"
#include "net/i_http_client.hpp"
#include "parsers/routes/routes_parser.hpp"
#include "parsers/stations/stations_parser.hpp"
#include "url_builder.hpp"

namespace route_finder {

class ApiClient {
 public:
  ApiClient(IHttpClient* http_client, const std::string& api_key,
            std::ostream* http_error_log = nullptr)
      : http_client_(http_client),
        api_key_(api_key),
        http_error_log_(http_error_log) {}

  ApiExpectedResult<std::string> FetchRoutesRaw(const SearchRequest& request) {
    ApiUrlBuilder builder = ApiUrlBuilder()
                                .SetBase(kBaseUrl)
                                .SetEndpoint("search")
                                .AddParam(kParamApiKey, api_key_)
                                .AddParam(kParamFrom, request.from_code)
                                .AddParam(kParamTo, request.to_code)
                                .AddParam(kParamFormat, "json")
                                .AddParam(kParamLang, "ru_RU");

    if (!request.date.empty()) {
      builder.AddParam(kParamDate, request.date);
    }

    builder.AddParam(kParamTransfers,
                     request.prefs.allow_transfers ? "true" : "false");

    if (!request.prefs.transport.allowed.empty()) {
      builder.AddParam(
          kParamTransportTypes,
          FormatTransportTypesQueryValue(request.prefs.transport.allowed));
    }

    std::expected<std::string, int> response =
        http_client_->Get(builder.Build());
    if (!response) {
      LogHttpFailure("search (routes)", response.error());
      return std::unexpected(ApiError::kNetworkError);
    }
    return std::move(*response);
  }

  ApiExpectedResult<std::vector<Route>> FetchRoutes(
      const SearchRequest& request) {
    ApiExpectedResult<std::string> raw = FetchRoutesRaw(request);
    if (!raw) return std::unexpected(raw.error());
    return search_parser_.Parse(std::move(*raw));
  }

  ApiExpectedResult<std::string> FetchStationsListRaw() {
    std::string url = ApiUrlBuilder()
                          .SetBase(kBaseUrl)
                          .SetEndpoint("stations_list")
                          .AddParam(kParamApiKey, api_key_)
                          .AddParam(kParamFormat, "json")
                          .AddParam(kParamLang, "ru_RU")
                          .Build();

    std::expected<std::string, int> response = http_client_->Get(url);
    if (!response) {
      LogHttpFailure("stations_list", response.error());
      return std::unexpected(ApiError::kNetworkError);
    }
    return std::move(*response);
  }

  ApiExpectedResult<std::vector<Location>> FetchStationsList() {
    ApiExpectedResult<std::string> raw = FetchStationsListRaw();
    if (!raw) return std::unexpected(raw.error());
    return stations_parser_.Parse(std::move(*raw));
  }

 private:
  void LogHttpFailure(std::string_view operation, int error_code) const {
    if (http_error_log_ == nullptr) {
      return;
    }
    *http_error_log_ << "[http] " << operation << " failed: ";
    if (error_code < 0) {
      *http_error_log_ << "no response (network error or timeout)\n";
    } else {
      *http_error_log_ << "HTTP status " << error_code;
      if (error_code == 401 || error_code == 403) {
        *http_error_log_ << " (check API key)";
      }
      *http_error_log_ << '\n';
    }
  }

  static constexpr const char* kBaseUrl = "https://api.rasp.yandex-net.ru/v3.0";

  IHttpClient* http_client_;
  std::string api_key_;
  std::ostream* http_error_log_;
  ResponseParser search_parser_;
  StationsParser stations_parser_;
};

}
