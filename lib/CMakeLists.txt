add_library(lib STATIC empty.cpp)

add_subdirectory(domain)
add_subdirectory(net)
add_subdirectory(api)
add_subdirectory(bitap)
add_subdirectory(location)
add_subdirectory(cache)
add_subdirectory(core)
add_subdirectory(cli)

target_include_directories(lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${cpr_SOURCE_DIR}/include)
    
target_link_libraries(lib PUBLIC nlohmann_json::nlohmann_json cpr::cpr)