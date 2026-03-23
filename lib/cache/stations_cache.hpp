#pragma once

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>

namespace route_finder {

class FileStationsCache {
 public:
  explicit FileStationsCache(std::filesystem::path stations_file,
                             std::chrono::seconds ttl = std::chrono::hours(24 * 7))
      : stations_file_(stations_file), ttl_(ttl) {
    std::filesystem::path parent = stations_file_.parent_path();
    if (!parent.empty()) {
      std::filesystem::create_directories(parent);
    }
  }

  std::optional<std::string> Get() const;
  void Put(const std::string& json);

 private:
  static bool IsExpired(std::filesystem::file_time_type mtime,
                        std::chrono::seconds ttl);

  std::filesystem::path stations_file_;
  std::chrono::seconds ttl_;
};

}
