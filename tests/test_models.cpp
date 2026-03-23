#include <gtest/gtest.h>
#include "domain/models.hpp"

using namespace route_finder;

TEST(TransportPreferences, EmptyAllowedAcceptsEverything) {
    TransportPreferences prefs;
    EXPECT_TRUE(prefs.IsAllowed("bus"));
    EXPECT_TRUE(prefs.IsAllowed("train"));
    EXPECT_TRUE(prefs.IsAllowed("plane"));
}

TEST(TransportPreferences, NonEmptyAllowedFilters) {
    TransportPreferences prefs;
    prefs.allowed = {"bus"};
    EXPECT_TRUE(prefs.IsAllowed("bus"));
    EXPECT_FALSE(prefs.IsAllowed("train"));
}