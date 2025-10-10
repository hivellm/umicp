#!/usr/bin/env python
# -*- coding: utf-8 -*-

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy, get, rmdir
import os


class UmicpConan(ConanFile):
    name = "umicp"
    version = "0.1.1"
    license = "MIT"
    author = "HiveLLM Team"
    url = "https://github.com/hivellm/umicp"
    homepage = "https://github.com/hivellm/umicp"
    description = "Universal Matrix Intelligent Communication Protocol - High-performance communication protocol for AI model interoperability"
    topics = ("ai", "communication", "protocol", "matrix", "websocket", "http2")

    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_websocket": [True, False],
        "with_http": [True, False],
        "with_tests": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_websocket": False,  # Disabled by default for simpler dependencies
        "with_http": False,       # Disabled by default for simpler dependencies
        "with_tests": False
    }

    exports_sources = "CMakeLists.txt", "src/*", "include/*", "examples/*", "tests/*", "*.pc.in", "*.cmake.in", "CPack.cmake"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def requirements(self):
        # Core dependencies
        self.requires("nlohmann_json/3.11.3")
        self.requires("openssl/3.2.0")

        # WebSocket support
        if self.options.with_websocket:
            self.requires("websocketpp/0.8.2")
            self.requires("asio/1.28.0")

        # HTTP support
        if self.options.with_http:
            self.requires("libcurl/8.4.0")

        # Test dependencies
        if self.options.with_tests:
            self.requires("gtest/1.14.0")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_TESTS"] = self.options.with_tests  # Match CMakeLists.txt variable
        tc.variables["BUILD_EXAMPLES"] = False  # Match CMakeLists.txt variable
        tc.variables["ENABLE_WEBSOCKET"] = self.options.with_websocket
        tc.variables["ENABLE_HTTP"] = self.options.with_http
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if self.options.with_tests:
            cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

        # Copy license
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))

        # Remove unwanted files
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))

    def package_info(self):
        self.cpp_info.libs = ["umicp"]
        self.cpp_info.includedirs = ["include"]

        # Add definitions
        if self.options.with_websocket:
            self.cpp_info.defines.append("UMICP_ENABLE_WEBSOCKET")
        if self.options.with_http:
            self.cpp_info.defines.append("UMICP_ENABLE_HTTP")

        # Platform-specific settings
        if self.settings.os == "Linux":
            self.cpp_info.system_libs.extend(["pthread", "dl"])
        elif self.settings.os == "Windows":
            self.cpp_info.system_libs.extend(["ws2_32", "wsock32"])

        # CMake targets
        self.cpp_info.set_property("cmake_file_name", "UMICP")
        self.cpp_info.set_property("cmake_target_name", "UMICP::UMICP")
        self.cpp_info.set_property("pkg_config_name", "umicp")

