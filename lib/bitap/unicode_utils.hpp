#pragma once

#include <cstddef>
#include <string>

namespace route_finder {

inline std::u32string DecodeUtf8(const std::string& input) {
  std::u32string result;
  result.reserve(input.size());
  size_t i = 0;
  while (i < input.size()) {
    unsigned char byte = static_cast<unsigned char>(input[i]);
    char32_t codepoint = 0;
    size_t extra = 0;

    if (byte < 0x80) {
      codepoint = byte;
    } else if ((byte & 0xE0) == 0xC0) {
      codepoint = byte & 0x1F;
      extra = 1;
    } else if ((byte & 0xF0) == 0xE0) {
      codepoint = byte & 0x0F;
      extra = 2;
    } else if ((byte & 0xF8) == 0xF0) {
      codepoint = byte & 0x07;
      extra = 3;
    } else {
      result.push_back(U'\uFFFD');
      ++i;
      continue;
    }

    for (size_t k = 0; k < extra; ++k) {
      if (i + 1 + k >= input.size()) {
        codepoint = U'\uFFFD';
        break;
      }
      codepoint = (codepoint << 6) |
                  (static_cast<unsigned char>(input[i + 1 + k]) & 0x3F);
    }

    i += 1 + extra;
    result.push_back(codepoint);
  }
  return result;
}

inline std::string EncodeUtf8(const std::u32string& input) {
  std::string result;
  result.reserve(input.size() * 2);
  for (char32_t cp : input) {
    if (cp < 0x80) {
      result.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
      result.push_back(static_cast<char>(0xC0 | (cp >> 6)));
      result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
      result.push_back(static_cast<char>(0xE0 | (cp >> 12)));
      result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
      result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
      result.push_back(static_cast<char>(0xF0 | (cp >> 18)));
      result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
      result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
      result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
  }
  return result;
}

inline char32_t ToLowerCodepoint(char32_t c) {
  if (c >= U'A' && c <= U'Z') {
    return c + 32;
  }
  if (c >= U'\u0410' && c <= U'\u042F') {
    return c + 0x20;
  }
  if (c == U'\u0401') {
    return U'\u0451';
  }
  return c;
}

inline std::u32string ToLower(const std::u32string& input) {
  std::u32string result;
  result.reserve(input.size());
  for (char32_t c : input) {
    result.push_back(ToLowerCodepoint(c));
  }
  return result;
}

}
