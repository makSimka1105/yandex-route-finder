#include <gtest/gtest.h>

#include "domain/errors.hpp"

using namespace route_finder;

TEST(ApiErrors, Messages) {
  EXPECT_STREQ(ApiErrorMessage(ApiError::kNetworkError), "network error");
  EXPECT_STREQ(ApiErrorMessage(ApiError::kParseError), "parse error");
  EXPECT_STREQ(ApiErrorMessage(ApiError::kInvalidResponse), "invalid response");
}

TEST(ApiErrors, UnknownEnumUsesDefault) {
  EXPECT_STREQ(ApiErrorMessage(static_cast<ApiError>(999)), "unknown error");
}
