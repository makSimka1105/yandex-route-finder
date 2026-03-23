cmake_minimum_required(VERSION 3.24)

project(
    route_finder
    LANGUAGES CXX
)

enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

#find_package(CURL REQUIRED COMPONENTS HTTP HTTPS)
#set(CPR_FORCE_USE_SYSTEM_CURL ON CACHE BOOL "Use system libcurl" FORCE)

FetchContent_Declare(cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 2b2b169)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(json)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0)
FetchContent_MakeAvailable(googletest)

add_subdirectory(lib)
add_subdirectory(bin)
add_subdirectory(tests)
