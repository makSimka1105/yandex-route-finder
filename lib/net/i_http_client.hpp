#pragma once

#include <expected>
#include <string>

namespace route_finder {

class IHttpClient {
 public:
  virtual ~IHttpClient() = default;

  virtual std::expected<std::string, int> Get(const std::string& url) = 0;
};

}
