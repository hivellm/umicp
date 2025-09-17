/**
 * UMICP Schema Registry Header
 * Schema validation and management for message types
 */

#ifndef UMICP_SCHEMA_REGISTRY_H
#define UMICP_SCHEMA_REGISTRY_H

#include "umicp_types.h"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <string>
#include <mutex>

namespace umicp {

// Forward declarations
struct SchemaDefinition;
class SchemaValidator;

// Schema types
enum class SchemaType {
    JSON_SCHEMA = 0,
    CBOR_SCHEMA = 1,
    PROTOBUF = 2,
    CUSTOM = 3
};

// Schema definition structure
struct SchemaDefinition {
    std::string id;
    std::string name;
    std::string version;
    SchemaType type;
    std::string schema_content;
    std::unordered_set<std::string> compatible_versions;
    std::chrono::steady_clock::time_point created_at;
    std::chrono::steady_clock::time_point updated_at;

    SchemaDefinition(std::string schema_id, std::string schema_name,
                    std::string schema_version, SchemaType schema_type,
                    std::string content)
        : id(std::move(schema_id)), name(std::move(schema_name)),
          version(std::move(schema_version)), type(schema_type),
          schema_content(std::move(content)),
          created_at(std::chrono::steady_clock::now()),
          updated_at(std::chrono::steady_clock::now()) {}
};

// Schema validation result
struct ValidationResult {
    bool valid;
    std::string error_message;
    std::vector<std::string> warnings;

    ValidationResult(bool is_valid = true, std::string error = "")
        : valid(is_valid), error_message(std::move(error)) {}
};

// Schema registry class
class SchemaRegistry {
public:
    SchemaRegistry();
    ~SchemaRegistry();

    // Schema management
    Result<void> register_schema(const SchemaDefinition& schema);
    Result<void> update_schema(const std::string& schema_id, const SchemaDefinition& schema);
    Result<void> remove_schema(const std::string& schema_id);
    Result<const SchemaDefinition*> get_schema(const std::string& schema_id) const;
    std::vector<std::string> get_all_schema_ids() const;

    // Schema validation
    ValidationResult validate_message(const std::string& schema_id,
                                    const ByteBuffer& message_data,
                                    const std::string& content_type = "json");
    ValidationResult validate_envelope(const Envelope& envelope);

    // Schema compatibility
    bool is_schema_compatible(const std::string& schema_id,
                             const std::string& target_version) const;
    std::vector<std::string> get_compatible_versions(const std::string& schema_id) const;

    // Schema discovery
    std::vector<SchemaDefinition> find_schemas_by_name(const std::string& name) const;
    std::vector<SchemaDefinition> find_schemas_by_type(SchemaType type) const;

    // Statistics
    struct Stats {
        size_t total_schemas;
        size_t total_validations;
        size_t validation_errors;
        std::chrono::steady_clock::time_point last_validation;
    };

    Stats get_stats() const;
    void reset_stats();

private:
    std::unordered_map<std::string, std::unique_ptr<SchemaDefinition>> schemas_;
    mutable Stats stats_;
    mutable std::mutex registry_mutex_;

    // Validation helpers
    ValidationResult validate_json_schema(const SchemaDefinition& schema,
                                        const std::string& json_data);
    ValidationResult validate_cbor_schema(const SchemaDefinition& schema,
                                        const ByteBuffer& cbor_data);
};

} // namespace umicp

#endif // UMICP_SCHEMA_REGISTRY_H
