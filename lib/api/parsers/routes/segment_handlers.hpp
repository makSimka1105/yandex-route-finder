#pragma once

#include <cstdint>
#include <string>

#include "domain/models.hpp"
#include <nlohmann/json.hpp>
#include "nlohmann/json_fwd.hpp"
#include "parsers/field_handler.hpp"

namespace route_finder {

class StationExtractor : public IFieldHandler<Segment> {
 protected:
  void Extract(const nlohmann::json& json, Segment& seg) override {
    if (json.contains("from") && json["from"].is_object()) {
      const nlohmann::json& from = json["from"];
      if (from.contains("title")) {
        seg.from_station = from["title"].get<std::string>();
      }
      if (from.contains("code") && from["code"].is_string()) {
        seg.from_code = from["code"].get<std::string>();
      }
    }
    if (json.contains("to") && json["to"].is_object()) {
      const nlohmann::json& to = json["to"];
      if (to.contains("title")) {
        seg.to_station = to["title"].get<std::string>();
      }
      if (to.contains("code") && to["code"].is_string()) {
        seg.to_code = to["code"].get<std::string>();
      }
    }
  }
};

class ThreadExtractor : public IFieldHandler<Segment> {
 protected:
  void Extract(const nlohmann::json& json, Segment& seg) override {
    if (!json.contains("thread")) {
      return;
    }
    const nlohmann::json& thread = json["thread"];
    if (thread.contains("transport_type")) {
      seg.transport_type = thread["transport_type"].get<std::string>();
    }
    if (thread.contains("number") && !thread["number"].is_null()) {
      const nlohmann::json& num = thread["number"];
      if (num.is_string()) {
        seg.thread_number = num.get<std::string>();
      } else if (num.is_number_integer()) {
        seg.thread_number = std::to_string(num.get<std::int64_t>());
      }
    }
    if (thread.contains("carrier") && thread["carrier"].contains("title")) {
      seg.carrier = thread["carrier"]["title"].get<std::string>();
    }
  }
};

class TimeExtractor : public IFieldHandler<Segment> {
 protected:
  void Extract(const nlohmann::json& json, Segment& seg) override {
    if (json.contains("departure") && !json["departure"].is_null()) {
      seg.departure = json["departure"].get<std::string>();
    }
    if (json.contains("arrival") && !json["arrival"].is_null()) {
      seg.arrival = json["arrival"].get<std::string>();
    }
  }
};

}
