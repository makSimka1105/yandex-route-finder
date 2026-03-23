target_sources(lib PRIVATE
    stations_cache.cpp
    routes_cache.cpp
)

target_include_directories(lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})
