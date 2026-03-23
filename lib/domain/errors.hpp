#pragma once

#include <expected>

namespace route_finder {

enum class ApiError {
  kNetworkError,
  kParseError,
  kInvalidResponse,
};
template <typename T>
using ApiExpectedResult = std::expected<T, ApiError>;

inline const char* ApiErrorMessage(ApiError e) {
  switch (e) {
    case ApiError::kNetworkError:
      return "network error";
    case ApiError::kParseError:
      return "parse error";
    case ApiError::kInvalidResponse:
      return "invalid response";
  }
  return "unknown error";
}

}