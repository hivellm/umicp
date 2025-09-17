# Simple Makefile for UMICP Tests
# Fallback when CMake has issues

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -I../include -I/usr/include/gtest -DUMICP_TESTING
LIBS = -lgtest -lgtest_main -ljson-c -lssl -lcrypto -lz -lpthread

# Source files
UTILS_SRC = utils/test_helpers.cpp
MAIN_SRC = test_main.cpp

# Core library sources (simplified)
CORE_SRCS = ../src/matrix_ops.cpp ../src/protocol.cpp ../src/transport.cpp ../src/serialization.cpp ../src/security.cpp

# Test executables
TESTS = simple_matrix_test simple_transport_test

.PHONY: all clean

all: $(TESTS)

simple_matrix_test: unit/test_matrix_ops.cpp $(UTILS_SRC) $(MAIN_SRC) $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

simple_transport_test: unit/test_transport.cpp $(UTILS_SRC) $(MAIN_SRC) $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TESTS)

test: $(TESTS)
	@echo "Running simple matrix tests..."
	./simple_matrix_test
	@echo "Running simple transport tests..."
	./simple_transport_test
