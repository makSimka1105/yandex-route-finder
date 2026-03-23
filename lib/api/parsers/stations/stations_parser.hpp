#pragma once

#include <memory>
#include <string>
#include <vector>

#include "domain/errors.hpp"
#include "domain/models.hpp"
#include "nlohmann/json_fwd.hpp"
#include "parsers/field_handler.hpp"

namespace route_finder {

class StationsParser {
 public:
  StationsParser();
  ~StationsParser() = default;

  ApiExpectedResult<std::vector<Location>> Parse(std::string raw_json);

 private:
  ApiExpectedResult<nlohmann::json> TryParseJson(std::string raw);
  std::vector<Location> ExtractLocations(const nlohmann::json& root);

  std::unique_ptr<IFieldHandler<Location>> location_chain_;
};

}