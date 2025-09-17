/**
 * UMICP Schema Registry Implementation
 * Schema validation and management for message types
 */

#include "schema_registry.h"
#include <algorithm>
#include <sstream>

namespace umicp {

SchemaRegistry::SchemaRegistry()
    : stats_{0, 0, 0, std::chrono::steady_clock::now()} {
}

SchemaRegistry::~SchemaRegistry() = default;

// ============================================================================
// Schema Management
// ============================================================================

Result<void> SchemaRegistry::register_schema(const SchemaDefinition& schema) {
    std::lock_guard<std::mutex> lock(registry_mutex_);

    if (schema.id.empty()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Schema ID cannot be empty");
    }

    if (schema.name.empty()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Schema name cannot be empty");
    }

    if (schemas_.find(schema.id) != schemas_.end()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Schema ID already exists");
    }

    auto schema_ptr = std::make_unique<SchemaDefinition>(schema);
    schemas_[schema.id] = std::move(schema_ptr);
    stats_.total_schemas++;

    return Result<void>();
}

Result<void> SchemaRegistry::update_schema(const std::string& schema_id, const SchemaDefinition& schema) {
    std::lock_guard<std::mutex> lock(registry_mutex_);

    auto it = schemas_.find(schema_id);
    if (it == schemas_.end()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Schema not found");
    }

    if (schema.id != schema_id) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Schema ID mismatch");
    }

    auto updated_schema = std::make_unique<SchemaDefinition>(schema);
    updated_schema->updated_at = std::chrono::steady_clock::now();
    it->second = std::move(updated_schema);

    return Result<void>();
}

Result<void> SchemaRegistry::remove_schema(const std::string& schema_id) {
    std::lock_guard<std::mutex> lock(registry_mutex_);

    auto it = schemas_.find(schema_id);
    if (it == schemas_.end()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Schema not found");
    }

    schemas_.erase(it);
    stats_.total_schemas--;

    return Result<void>();
}

Result<const SchemaDefinition*> SchemaRegistry::get_schema(const std::string& schema_id) const {
    std::lock_guard<std::mutex> lock(registry_mutex_);

    auto it = schemas_.find(schema_id);
    if (it == schemas_.end()) {
        return Result<const SchemaDefinition*>(ErrorCode::INVALID_ARGUMENT, "Schema not found");
    }

    return Result<const SchemaDefinition*>(it->second.get());
}

std::vector<std::string> SchemaRegistry::get_all_schema_ids() const {
    std::lock_guard<std::mutex> lock(registry_mutex_);

    std::vector<std::string> ids;
    ids.reserve(schemas_.size());

    for (const auto& [id, schema] : schemas_) {
        ids.push_back(id);
    }

    return ids;
}

// ============================================================================
// Schema Validation
// ============================================================================

ValidationResult SchemaRegistry::validate_message(const std::string& schema_id,
                                                const ByteBuffer& message_data,
                                                const std::string& content_type) {
    std::lock_guard<std::mutex> lock(registry_mutex_);

    stats_.total_validations++;
    stats_.last_validation = std::chrono::steady_clock::now();

    auto schema_result = get_schema(schema_id);
    if (!schema_result.is_success()) {
        stats_.validation_errors++;
        return ValidationResult(false, "Schema not found: " + schema_id);
    }

    const auto* schema = schema_result.value;

    ValidationResult result;
    try {
        if (content_type == "json") {
            std::string json_str(message_data.begin(), message_data.end());
            result = validate_json_schema(*schema, json_str);
        } else if (content_type == "cbor") {
            result = validate_cbor_schema(*schema, message_data);
        } else {
            result = ValidationResult(false, "Unsupported content type: " + content_type);
        }
    } catch (const std::exception& e) {
        result = ValidationResult(false, "Validation error: " + std::string(e.what()));
    }

    if (!result.valid) {
        stats_.validation_errors++;
    }

    return result;
}

