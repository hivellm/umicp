/**
 * UMICP Configuration Implementation
 * Configuration management and validation
 */

#include "umicp_types.h"
#include "config.h"
#include <algorithm>
#include <regex>

namespace umicp {

Result<UMICPConfig> ConfigManager::create_default() {
    UMICPConfig config;
    // Default values are already set in the constructor
    return Result<UMICPConfig>(config);
}

Result<UMICPConfig> ConfigManager::validate(const UMICPConfig& config) {
    // Validate version
    auto version_result = validate_version(config.version);
    if (!version_result.is_success()) {
        return Result<UMICPConfig>(version_result.code, version_result.error_message.value());
    }

    // Validate message size
    auto size_result = validate_message_size(config.max_message_size);
    if (!size_result.is_success()) {
        return Result<UMICPConfig>(size_result.code, size_result.error_message.value());
    }

    // Validate timeouts
    auto timeout_result = validate_timeout(config.connection_timeout);
    if (!timeout_result.is_success()) {
        return Result<UMICPConfig>(timeout_result.code, timeout_result.error_message.value());
    }

    timeout_result = validate_timeout(config.heartbeat_interval);
    if (!timeout_result.is_success()) {
        return Result<UMICPConfig>(timeout_result.code, timeout_result.error_message.value());
    }

    // Validate compression threshold
    if (config.compression_threshold > config.max_message_size) {
        return Result<UMICPConfig>(ErrorCode::INVALID_ARGUMENT,
            "Compression threshold cannot be larger than max message size");
    }

    return Result<UMICPConfig>(config);
}

Result<UMICPConfig> ConfigManager::merge(const UMICPConfig& base, const UMICPConfig& override) {
    UMICPConfig merged = base;

    // Override non-zero/non-empty values
    if (!override.version.empty()) merged.version = override.version;
    if (override.max_message_size > 0) merged.max_message_size = override.max_message_size;
    if (override.connection_timeout > 0) merged.connection_timeout = override.connection_timeout;
    if (override.heartbeat_interval > 0) merged.heartbeat_interval = override.heartbeat_interval;
    if (override.compression_threshold > 0) merged.compression_threshold = override.compression_threshold;

    // Override boolean flags
    merged.enable_binary = override.enable_binary;
    merged.enable_compression = override.enable_compression;
    merged.require_auth = override.require_auth;
    merged.require_encryption = override.require_encryption;
    merged.validate_certificates = override.validate_certificates;

    // Override enums
    if (static_cast<int>(override.preferred_format) != 0) {
        merged.preferred_format = override.preferred_format;
    }

    return Result<UMICPConfig>(merged);
}

Result<UMICPConfig> ConfigManager::load_from_file(const std::string& /* filename */) {
    // TODO: Implement file loading (JSON/TOML/YAML)
    // For now, return default config
    return create_default();
}

Result<void> ConfigManager::save_to_file(const UMICPConfig& /* config */, const std::string& /* filename */) {
    // TODO: Implement file saving
    return Result<void>(ErrorCode::NOT_IMPLEMENTED, "File operations not implemented yet");
}

Result<void> ConfigManager::validate_version(const std::string& version) {
    // Version should be in format "major.minor"
    std::regex version_pattern("^\\d+\\.\\d+$");
    if (!std::regex_match(version, version_pattern)) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Invalid version format (expected major.minor)");
    }

    // Check if it's a supported version
    if (version != "1.0" && version != "1.1") {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Unsupported protocol version");
    }

    return Result<void>();
}

Result<void> ConfigManager::validate_message_size(size_t size) {
    const size_t MIN_SIZE = 1024;        // 1KB minimum
    const size_t MAX_SIZE = 100 * 1024 * 1024; // 100MB maximum

    if (size < MIN_SIZE) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Message size too small (minimum 1KB)");
    }

    if (size > MAX_SIZE) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Message size too large (maximum 100MB)");
    }

    return Result<void>();
}

Result<void> ConfigManager::validate_timeout(uint32_t timeout) {
    const uint32_t MIN_TIMEOUT = 100;      // 100ms minimum
    const uint32_t MAX_TIMEOUT = 300000;   // 5 minutes maximum

    if (timeout < MIN_TIMEOUT) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Timeout too small (minimum 100ms)");
    }

    if (timeout > MAX_TIMEOUT) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Timeout too large (maximum 5 minutes)");
    }

    return Result<void>();
}

} // namespace umicp
