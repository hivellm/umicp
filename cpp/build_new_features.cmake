# Simplified CMake for building and testing new features (Phases 1-3)
# This avoids complex dependencies and focuses on new code

cmake_minimum_required(VERSION 3.16)
project(UMICP_NEW_FEATURES VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files for new features
set(NEW_FEATURE_SOURCES
    src/event_emitter.cpp
    src/multiplexed_peer.cpp
    src/websocket_server.cpp
    src/websocket_client.cpp
    src/envelope.cpp
    src/buffer.cpp
    src/serialization.cpp
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Create library for new features
add_library(umicp_new_features STATIC ${NEW_FEATURE_SOURCES})

# Find GoogleTest
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50331fc36141fd20fd08fa9b13fb1e.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    set(GTEST_LIBRARIES gtest)
    set(GTEST_MAIN_LIBRARIES gtest_main)
else()
    set(GTEST_LIBRARIES GTest::gtest)
    set(GTEST_MAIN_LIBRARIES GTest::gtest_main)
endif()

# Test executables
add_executable(event_emitter_tests tests/test_event_emitter.cpp)
target_link_libraries(event_emitter_tests umicp_new_features ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})

add_executable(multiplexed_peer_tests tests/test_multiplexed_peer.cpp)
target_link_libraries(multiplexed_peer_tests umicp_new_features ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})

add_executable(websocket_tests tests/test_websocket_components.cpp)
target_link_libraries(websocket_tests umicp_new_features ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})

# Enable testing
enable_testing()

add_test(NAME EventEmitterTests COMMAND event_emitter_tests)
add_test(NAME MultiplexedPeerTests COMMAND multiplexed_peer_tests)
add_test(NAME WebSocketTests COMMAND websocket_tests)

