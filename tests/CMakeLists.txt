add_executable(tests
    test_models.cpp
    test_url_builder.cpp
    test_json_parser.cpp
    test_bitap_search.cpp
    test_cache.cpp
    test_application.cpp
    test_cli.cpp
    test_errors.cpp
)
target_link_libraries(tests PRIVATE lib GTest::gtest GTest::gtest_main)

target_compile_definitions(tests PRIVATE
    MOCK_JSONS="${CMAKE_CURRENT_SOURCE_DIR}/mock_jsons"
)

include(GoogleTest)
gtest_discover_tests(tests DISCOVERY_MODE POST_BUILD)

cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL cmake_language EVAL CODE "
    if(TARGET coverage AND TARGET tests)
        add_dependencies(coverage tests)
    endif()
")
