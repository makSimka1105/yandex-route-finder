#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "domain/errors.hpp"
#include "domain/models.hpp"
#include "nlohmann/json_fwd.hpp"
#include "parsers/field_handler.hpp"

namespace route_finder {
class ResponseParser {
 public:
  ResponseParser();
  ~ResponseParser() = default;

  ApiExpectedResult<std::vector<Route>> Parse(std::string raw_json);

 private:
  ApiExpectedResult<nlohmann::json> TryParseJson(std::string raw);
  std::vector<Route> ExtractRoutes(const nlohmann::json& root);
  void AppendBranchesFromDetails(const nlohmann::json& item, Route& route);

  std::unique_ptr<IFieldHandler<Segment>> segment_chain_;
};
}
