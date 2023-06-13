cmake_minimum_required(VERSION 3.10.0)
project(bro-code)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log_setup log regex)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Update name and srcs
add_library(server_lib src/server.cc)
add_library(session_lib src/session.cc)
add_library(config_parser_lib src/config_parser.cc)
add_library(log_lib src/log.cc)
add_library(handler_lib src/RequestHandler.cc)
add_library(echo_request_lib src/EchoRequestHandler.cc)
add_library(static_request_lib src/StaticRequestHandler.cc)
add_library(error_request_lib src/ErrorRequestHandler.cc)
add_library(api_request_lib src/ApiRequestHandler.cc)
add_library(block_request_lib src/BlockRequestHandler.cc)
add_library(health_request_lib src/HealthRequestHandler.cc)
add_library(board_request_lib src/BoardRequestHandler.cc)
add_library(mime_lib src/MIME.cc)
add_library(echo_handler_factory_lib src/EchoHandlerFactory.cc)
add_library(static_handler_factory_lib src/StaticHandlerFactory.cc)
add_library(error_handler_factory_lib src/ErrorHandlerFactory.cc)
add_library(api_handler_factory_lib src/ApiHandlerFactory.cc)
add_library(block_handler_factory_lib src/BlockHandlerFactory.cc)
add_library(health_handler_factory_lib src/HealthHandlerFactory.cc)
add_library(board_handler_factory_lib src/BoardHandlerFactory.cc)
add_library(http_parser_lib src/HttpParser.cc)

# Update executable name, srcs, and deps
add_executable(server src/server_main.cc)
target_link_libraries(handler_lib echo_request_lib static_request_lib error_request_lib api_request_lib block_request_lib health_request_lib board_request_lib)
target_link_libraries(static_request_lib handler_lib Boost::filesystem mime_lib)
target_link_libraries(echo_request_lib handler_lib)
target_link_libraries(error_request_lib handler_lib)
target_link_libraries(api_request_lib handler_lib Boost::filesystem mime_lib)
target_link_libraries(block_request_lib handler_lib)
target_link_libraries(health_request_lib handler_lib)
target_link_libraries(board_request_lib handler_lib)
target_link_libraries(session_lib http_parser_lib)
target_link_libraries(server server_lib session_lib Boost::system pthread config_parser_lib echo_request_lib static_request_lib error_request_lib api_request_lib health_request_lib board_request_lib echo_handler_factory_lib static_handler_factory_lib error_handler_factory_lib api_handler_factory_lib block_handler_factory_lib health_handler_factory_lib board_handler_factory_lib log_lib Boost::regex Boost::log_setup Boost::log)
target_link_libraries(mime_lib)

# Add config parser tests
add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test config_parser_lib gtest_main log_lib Boost::regex Boost::log_setup Boost::log)
add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test server_lib session_lib echo_request_lib static_request_lib error_request_lib api_request_lib block_request_lib health_request_lib board_request_lib echo_handler_factory_lib static_handler_factory_lib error_handler_factory_lib api_handler_factory_lib block_handler_factory_lib health_handler_factory_lib board_handler_factory_lib config_parser_lib gtest_main log_lib Boost::regex Boost::log_setup Boost::log)
add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test session_lib echo_request_lib static_request_lib error_request_lib health_request_lib config_parser_lib gtest_main gmock_main log_lib Boost::regex Boost::log_setup Boost::log)
add_executable(echo_handler_test tests/echo_handler_test.cc)
target_link_libraries(echo_handler_test echo_request_lib gtest_main log_lib Boost::regex Boost::log_setup Boost::log)
add_executable(static_handler_test tests/static_handler_test.cc)
target_link_libraries(static_handler_test static_request_lib config_parser_lib gtest_main log_lib Boost::regex Boost::log_setup Boost::log mime_lib)
add_executable(error_handler_test tests/error_handler_test.cc)
target_link_libraries(error_handler_test error_request_lib gtest_main log_lib Boost::regex Boost::log_setup Boost::log)
add_executable(api_handler_test tests/api_handler_test.cc)
target_link_libraries(api_handler_test api_request_lib config_parser_lib gtest_main log_lib Boost::regex Boost::log_setup Boost::log)
add_executable(block_handler_test tests/block_handler_test.cc)
target_link_libraries(block_handler_test block_request_lib config_parser_lib gtest_main log_lib Boost::regex Boost::log_setup Boost::log mime_lib)
add_executable(health_handler_test tests/health_handler_test.cc)
target_link_libraries(health_handler_test health_request_lib gtest_main log_lib Boost::regex Boost::log_setup Boost::log)
add_executable(board_handler_test tests/board_handler_test.cc)
target_link_libraries(board_handler_test board_request_lib config_parser_lib gtest_main log_lib Boost::regex Boost::log_setup Boost::log)
add_executable(mime_test tests/MIME_test.cc)
target_link_libraries(mime_test mime_lib gtest_main)
add_executable(http_parser_test tests/http_parser_test.cc)  
target_link_libraries(http_parser_test http_parser_lib gtest_main log_lib Boost::regex Boost::log_setup Boost::log)

gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(echo_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(static_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(error_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(api_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(block_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(health_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(board_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(mime_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(http_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)


if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Coverage")
    add_test(NAME integration_test COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_test.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

# generate_coverage_report(TARGETS example_server example_lib TESTS example_lib_test)
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS config_parser_lib server_lib session_lib server handler_lib echo_request_lib static_request_lib error_request_lib api_request_lib block_request_lib health_request_lib board_request_lib mime_lib http_parser_lib TESTS config_parser_test server_test session_test echo_handler_test static_handler_test error_handler_test api_handler_test block_handler_test health_handler_test board_handler_test mime_test http_parser_test)
