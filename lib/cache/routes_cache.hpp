#pragma once

#include <chrono>
#include <filesystem>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>

#include "domain/models.hpp"

namespace route_finder {


class FileRoutesCache {
 public:
  explicit FileRoutesCache(std::filesystem::path routes_dir,
                           size_t max_entries = 20,
                           std::chrono::seconds ttl = std::chrono::hours(24));

  std::optional<std::string> Get(const SearchRequest& req);
  void Put(const SearchRequest& req, const std::string& raw_json);

 private:
  struct FileSlot {
    std::filesystem::path path;
    std::filesystem::file_time_type mtime{};
  };

  using LruList = std::list<std::pair<std::string, FileSlot>>;
  using LruIterator = LruList::iterator;

  static std::string FileStemFromRequest(const SearchRequest& req);

  bool MtimeIsExpired(std::filesystem::file_time_type mtime) const;

  void RemoveExpiredEntries();
  void MoveToFront(LruIterator it);
  void EvictTail();
  void UnlinkNode(LruIterator it);

  void LoadFromDirectory();

  std::filesystem::path routes_dir_;
  size_t max_entries_;
  std::chrono::seconds ttl_;

  LruList lru_order_;
  std::unordered_map<std::string, LruIterator> node_by_stem_;
};

}
