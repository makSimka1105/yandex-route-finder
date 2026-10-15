# Yandex Route Finder

![C++23](https://img.shields.io/badge/C%2B%2B-23-00599C?logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.24+-064F8C?logo=cmake&logoColor=white)
![GoogleTest](https://img.shields.io/badge/tests-GoogleTest%20%2B%20gmock-4285F4)

Interactive console app that finds train, plane, bus and suburban routes between cities using the
[Yandex Schedules API](https://yandex.ru/dev/rasp/doc/ru/). It supports direct trips and trips with one
transfer, fuzzy station search with typo tolerance, and a disk cache that keeps the app within the
API limit of 500 requests per day.

```
> from санкт-петербург
> to масква
> date 2026-03-24
> transport train
> search
```

## Features

- **Fuzzy location search.** Station and city names are matched with a Unicode-aware Bitap
  (shift-or) algorithm. The allowed number of typos grows with query length (up to 3 insertions,
  deletions or substitutions), so `масква` still resolves to Москва. Matches are ranked by error
  count and title length, and the best one is picked automatically.
- **Two-level disk cache.** The station directory is cached for 7 days. Route searches go to an LRU
  cache with TTL (20 entries, 24 h) that is rebuilt from the cache directory on startup, so it
  survives restarts.
- **Failure tolerant.** Network, parsing and API errors travel as `std::expected<T, ApiError>`
  instead of exceptions. A bad response or a dropped connection prints an error and the session
  continues.
- **Search preferences.** Filter by transport type, enable or disable transfers, pick a date
  (defaults to today).

## Architecture

```
lib/
  net/        IHttpClient interface + cpr implementation
  api/        ApiClient, URL builder, JSON parsers for stations and routes
  cache/      stations cache, LRU routes cache, CacheManager facade
  bitap/      fuzzy matcher and UTF-8 / UTF-32 helpers
  location/   LocationStore: index by code, top-N fuzzy lookup
  core/       Application: sync stations, search, apply preferences
  cli/        command handlers
bin/          entry point
tests/        unit tests with a mocked HTTP client and recorded JSON responses
```

Patterns worth a look:

- **Chain of Responsibility in the CLI.** Each command (`from`, `to`, `date`, `search`, ...) is a
  separate `ICliHandler`. A handler either handles the input or passes it on, so a new command means
  a new class and no changes in a central `switch`.
- **Chain of field handlers in the parsers.** JSON parsing is split into small `IFieldHandler<T>`
  objects, each filling one part of a model. The parsers stay flat and every handler can be tested
  on its own.
- **Dependency inversion for HTTP.** `ApiClient` depends on `IHttpClient`. Production code uses
  libcpr, tests use a gmock client that replays recorded API responses, so the whole search flow is
  tested without network access.

## Build

Requirements: a C++23 compiler (GCC 14+ or Clang 18+), CMake 3.24+, OpenSSL dev headers. cpr,
nlohmann/json and GoogleTest are fetched by CMake.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Usage

Get a free API key at [yandex.ru/dev/rasp](https://yandex.ru/dev/rasp/raspapi/) and pass it as the
first argument:

```bash
./build/bin/route_finder <api_key>
```

| Command | Description |
|---|---|
| `from <code \| query>` | set departure city or station |
| `to <code \| query>` | set destination |
| `date <YYYY-MM-DD>` / `date clear` | search date, today when unset |
| `transfers on \| off \| clear` | allow routes with one transfer (on by default) |
| `transport <type>` / `transport clear` | one or more of `train`, `plane`, `bus`, `suburban`, `sea`, ... |
| `find <query>` / `find city <query>` / `find station <query>` | fuzzy lookup without searching |
| `show` | print the current request |
| `search` | run the search |
| `help`, `exit` | |

## Stack

C++23 (`std::expected`, `std::optional`, `std::u32string`), libcpr, nlohmann/json, GoogleTest and gmock,
CMake FetchContent.
