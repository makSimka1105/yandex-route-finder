#pragma once

#include <string>

#include "domain/models.hpp"
#include "nlohmann/json_fwd.hpp"
#include "parsers/field_handler.hpp"

namespace route_finder {

class TitleExtractor : public IFieldHandler<Location> {
 protected:
  void Extract(const nlohmann::json& json, Location& location) override {
    if (json.contains("title") && json["title"].is_string()) {
      location.title = json["title"].get<std::string>();
    }
  }
};

class CodeExtractor : public IFieldHandler<Location> {
 protected:
  void Extract(const nlohmann::json& json, Location& location) override {
    if (json.contains("codes") && json["codes"].contains("yandex_code")) {
      location.code = json["codes"]["yandex_code"].get<std::string>();
    }
  }
};

class TransportExtractor : public IFieldHandler<Location> {
 protected:
  void Extract(const nlohmann::json& json, Location& location) override {
    if (json.contains("transport_type")) {
      location.transport_type = json["transport_type"].get<std::string>();
    }
  }
};

class StationTypeExtractor : public IFieldHandler<Location> {
 protected:
  void Extract(const nlohmann::json& json, Location& location) override {
    if (json.contains("station_type")) {
      location.station_type = json["station_type"].get<std::string>();
    }
  }
};

}
