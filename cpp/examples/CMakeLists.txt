# UMICP Examples CMake Configuration

cmake_minimum_required(VERSION 3.16)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSON REQUIRED json-c)
pkg_check_modules(ZLIB REQUIRED zlib)
pkg_check_modules(OPENSSL REQUIRED openssl)
pkg_check_modules(CURL REQUIRED libcurl)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${JSON_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

# Link directories
link_directories(
    ${JSON_LIBRARY_DIRS}
    ${ZLIB_LIBRARY_DIRS}
    ${OPENSSL_LIBRARY_DIRS}
    ${CURL_LIBRARY_DIRS}
)

# Helper function to create example executable
function(add_umicp_example target_name source_file)
    add_executable(${target_name} ${source_file})

    target_link_libraries(${target_name}
        umicp_static
        ${JSON_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${CURL_LIBRARIES}
        uuid
        websockets  # libwebsockets
    )

    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)
    endif()

    install(TARGETS ${target_name} RUNTIME DESTINATION bin)
endfunction()

# Create examples
add_umicp_example(basic_example basic_example.cpp)
add_umicp_example(event_system_example event_system_example.cpp)
add_umicp_example(multiplexed_peer_example multiplexed_peer_example.cpp)
add_umicp_example(websocket_peer_example websocket_peer_example.cpp)
add_umicp_example(http_example http_example.cpp)
add_umicp_example(service_discovery_example service_discovery_example.cpp)
add_umicp_example(connection_pool_example connection_pool_example.cpp)
