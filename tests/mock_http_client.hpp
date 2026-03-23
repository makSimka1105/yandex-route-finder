#pragma once

#include "net/i_http_client.hpp"

#include <expected>
#include <string>
#include <utility>

namespace route_finder {

class MockHttpClient : public IHttpClient {
 public:
  std::expected<std::string, int> response;
  std::string last_requested_url;

  void SetResponseRef(const std::string* ref) { response_ref_ = ref; }

  void SetStationsResponseRef(const std::string* ref) { stations_ref_ = ref; }

  void SetRoutesResponseRef(const std::string* ref) { routes_ref_ = ref; }

  int GetCallCount() const { return call_count_; }

  void ResetCallCount() { call_count_ = 0; }

  std::expected<std::string, int> Get(const std::string& url) override {
    ++call_count_;
    last_requested_url = url;
    if (stations_ref_ != nullptr &&
        url.find("stations_list") != std::string::npos) {
      return std::string(*stations_ref_);
    }
    if (routes_ref_ != nullptr && url.find("/search") != std::string::npos) {
      return std::string(*routes_ref_);
    }
    if (response_ref_) {
      return std::string(*response_ref_);
    }
    return std::move(response);
  }

 private:
  const std::string* response_ref_ = nullptr;
  const std::string* stations_ref_ = nullptr;
  const std::string* routes_ref_ = nullptr;
  int call_count_ = 0;
};

}
