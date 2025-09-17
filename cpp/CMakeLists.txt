# UMICP C++ Core Implementation - CMake Configuration
# Based on BIP-05 Universal Matrix Intelligent Communication Protocol

cmake_minimum_required(VERSION 3.16)
project(UMICP VERSION 1.0.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter -Werror)
endif()

# LLVM/Clang optimization flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-flto -fvectorize -fslp-vectorize)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-flto -ftree-vectorize)
endif()

# Dependencies
find_package(PkgConfig REQUIRED)

# Required packages
pkg_check_modules(JSON REQUIRED json-c)
pkg_check_modules(ZLIB REQUIRED zlib)
pkg_check_modules(OPENSSL REQUIRED openssl)

# WebSocket and HTTP/2 dependencies moved to bindings

# Optional packages
pkg_check_modules(CBOR libcbor)
pkg_check_modules(MSGPACK libmsgpack)
pkg_check_modules(LZ4 liblz4)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${JSON_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${CBOR_INCLUDE_DIRS}
    ${LZ4_INCLUDE_DIRS}
)

# Link directories
link_directories(
    ${JSON_LIBRARY_DIRS}
    ${ZLIB_LIBRARY_DIRS}
    ${OPENSSL_LIBRARY_DIRS}
    ${CBOR_LIBRARY_DIRS}
    ${LZ4_LIBRARY_DIRS}
)

# Source files
set(UMICP_SOURCES
    src/envelope.cpp
    src/frame.cpp
    src/protocol.cpp
    src/transport.cpp
    src/buffer.cpp
    src/config.cpp
    src/matrix_ops.cpp
    src/serialization.cpp
    src/security.cpp
    src/compression.cpp
    src/c_api.cpp
)

# Header files
set(UMICP_HEADERS
    include/umicp.h
    include/umicp_types.h
    include/envelope.h
    include/frame.h
    include/protocol.h
    include/transport.h
    include/buffer.h
    include/config.h
    include/matrix_ops.h
    include/serialization.h
    include/security.h
    include/compression.h
)

# Create static library
add_library(umicp_static STATIC ${UMICP_SOURCES} ${UMICP_HEADERS})
target_link_libraries(umicp_static
    ${JSON_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CBOR_LIBRARIES}
    ${LZ4_LIBRARIES}
)

# Create shared library
add_library(umicp_shared SHARED ${UMICP_SOURCES} ${UMICP_HEADERS})
target_link_libraries(umicp_shared
    ${JSON_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CBOR_LIBRARIES}
    ${LZ4_LIBRARIES}
)

# Set library properties
set_target_properties(umicp_static PROPERTIES
    OUTPUT_NAME umicp
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

set_target_properties(umicp_shared PROPERTIES
    OUTPUT_NAME umicp
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Build examples
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Examples already defined above

# Build tests
option(BUILD_TESTS "Build test programs" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)

    # Add custom test targets
    add_custom_target(test_quick
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L "quick"
        DEPENDS umicp_tests
        COMMENT "Running quick tests"
    )

    add_custom_target(test_full
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS umicp_tests
        COMMENT "Running complete test suite"
    )
endif()

# Examples moved to bindings

# Installation
install(TARGETS umicp_static umicp_shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${UMICP_HEADERS} DESTINATION include/umicp)

# Generate pkg-config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/umicp.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/umicp.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/umicp.pc
    DESTINATION lib/pkgconfig
)

# Create alias targets
add_library(UMICP::static ALIAS umicp_static)
add_library(UMICP::shared ALIAS umicp_shared)

# Export targets
export(TARGETS umicp_static umicp_shared
    FILE UMICPTargets.cmake
    NAMESPACE UMICP::
)

# Generate and install CMake package config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    UMICPConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    UMICPConfig.cmake.in
    UMICPConfig.cmake
    INSTALL_DESTINATION lib/cmake/UMICP
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/UMICPConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/UMICPConfigVersion.cmake
    DESTINATION lib/cmake/UMICP
)
