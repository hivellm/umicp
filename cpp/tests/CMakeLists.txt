# UMICP Tests - CMake Configuration
# Comprehensive test suite for all UMICP components

cmake_minimum_required(VERSION 3.16)

# Find testing framework
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    # Try to find system GoogleTest
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(GTEST gtest)
        pkg_check_modules(GTEST_MAIN gtest_main)
    endif()

    # If still not found, try direct linking
    if(NOT GTEST_FOUND)
        find_library(GTEST_LIBRARY gtest)
        find_library(GTEST_MAIN_LIBRARY gtest_main)
        if(GTEST_LIBRARY AND GTEST_MAIN_LIBRARY)
            set(GTEST_LIBRARIES ${GTEST_LIBRARY})
            set(GTEST_MAIN_LIBRARIES ${GTEST_MAIN_LIBRARY})
            set(GTEST_FOUND TRUE)
        endif()
    endif()

    # Last resort: download and build GoogleTest
    if(NOT GTEST_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/03597a01ee50331fc36141fd20fd08fa9b13fb1e.zip
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        set(GTEST_LIBRARIES gtest)
        set(GTEST_MAIN_LIBRARIES gtest_main)
    endif()
else()
    set(GTEST_LIBRARIES GTest::gtest)
    set(GTEST_MAIN_LIBRARIES GTest::gtest_main)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Test sources
set(TEST_SOURCES
    test_main.cpp
    unit/test_matrix_ops.cpp
    unit/test_serialization.cpp
    unit/test_security.cpp
    unit/test_transport.cpp
    unit/test_protocol.cpp
    unit/test_matrix_ops_edge_cases.cpp
    unit/test_error_conditions.cpp
    unit/test_ssl_compression.cpp
    unit/test_concurrency_comprehensive.cpp
    unit/test_memory_management.cpp
    integration/test_s2s_transport.cpp
    integration/test_protocol_integration.cpp
    performance/test_matrix_performance.cpp
    performance/test_transport_performance.cpp
    stress/test_stress_comprehensive.cpp
    utils/test_helpers.cpp
    # New Phase 1-3 tests
    test_event_emitter.cpp
    test_multiplexed_peer.cpp
    test_websocket_components.cpp
    # New comprehensive core tests
    unit/test_envelope_comprehensive.cpp
    unit/test_frame_comprehensive.cpp
    unit/test_buffer_comprehensive.cpp
    unit/test_compression_comprehensive.cpp
    unit/test_config_comprehensive.cpp
    unit/test_schema_registry_comprehensive.cpp
    # HTTP transport tests
    unit/test_http_client.cpp
    unit/test_http_server.cpp
    integration/test_http_integration.cpp
    # Service Discovery and Connection Pool tests
    unit/test_service_discovery.cpp
    unit/test_connection_pool.cpp
)

# Create test executable
add_executable(umicp_tests ${TEST_SOURCES})

# Link libraries
target_link_libraries(umicp_tests
    umicp_static
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    ${JSON_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CURL_LIBRARIES}
    ${CBOR_LIBRARIES}
    pthread
)

# Compiler flags for tests
target_compile_options(umicp_tests PRIVATE
    -Wall -Wextra -Wpedantic
    -DUMICP_TESTING
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(umicp_tests)

# Individual test targets for development
add_executable(matrix_tests unit/test_matrix_ops.cpp utils/test_helpers.cpp test_main.cpp)
target_link_libraries(matrix_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(transport_tests unit/test_transport.cpp integration/test_s2s_transport.cpp utils/test_helpers.cpp test_main.cpp)
target_link_libraries(transport_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(protocol_tests unit/test_protocol.cpp utils/test_helpers.cpp test_main.cpp)
target_link_libraries(protocol_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

# Edge case tests
add_executable(edge_case_tests unit/test_matrix_ops_edge_cases.cpp utils/test_helpers.cpp test_main.cpp)
target_link_libraries(edge_case_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

# Error condition tests
add_executable(error_tests unit/test_error_conditions.cpp utils/test_helpers.cpp test_main.cpp)
target_link_libraries(error_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

# Concurrency tests
add_executable(concurrency_tests unit/test_concurrency_comprehensive.cpp utils/test_helpers.cpp test_main.cpp)
target_link_libraries(concurrency_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

# Memory management tests
add_executable(memory_tests unit/test_memory_management.cpp utils/test_helpers.cpp test_main.cpp)
target_link_libraries(memory_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

# Stress tests
add_executable(stress_tests stress/test_stress_comprehensive.cpp utils/test_helpers.cpp test_main.cpp)
target_link_libraries(stress_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

# Performance tests (separate executable)
add_executable(performance_tests
    performance/test_matrix_performance.cpp
    performance/test_transport_performance.cpp
    utils/test_helpers.cpp
    test_main.cpp
)
target_link_libraries(performance_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

# Test data directory (create if needed)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Custom test targets
add_custom_target(test_unit
    COMMAND matrix_tests
    COMMAND transport_tests
    COMMAND protocol_tests
    DEPENDS matrix_tests transport_tests protocol_tests
    COMMENT "Running unit tests"
)

add_custom_target(test_edge_cases
    COMMAND edge_case_tests
    DEPENDS edge_case_tests
    COMMENT "Running edge case tests"
)

add_custom_target(test_errors
    COMMAND error_tests
    DEPENDS error_tests
    COMMENT "Running error condition tests"
)

add_custom_target(test_concurrency
    COMMAND concurrency_tests
    DEPENDS concurrency_tests
    COMMENT "Running concurrency tests"
)

add_custom_target(test_memory
    COMMAND memory_tests
    DEPENDS memory_tests
    COMMENT "Running memory management tests"
)

add_custom_target(test_stress
    COMMAND stress_tests
    DEPENDS stress_tests
    COMMENT "Running stress tests"
)

add_custom_target(test_performance
    COMMAND performance_tests
    DEPENDS performance_tests
    COMMENT "Running performance benchmarks"
)

add_custom_target(test_comprehensive
    COMMAND edge_case_tests
    COMMAND error_tests
    COMMAND concurrency_tests
    COMMAND memory_tests
    COMMAND stress_tests
    DEPENDS edge_case_tests error_tests concurrency_tests memory_tests stress_tests
    COMMENT "Running comprehensive test suite"
)

add_custom_target(test_all
    COMMAND umicp_tests
    DEPENDS umicp_tests
    COMMENT "Running complete test suite"
)

# New Phase 1-3 test targets
add_executable(event_emitter_tests test_event_emitter.cpp test_main.cpp)
target_link_libraries(event_emitter_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(multiplexed_peer_tests test_multiplexed_peer.cpp)
target_link_libraries(multiplexed_peer_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread websockets uuid)

add_executable(websocket_tests test_websocket_components.cpp)
target_link_libraries(websocket_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread websockets uuid)

# New comprehensive test targets
add_executable(envelope_comprehensive_tests unit/test_envelope_comprehensive.cpp test_main.cpp)
target_link_libraries(envelope_comprehensive_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(frame_comprehensive_tests unit/test_frame_comprehensive.cpp test_main.cpp)
target_link_libraries(frame_comprehensive_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(buffer_comprehensive_tests unit/test_buffer_comprehensive.cpp test_main.cpp)
target_link_libraries(buffer_comprehensive_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(compression_comprehensive_tests unit/test_compression_comprehensive.cpp test_main.cpp)
target_link_libraries(compression_comprehensive_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread ${ZLIB_LIBRARIES})

add_executable(compression_simple_tests unit/test_compression_simple.cpp)
target_link_libraries(compression_simple_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread ${ZLIB_LIBRARIES})

add_executable(config_comprehensive_tests unit/test_config_comprehensive.cpp test_main.cpp)
target_link_libraries(config_comprehensive_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread ${JSON_LIBRARIES})

add_executable(schema_comprehensive_tests unit/test_schema_registry_comprehensive.cpp test_main.cpp)
target_link_libraries(schema_comprehensive_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(compression_core_tests unit/test_compression_core.cpp test_main.cpp)
target_link_libraries(compression_core_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread ${ZLIB_LIBRARIES})

add_executable(e2e_basic_tests integration/test_e2e_basic.cpp test_main.cpp)
target_link_libraries(e2e_basic_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread websockets uuid)

add_custom_target(test_new_features
    COMMAND event_emitter_tests
    COMMAND multiplexed_peer_tests
    COMMAND websocket_tests
    DEPENDS event_emitter_tests multiplexed_peer_tests websocket_tests
    COMMENT "Running Phase 1-3 feature tests"
)

add_custom_target(test_comprehensive_core
    COMMAND envelope_comprehensive_tests
    COMMAND frame_comprehensive_tests
    COMMAND buffer_comprehensive_tests
    COMMAND compression_comprehensive_tests
    COMMAND config_comprehensive_tests
    COMMAND schema_comprehensive_tests
    DEPENDS envelope_comprehensive_tests frame_comprehensive_tests buffer_comprehensive_tests
            compression_comprehensive_tests config_comprehensive_tests schema_comprehensive_tests
    COMMENT "Running comprehensive core tests (100% coverage)"
)

add_custom_target(test_full_suite
    COMMAND ${CMAKE_COMMAND} -E echo "Running full test suite..."
    COMMAND event_emitter_tests
    COMMAND multiplexed_peer_tests
    COMMAND websocket_tests
    COMMAND envelope_comprehensive_tests
    COMMAND frame_comprehensive_tests
    COMMAND buffer_comprehensive_tests
    COMMAND compression_comprehensive_tests
    COMMAND config_comprehensive_tests
    COMMAND schema_comprehensive_tests
    COMMAND matrix_tests
    DEPENDS event_emitter_tests multiplexed_peer_tests websocket_tests
            envelope_comprehensive_tests frame_comprehensive_tests buffer_comprehensive_tests
            compression_comprehensive_tests config_comprehensive_tests schema_comprehensive_tests
            matrix_tests
    COMMENT "Running complete test suite for 100% core coverage"
)

# HTTP transport test targets
add_executable(http_client_tests unit/test_http_client.cpp)
target_link_libraries(http_client_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} ${CURL_LIBRARIES} uuid pthread)

add_executable(http_server_tests unit/test_http_server.cpp)
target_link_libraries(http_server_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} uuid pthread)

add_executable(http_integration_tests integration/test_http_integration.cpp)
target_link_libraries(http_integration_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} ${CURL_LIBRARIES} uuid pthread)

add_custom_target(test_http
    COMMAND http_client_tests
    COMMAND http_server_tests
    COMMAND http_integration_tests
    DEPENDS http_client_tests http_server_tests http_integration_tests
    COMMENT "Running HTTP transport tests"
)

# Service Discovery and Connection Pool test targets
add_executable(service_discovery_tests unit/test_service_discovery.cpp)
target_link_libraries(service_discovery_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

add_executable(connection_pool_tests unit/test_connection_pool.cpp)
target_link_libraries(connection_pool_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} websockets uuid pthread)

add_custom_target(test_new_networking
    COMMAND service_discovery_tests
    COMMAND connection_pool_tests
    DEPENDS service_discovery_tests connection_pool_tests
    COMMENT "Running Service Discovery and Connection Pool tests"
)

# Custom Endpoint Tests (v0.2.2)
add_executable(custom_endpoint_tests test_custom_endpoint.cpp test_main.cpp)
target_link_libraries(custom_endpoint_tests umicp_static ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} ${CURL_LIBRARIES} pthread)

add_custom_target(test_custom_endpoints
    COMMAND custom_endpoint_tests
    DEPENDS custom_endpoint_tests
    COMMENT "Running Custom Endpoint tests (v0.2.2)"
)
