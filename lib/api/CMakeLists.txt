target_sources(lib PRIVATE
    parsers/routes/routes_parser.cpp
    parsers/stations/stations_parser.cpp
)

target_include_directories(lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})
