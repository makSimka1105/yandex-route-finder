#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace route_finder {

struct BitapResult {
  bool matched = false;
  int errors = 0;
};

inline BitapResult BitapSearch(const std::u32string& text,
                               const std::u32string& pattern, int max_errors) {
  if (pattern.empty()) return {true, 0};
  if (pattern.size() > 63) return {false, 0};
  max_errors = std::min(max_errors, 3);

  std::unordered_map<char32_t, uint64_t> mask;
  for (size_t i = 0; i < pattern.size(); ++i) {
    mask[pattern[i]] |= (1ULL << i);
  }

  uint64_t R[4];
  for (int d = 0; d <= max_errors; ++d) {
    R[d] = ~((1ULL << d) - 1);
  }

  uint64_t match_bit = 1ULL << (pattern.size() - 1);
  int best = max_errors + 1;

  for (char32_t c : text) {
    std::unordered_map<char32_t, uint64_t>::iterator it = mask.find(c);
    uint64_t char_mask = (it != mask.end()) ? it->second : 0;
    uint64_t prev[4];
    for (int d = 0; d <= max_errors; ++d) {
      prev[d] = R[d];
    }
    R[0] = (R[0] << 1) | ~char_mask;

    for (int d = 1; d <= max_errors; ++d) {
      uint64_t match = (prev[d] << 1) | ~char_mask;
      uint64_t sub = prev[d - 1] << 1;
      uint64_t ins = R[d - 1] << 1;
      uint64_t del = prev[d - 1];
      R[d] = match & sub & ins & del;
    }

    for (int d = 0; d <= max_errors; ++d) {
      if ((R[d] & match_bit) == 0 && d < best) {
        best = d;
      }
    }
  }

  if (best <= max_errors) {
    return {true, best};
  }
  return {false, 0};
}

}
