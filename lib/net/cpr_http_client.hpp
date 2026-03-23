#pragma once

#include <cpr/cpr.h>

#include <cstdint>
#include <expected>
#include <string>
#include <utility>

#include "cpr/api.h"
#include "cpr/cprtypes.h"
#include "cpr/response.h"
#include "cpr/timeout.h"
#include "i_http_client.hpp"

namespace route_finder {

class CprHttpClient : public IHttpClient {
 public:
  CprHttpClient() = default;

  std::expected<std::string, int> Get(const std::string& url) override {
    cpr::Response response =
        cpr::Get(cpr::Url{url}, cpr::Timeout{60000});

    if (response.status_code == 0) {
      return std::unexpected(-1);
    }
    if (response.status_code != 200) {
      return std::unexpected(static_cast<int>(response.status_code));
    }
    return std::move(response.text);
  };
};

}
