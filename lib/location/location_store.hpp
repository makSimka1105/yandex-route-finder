#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "domain/models.hpp"

namespace route_finder {

class LocationStore {
 public:
  void Build(std::vector<Location> all_locations);

  std::vector<const Location*> FindTopN(
      const std::string& search_query_utf8, size_t max_results = 20,
      std::optional<Location::LocationType> required_location_type =
          std::nullopt) const;

  const Location* FindBest(
      const std::string& search_query_utf8,
      std::optional<Location::LocationType> required_location_type =
          std::nullopt) const;

  const Location* FindByCode(const std::string& yandex_location_code) const;

  bool Empty() const { return locations_.empty(); }

 private:
  std::vector<Location> locations_;
  std::vector<std::u32string> titles_normalized_utf32_;
  std::unordered_map<std::string, size_t> location_code_to_index_;
};

}
