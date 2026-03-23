#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "domain/models.hpp"
#include "bitap/bitap.hpp"
#include "bitap/unicode_utils.hpp"
#include "location/location_store.hpp"

using namespace route_finder;

TEST(Bitap, ExactSubstring) {
  std::u32string text = ToLower(DecodeUtf8("Московский вокзал"));
  std::u32string pattern = ToLower(DecodeUtf8("вокзал"));
  BitapResult r = BitapSearch(text, pattern, 0);
  EXPECT_TRUE(r.matched);
  EXPECT_EQ(r.errors, 0);
}

TEST(Bitap, ExactFullMatch) {
  std::u32string text = ToLower(DecodeUtf8("Псков"));
  std::u32string pattern = ToLower(DecodeUtf8("псков"));
  BitapResult r = BitapSearch(text, pattern, 0);
  EXPECT_TRUE(r.matched);
  EXPECT_EQ(r.errors, 0);
}

TEST(Bitap, FuzzySubstring) {
  std::u32string text = ToLower(DecodeUtf8("Московский вокзал"));
  std::u32string pattern = ToLower(DecodeUtf8("вакзал"));
  BitapResult r = BitapSearch(text, pattern, 1);
  EXPECT_TRUE(r.matched);
  EXPECT_EQ(r.errors, 1);
}

TEST(Bitap, FuzzyFullMatch) {
  std::u32string text = ToLower(DecodeUtf8("Санкт-Петербург"));
  std::u32string pattern = ToLower(DecodeUtf8("сакт-питербург"));
  BitapResult r = BitapSearch(text, pattern, 2);
  EXPECT_TRUE(r.matched);
  EXPECT_LE(r.errors, 2);
}

TEST(Bitap, NoMatch) {
  std::u32string text = ToLower(DecodeUtf8("Псков"));
  std::u32string pattern = ToLower(DecodeUtf8("москва"));
  BitapResult r = BitapSearch(text, pattern, 1);
  EXPECT_FALSE(r.matched);
}

TEST(Bitap, EmptyPattern) {
  std::u32string text = ToLower(DecodeUtf8("Псков"));
  std::u32string empty;
  BitapResult r = BitapSearch(text, empty, 2);
  EXPECT_TRUE(r.matched);
  EXPECT_EQ(r.errors, 0);
}


class LocationStoreTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    std::vector<Location> data;
    data.push_back({"Санкт-Петербург", "c2", Location::kCity, "", {}, {}});
    data.push_back({"Московский вокзал", "s1", Location::kStation,
                    "train_station", "train", "Санкт-Петербург"});
    data.push_back({"Пулково", "s2", Location::kStation, "airport", "plane",
                    "Санкт-Петербург"});
    data.push_back({"Псков", "c150", Location::kCity, "", {}, {}});
    data.push_back({"Псков-Пассажирский", "s3", Location::kStation, "station",
                    "train", "Псков"});
    data.push_back({"Москва", "c213", Location::kCity, "", {}, {}});
    data.push_back({"Великие Луки", "c959", Location::kCity, "", {}, {}});
    store_.Build(std::move(data));
  }

  inline static LocationStore store_;
};

TEST_F(LocationStoreTest, FindByCodeExact) {
  const Location* loc = store_.FindByCode("c2");
  ASSERT_NE(loc, nullptr);
  EXPECT_EQ(loc->title, "Санкт-Петербург");
}

TEST_F(LocationStoreTest, FindByCodeMissing) {
  EXPECT_EQ(store_.FindByCode("unknown"), nullptr);
}

TEST_F(LocationStoreTest, FindBestExact) {
  const Location* loc = store_.FindBest("Псков");
  ASSERT_NE(loc, nullptr);
  EXPECT_EQ(loc->code, "c150");
}

TEST_F(LocationStoreTest, FindBestFuzzy) {
  const Location* loc = store_.FindBest("сакт-питербург");
  ASSERT_NE(loc, nullptr);
  EXPECT_EQ(loc->code, "c2");
}

TEST_F(LocationStoreTest, FindTopSubstring) {
  std::vector<const Location*> results = store_.FindTopN("вокзал", 5);
  ASSERT_FALSE(results.empty());
  EXPECT_EQ(results[0]->title, "Московский вокзал");
}

TEST_F(LocationStoreTest, FindTopFuzzySubstring) {
  std::vector<const Location*> results = store_.FindTopN("вакзал", 5);
  ASSERT_FALSE(results.empty());
  EXPECT_EQ(results[0]->title, "Московский вокзал");
}

TEST_F(LocationStoreTest, FilterByType) {
  std::vector<const Location*> cities =
      store_.FindTopN("пск", 10, Location::kCity);
  for (const auto* loc : cities) {
    EXPECT_EQ(loc->location_type, Location::kCity);
  }
}

TEST_F(LocationStoreTest, FindTopReturnsLimitedResults) {
  std::vector<const Location*> results = store_.FindTopN("п", 2);
  EXPECT_LE(results.size(), 2u);
}