ValidationResult SchemaRegistry::validate_envelope(const Envelope& envelope) {
    std::lock_guard<std::mutex> lock(registry_mutex_);

    if (!envelope.schema_uri) {
        return ValidationResult(true); // No schema specified, validation passes
    }

    stats_.total_validations++;
    stats_.last_validation = std::chrono::steady_clock::now();

    return ValidationResult(false, "Envelope schema validation not implemented yet");
}

// ============================================================================
// Schema Compatibility
// ============================================================================

bool SchemaRegistry::is_schema_compatible(const std::string& schema_id,
                                        const std::string& target_version) const {
    std::lock_guard<std::mutex> lock(registry_mutex_);

    auto it = schemas_.find(schema_id);
    if (it == schemas_.end()) {
        return false;
    }

    return it->second->compatible_versions.count(target_version) > 0;
}

std::vector<std::string> SchemaRegistry::get_compatible_versions(const std::string& schema_id) const {
    std::lock_guard<std::mutex> lock(registry_mutex_);

    auto it = schemas_.find(schema_id);
    if (it == schemas_.end()) {
        return {};
    }

    return std::vector<std::string>(
        it->second->compatible_versions.begin(),
        it->second->compatible_versions.end()
    );
}

// ============================================================================
// Schema Discovery
// ============================================================================

std::vector<SchemaDefinition> SchemaRegistry::find_schemas_by_name(const std::string& name) const {
    std::lock_guard<std::mutex> lock(registry_mutex_);

    std::vector<SchemaDefinition> results;

    for (const auto& [id, schema] : schemas_) {
        if (schema->name.find(name) != std::string::npos) {
            results.push_back(*schema);
        }
    }

    return results;
}

std::vector<SchemaDefinition> SchemaRegistry::find_schemas_by_type(SchemaType type) const {
    std::lock_guard<std::mutex> lock(registry_mutex_);

    std::vector<SchemaDefinition> results;

    for (const auto& [id, schema] : schemas_) {
        if (schema->type == type) {
            results.push_back(*schema);
        }
    }

    return results;
}

// ============================================================================
// Statistics
// ============================================================================

SchemaRegistry::Stats SchemaRegistry::get_stats() const {
    std::lock_guard<std::mutex> lock(registry_mutex_);
    return stats_;
}

void SchemaRegistry::reset_stats() {
    std::lock_guard<std::mutex> lock(registry_mutex_);
    stats_ = {schemas_.size(), 0, 0, std::chrono::steady_clock::now()};
}

// ============================================================================
// Validation Helpers
// ============================================================================

ValidationResult SchemaRegistry::validate_json_schema(const SchemaDefinition& schema,
                                                    const std::string& json_data) {
    // Basic JSON validation - check if it's valid JSON
    if (json_data.empty()) {
        return ValidationResult(false, "Empty JSON data");
    }

    // Check basic JSON structure
    if (json_data.front() != '{' && json_data.front() != '[') {
        return ValidationResult(false, "Invalid JSON: must start with '{' or '['");
    }

    if (json_data.back() != '}' && json_data.back() != ']') {
        return ValidationResult(false, "Invalid JSON: must end with '}' or ']'");
    }

    // For now, just do basic validation
    // In production, this would use a proper JSON schema validator
    return ValidationResult(true);
}

ValidationResult SchemaRegistry::validate_cbor_schema(const SchemaDefinition& schema,
                                                    const ByteBuffer& cbor_data) {
    if (cbor_data.empty()) {
        return ValidationResult(false, "Empty CBOR data");
    }

    // Basic CBOR validation - check if it starts with a valid CBOR type
    uint8_t first_byte = cbor_data[0];
    uint8_t major_type = first_byte >> 5;

    if (major_type > 7) {
        return ValidationResult(false, "Invalid CBOR: invalid major type");
    }

    // For now, just do basic validation
    // In production, this would use a proper CBOR schema validator
    return ValidationResult(true);
}

} // namespace umicp
