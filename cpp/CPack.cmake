# CPack configuration for UMICP C++ Library

set(CPACK_PACKAGE_NAME "umicp-dev")
set(CPACK_PACKAGE_VENDOR "HiveLLM")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "UMICP C++ Development Library")
set(CPACK_PACKAGE_DESCRIPTION "UMICP C++ library with headers and CMake config for development")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/hivellm/umicp")
set(CPACK_PACKAGE_CONTACT "HiveLLM Team <team@hivellm.com>")

# Version
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

# Installation directory
set(CPACK_PACKAGE_INSTALL_DIRECTORY "umicp-${CPACK_PACKAGE_VERSION}")

# License
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Package file name
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

# Components (libraries and headers only - this is a dev package)
set(CPACK_COMPONENTS_ALL libraries headers)
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")

# Component descriptions
set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "UMICP shared/static libraries (.so, .dylib, .dll, .a, .lib)")
set(CPACK_COMPONENT_HEADERS_DESCRIPTION "UMICP C++ header files and CMake configuration")

# Component dependencies
set(CPACK_COMPONENT_HEADERS_DEPENDS libraries)

# Source package
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-src")
set(CPACK_SOURCE_IGNORE_FILES
    "/\\.git/"
    "/\\.github/"
    "/build/"
    "/\\.vscode/"
    "/\\.idea/"
    "\\.swp$"
    "\\.orig$"
    "/CMakeLists\\.txt\\.user$"
    "/Makefile$"
    "~$"
)

# Generator-specific settings

# TGZ (Linux/macOS)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

# DEB (Debian/Ubuntu) - Development package
if(UNIX AND NOT APPLE)
    set(CPACK_DEBIAN_PACKAGE_NAME "libumicp-dev")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "HiveLLM Team")
    set(CPACK_DEBIAN_PACKAGE_SECTION "libdevel")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libssl-dev (>= 3.0.0), libstdc++-dev (>= 11.0.0)")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
endif()

# RPM (Red Hat/Fedora)
if(UNIX AND NOT APPLE)
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
    set(CPACK_RPM_PACKAGE_REQUIRES "openssl >= 3.0.0, libstdc++ >= 11.0.0")
    set(CPACK_RPM_PACKAGE_URL "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
endif()

# ZIP (Windows/Universal)
set(CPACK_ARCHIVE_THREADS 0)  # Use all available threads

# Windows ZIP only (no installer for dev library)
# Developers will extract ZIP to their preferred location

# macOS Bundle
if(APPLE)
    set(CPACK_BUNDLE_NAME "UMICP")
    set(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")
    set(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../Logo-HiveLLM-512x512.png")
endif()

# Include CPack
include(CPack)

