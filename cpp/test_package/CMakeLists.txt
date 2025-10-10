cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(UMICP REQUIRED CONFIG)

add_executable(test_umicp test_package.cpp)
target_link_libraries(test_umicp UMICP::UMICP)
target_compile_features(test_umicp PRIVATE cxx_std_17)

