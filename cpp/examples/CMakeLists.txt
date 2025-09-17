# UMICP Examples CMake Configuration

cmake_minimum_required(VERSION 3.16)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSON REQUIRED json-c)
pkg_check_modules(ZLIB REQUIRED zlib)
pkg_check_modules(OPENSSL REQUIRED openssl)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${JSON_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
)

# Link directories
link_directories(
    ${JSON_LIBRARY_DIRS}
    ${ZLIB_LIBRARY_DIRS}
    ${OPENSSL_LIBRARY_DIRS}
)

# Basic example executable
add_executable(basic_example basic_example.cpp)

target_link_libraries(basic_example
    umicp_static
    ${JSON_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    uuid
)

# Set C++ standard
set_target_properties(basic_example PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Compiler flags
if(MSVC)
    target_compile_options(basic_example PRIVATE /W4 /permissive-)
else()
    target_compile_options(basic_example PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Installation
install(TARGETS basic_example
    RUNTIME DESTINATION bin
)
