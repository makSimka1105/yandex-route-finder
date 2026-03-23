#include "stations_cache.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <ios>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>

namespace route_finder {


bool FileStationsCache::IsExpired(std::filesystem::file_time_type mtime,
                                  std::chrono::seconds ttl) {
  std::chrono::seconds age = std::chrono::duration_cast<std::chrono::seconds>(
      std::filesystem::file_time_type::clock::now() - mtime);
  return age > ttl;
}

std::optional<std::string> FileStationsCache::Get() const {
  if (!std::filesystem::exists(stations_file_)) {
    return std::nullopt;
  }
  std::filesystem::file_time_type mtime =
      std::filesystem::last_write_time(stations_file_);
  if (IsExpired(mtime, ttl_)) {
    return std::nullopt;
  }
  std::ifstream file(stations_file_, std::ios::binary);
  if (!file.is_open()) {
    return std::nullopt;
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

void FileStationsCache::Put(const std::string& json) {
  std::filesystem::path tmp = stations_file_;
  tmp += ".tmp";
  {
    std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
      return;
    }
    out << json;
  }
  std::error_code ec;
  std::filesystem::rename(tmp, stations_file_, ec);
  if (ec) {
    std::filesystem::remove(tmp, ec);
  }
  
}

}
