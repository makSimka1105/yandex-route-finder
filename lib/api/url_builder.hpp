#pragma once

#include <cstddef>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace route_finder {

class ApiUrlBuilder {
 public:
  ApiUrlBuilder& SetBase(const std::string& base) {
    base_ = base;
    return *this;
  };
  ApiUrlBuilder& SetEndpoint(const std::string& endpoint) {
    endpoint_ = endpoint;
    return *this;
  };
  ApiUrlBuilder& AddParam(const std::string& key, const std::string& value) {
    params_.emplace_back(key, value);
    return *this;
  };
  std::string Build() const {
    std::string url = base_;
    if (!endpoint_.empty()) {
      if (!url.empty() && url.back() != '/') {
        url += '/';
      }
      url += endpoint_ + '/';
    }

    if (!params_.empty()) {
      url += '?';
      for (size_t i = 0; i < params_.size(); ++i) {
        if (i > 0) {
          url += '&';
        }
        url += params_[i].first + '=' + params_[i].second;
      }
    }

    return url;
  };

 private:
  std::string base_;
  std::string endpoint_;
  std::vector<std::pair<std::string, std::string>> params_;
};

inline std::string FormatTransportTypesQueryValue(
    const std::set<std::string>& allowed) {
  std::ostringstream oss;
  bool first = true;
  for (const std::string& t : allowed) {
    if (!first) {
      oss << ',';
    }
    first = false;
    oss << t;
  }
  return oss.str();
}

}
