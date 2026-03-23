#include "location_store.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "domain/models.hpp"
#include "bitap/bitap.hpp"
#include "bitap/unicode_utils.hpp"

namespace route_finder {
namespace {

struct FuzzyMatchRanking {
  size_t location_index;
  int typos_count;
  size_t normalized_title_length;

  friend bool operator<(const FuzzyMatchRanking& left,
                        const FuzzyMatchRanking& right) {
    if (left.typos_count != right.typos_count) {
      return left.typos_count < right.typos_count;
    }
    return left.normalized_title_length < right.normalized_title_length;
  }
};

int MaxAllowedTyposForQueryLength(std::size_t query_char_count) {
  if (query_char_count <= 3) {
    return 1;
  }
  if (query_char_count <= 7) {
    return 2;
  }
  return 4;
}

bool PassesLocationTypeFilter(
    const Location& location,
    std::optional<Location::LocationType> required_location_type) {
  return !required_location_type ||
         location.location_type == *required_location_type;
}

}

void LocationStore::Build(std::vector<Location> all_locations) {
  locations_ = std::move(all_locations);
  titles_normalized_utf32_.clear();
  titles_normalized_utf32_.reserve(locations_.size());
  location_code_to_index_.clear();
  location_code_to_index_.reserve(locations_.size());

  for (size_t i = 0; i < locations_.size(); ++i) {
    titles_normalized_utf32_.push_back(
        ToLower(DecodeUtf8(locations_[i].title)));
    location_code_to_index_[locations_[i].code] = i;
  }
}

const Location* LocationStore::FindByCode(
    const std::string& yandex_location_code) const {
  std::unordered_map<std::string, size_t>::const_iterator iterator =
      location_code_to_index_.find(yandex_location_code);
  if (iterator != location_code_to_index_.end()) {
    return &locations_[iterator->second];
  }
  return nullptr;
}

std::vector<const Location*> LocationStore::FindTopN(
    const std::string& search_query_utf8, size_t max_results,
    std::optional<Location::LocationType> required_location_type) const {
  std::u32string query_normalized = ToLower(DecodeUtf8(search_query_utf8));
  if (query_normalized.empty()) {
    return {};
  }

  const int max_types =
      MaxAllowedTyposForQueryLength(query_normalized.size());

  std::vector<FuzzyMatchRanking> ranked_matches;

  for (size_t i = 0; i < locations_.size(); ++i) {
    if (!PassesLocationTypeFilter(locations_[i], required_location_type)) {
      continue;
    }

    BitapResult substring_match =
        BitapSearch(titles_normalized_utf32_[i], query_normalized, max_types);
    if (substring_match.matched) {
      ranked_matches.push_back(FuzzyMatchRanking{
          i,
          substring_match.errors,
          titles_normalized_utf32_[i].size(),
      });
    }
  }

  std::sort(ranked_matches.begin(), ranked_matches.end());

  std::vector<const Location*> matches;
  const size_t result_count = std::min(max_results, ranked_matches.size());
  matches.reserve(result_count);
  for (size_t rank = 0; rank < result_count; ++rank) {
    matches.push_back(&locations_[ranked_matches[rank].location_index]);
  }
  return matches;
}

const Location* LocationStore::FindBest(
    const std::string& search_query_utf8,
    std::optional<Location::LocationType> required_location_type) const {
  std::vector<const Location*> best_matches =
      FindTopN(search_query_utf8, 1, required_location_type);
  return best_matches.empty() ? nullptr : best_matches.front();
}

}
