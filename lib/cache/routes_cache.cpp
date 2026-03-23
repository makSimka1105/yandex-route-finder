#include "routes_cache.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <ios>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <unordered_map>
#include <vector>

#include "models.hpp"

namespace route_finder {

namespace {

struct ScannedFile {
  std::string stem;
  std::filesystem::path path;
  std::filesystem::file_time_type mtime{};
};

std::string SanitizeForFilename(const std::string& s) {
  std::string out;
  out.reserve(s.size());
  for (unsigned char ch : s) {
    if (std::isalnum(ch) != 0 || ch == '-' || ch == '_') {
      out += static_cast<char>(ch);
    } else {
      out += '_';
    }
  }
  return out.empty() ? std::string("_") : out;
}

bool ScannedFileNewerThenStemLess(const ScannedFile& a, const ScannedFile& b) {
  if (a.mtime != b.mtime) {
    return a.mtime > b.mtime;
  }
  return a.stem < b.stem;
}

}

FileRoutesCache::FileRoutesCache(std::filesystem::path routes_dir,
                                 size_t max_entries, std::chrono::seconds ttl)
    : routes_dir_(std::move(routes_dir)), max_entries_(max_entries), ttl_(ttl) {
  std::filesystem::create_directories(routes_dir_);
  LoadFromDirectory();
}

std::string FileRoutesCache::FileStemFromRequest(const SearchRequest& req) {
  std::ostringstream ss;
  ss << SanitizeForFilename(req.from_code) << '_'
     << SanitizeForFilename(req.to_code) << '_'
     << SanitizeForFilename(req.date) << '_'
     << 't' << (req.prefs.allow_transfers ? 1 : 0) << '_';

  const std::set<std::string>& allowed = req.prefs.transport.allowed;
  if (allowed.empty()) {
    ss << "all";
  } else {
    std::vector<std::string> parts(allowed.begin(), allowed.end());
    std::sort(parts.begin(), parts.end());
    for (size_t i = 0; i < parts.size(); ++i) {
      if (i > 0) {
        ss << '_';
      }
      ss << SanitizeForFilename(parts[i]);
    }
  }
  return ss.str();
}

bool FileRoutesCache::MtimeIsExpired(
    std::filesystem::file_time_type mtime) const {
  std::chrono::seconds age = std::chrono::duration_cast<std::chrono::seconds>(
      std::filesystem::file_time_type::clock::now() - mtime);
  return age > ttl_;
}

void FileRoutesCache::UnlinkNode(LruIterator it) {
  node_by_stem_.erase(it->first);
  lru_order_.erase(it);
}

void FileRoutesCache::MoveToFront(LruIterator it) {
  lru_order_.splice(lru_order_.begin(), lru_order_, it);
}

void FileRoutesCache::EvictTail() {
  if (lru_order_.empty()) {
    return;
  }
  LruIterator tail = lru_order_.end();
  --tail;
  std::error_code ec;
  std::filesystem::remove(tail->second.path, ec);
  UnlinkNode(tail);
}

void FileRoutesCache::RemoveExpiredEntries() {
  for (auto it = lru_order_.begin(); it != lru_order_.end();) {
    if (MtimeIsExpired(it->second.mtime)) {
      std::error_code ec;
      std::filesystem::remove(it->second.path, ec);
      node_by_stem_.erase(it->first);
      it = lru_order_.erase(it);
    } else {
      ++it;
    }
  }
}

void FileRoutesCache::LoadFromDirectory() {
  lru_order_.clear();
  node_by_stem_.clear();

  if (!std::filesystem::exists(routes_dir_)) {
    return;
  }

  std::vector<ScannedFile> alive;
  for (const auto& entry : std::filesystem::directory_iterator(routes_dir_)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    const std::filesystem::path& path = entry.path();
    if (path.extension() == ".tmp") {
      std::error_code ec;
      std::filesystem::remove(path, ec);
      continue;
    }
    if (path.extension() != ".json") {
      continue;
    }
    std::string stem = path.stem().string();
    std::filesystem::file_time_type mtime =
        std::filesystem::last_write_time(path);
    if (MtimeIsExpired(mtime)) {
      std::error_code ec;
      std::filesystem::remove(path, ec);
      continue;
    }
    alive.push_back(ScannedFile{std::move(stem), path, mtime});
  }

  std::sort(alive.begin(), alive.end(), ScannedFileNewerThenStemLess);

  for (auto& item : alive) {
    lru_order_.push_back(
        {std::move(item.stem), FileSlot{item.path, item.mtime}});
    LruIterator tail = lru_order_.end();
    --tail;
    node_by_stem_[tail->first] = tail;
  }
}

std::optional<std::string> FileRoutesCache::Get(const SearchRequest& req) {
  RemoveExpiredEntries();

  const std::string stem = FileStemFromRequest(req);
  std::unordered_map<std::string, LruIterator>::iterator map_it =
      node_by_stem_.find(stem);
  if (map_it == node_by_stem_.end()) {
    return std::nullopt;
  }

  LruIterator slot = map_it->second;
  if (!std::filesystem::exists(slot->second.path)) {
    UnlinkNode(slot);
    return std::nullopt;
  }

  std::filesystem::file_time_type mtime =
      std::filesystem::last_write_time(slot->second.path);
  slot->second.mtime = mtime;
  if (MtimeIsExpired(mtime)) {
    std::error_code ec;
    std::filesystem::remove(slot->second.path, ec);
    UnlinkNode(slot);
    return std::nullopt;
  }

  std::ifstream file(slot->second.path, std::ios::binary);
  if (!file.is_open()) {
    return std::nullopt;
  }
  std::ostringstream ss;
  ss << file.rdbuf();

  MoveToFront(slot);
  return ss.str();
}

void FileRoutesCache::Put(const SearchRequest& req, const std::string& raw_json) {
  RemoveExpiredEntries();

  const std::string stem = FileStemFromRequest(req);
  const std::filesystem::path final_path = routes_dir_ / (stem + ".json");
  const std::filesystem::path tmp_path = routes_dir_ / (stem + ".tmp");

  {
    std::ofstream out(tmp_path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
      return;
    }
    out << raw_json;
  }

  std::error_code ec;
  std::filesystem::rename(tmp_path, final_path, ec);
  if (ec) {
    std::filesystem::remove(tmp_path, ec);
    return;
  }

  std::filesystem::file_time_type new_mtime =
      std::filesystem::last_write_time(final_path);

  std::unordered_map<std::string, LruIterator>::iterator found =
      node_by_stem_.find(stem);
  if (found != node_by_stem_.end()) {
    LruIterator slot = found->second;
    slot->second.path = final_path;
    slot->second.mtime = new_mtime;
    MoveToFront(slot);
    return;
  }

  while (node_by_stem_.size() >= max_entries_) {
    EvictTail();
  }

  lru_order_.push_front({stem, FileSlot{final_path, new_mtime}});
  node_by_stem_[stem] = lru_order_.begin();
}

}
