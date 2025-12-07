/**
 * UMICP Tool Discovery - C++ Core
 * Tool discovery interfaces for auto-discovery services
 *
 * Version 0.2.0 - MCP-compatible tool discovery
 */

#ifndef UMICP_TOOL_DISCOVERY_H
#define UMICP_TOOL_DISCOVERY_H

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace umicp {

using json = nlohmann::json;

/**
 * Operation schema compatible with MCP JSON Schema
 *
 * Describes a single operation/tool with its input/output schemas,
 * allowing clients to discover and validate operations programmatically.
 */
struct OperationSchema {
    /// Operation name (e.g., "search_vectors")
    std::string name;

    /// Display title (e.g., "Search Vectors")
    std::optional<std::string> title;

    /// Description of what the operation does
    std::optional<std::string> description;

    /// JSON Schema for input parameters
    /// Should follow JSON Schema Draft 7 or later
    json input_schema;

    /// Optional JSON Schema for output
    std::optional<json> output_schema;

    /// Optional annotations for metadata
    /// Examples: {"read_only": true, "idempotent": true, "destructive": false}
    std::optional<json> annotations;

    /**
     * Constructor with minimal required fields
     */
    OperationSchema(const std::string& op_name, const json& schema)
        : name(op_name), input_schema(schema) {}

    /**
     * Builder-style setters
     */
    OperationSchema& with_title(const std::string& t) {
        title = t;
        return *this;
    }

    OperationSchema& with_description(const std::string& d) {
        description = d;
        return *this;
    }

    OperationSchema& with_output_schema(const json& schema) {
        output_schema = schema;
        return *this;
    }

    OperationSchema& with_annotations(const json& annot) {
        annotations = annot;
        return *this;
    }

    /**
     * Convert to JSON representation
     */
    json to_json() const {
        json result;
        result["name"] = name;

        if (title) result["title"] = *title;
        if (description) result["description"] = *description;

        result["input_schema"] = input_schema;

        if (output_schema) result["output_schema"] = *output_schema;
        if (annotations) result["annotations"] = *annotations;

        return result;
    }
};

/**
 * Server information for discovery
 *
 * Provides metadata about the server, its capabilities,
 * and available operations.
 */
struct ServerInfo {
    /// Server name/identifier
    std::string server;

    /// Server version
    std::string version;

    /// Protocol version (e.g., "UMICP/1.1", "UMICP/2.0")
    std::string protocol;

    /// List of supported features
    std::optional<std::vector<std::string>> features;

    /// Number of available operations
    std::optional<size_t> operations_count;

    /// Whether server is MCP-compatible
    std::optional<bool> mcp_compatible;

    /// Additional metadata
    std::optional<json> metadata;

    /**
     * Constructor with required fields
     */
    ServerInfo(const std::string& srv, const std::string& ver, const std::string& proto)
        : server(srv), version(ver), protocol(proto) {}

    /**
     * Builder-style setters
     */
    ServerInfo& with_features(const std::vector<std::string>& f) {
        features = f;
        return *this;
    }

    ServerInfo& with_operations_count(size_t count) {
        operations_count = count;
        return *this;
    }

    ServerInfo& with_mcp_compatible(bool compatible) {
        mcp_compatible = compatible;
        return *this;
    }

    ServerInfo& with_metadata(const json& meta) {
        metadata = meta;
        return *this;
    }

    /**
     * Convert to JSON representation
     */
    json to_json() const {
        json result;
        result["server"] = server;
        result["version"] = version;
        result["protocol"] = protocol;

        if (features) result["features"] = *features;
        if (operations_count) result["operations_count"] = *operations_count;
        if (mcp_compatible) result["mcp_compatible"] = *mcp_compatible;
        if (metadata) result["metadata"] = *metadata;

        return result;
    }
};

/**
 * Interface for services that support tool discovery
 *
 * Implement this interface to provide automatic tool discovery
 * and introspection for your UMICP service.
 *
 * Example:
 * ```cpp
 * class MyService : public DiscoverableService {
 * public:
 *     std::vector<OperationSchema> list_operations() const override {
 *         return {
 *             OperationSchema("search", json{{"type", "object"}, {"properties", {...}}})
 *                 .with_title("Search")
 *                 .with_description("Search for items")
 *                 .with_annotations(json{{"read_only", true}})
 *         };
 *     }
 *
 *     ServerInfo server_info() const override {
 *         return ServerInfo("my-service", "1.0.0", "UMICP/1.1")
 *             .with_features({"discovery", "search"})
 *             .with_mcp_compatible(true);
 *     }
 * };
 * ```
 */
class DiscoverableService {
public:
    virtual ~DiscoverableService() = default;

    /**
     * List all available operations with their schemas
     *
     * @return Vector of operation schemas describing all available operations
     */
    virtual std::vector<OperationSchema> list_operations() const = 0;

    /**
     * Get schema for a specific operation by name
     *
     * Default implementation searches through list_operations().
     * Override for more efficient lookup if needed.
     *
     * @param name Operation name to look up
     * @return Operation schema if found, std::nullopt otherwise
     */
    virtual std::optional<OperationSchema> get_schema(const std::string& name) const {
        auto operations = list_operations();
        for (const auto& op : operations) {
            if (op.name == name) {
                return op;
            }
        }
        return std::nullopt;
    }

    /**
     * Get server information and metadata
     *
     * @return Server information including version, protocol, features
     */
    virtual ServerInfo server_info() const = 0;

    /**
     * Helper: Generate JSON response for _list_operations
     *
     * Convenience method to generate a complete response for
     * the _list_operations discovery operation.
     *
     * @return JSON object with operations array and metadata
     */
    json generate_operations_response() const {
        auto ops = list_operations();
        json result;

        json ops_array = json::array();
        for (const auto& op : ops) {
            ops_array.push_back(op.to_json());
        }

        result["operations"] = ops_array;
        result["count"] = ops.size();

        auto info = server_info();
        result["protocol"] = info.protocol;
        if (info.mcp_compatible) {
            result["mcp_compatible"] = *info.mcp_compatible;
        }

        return result;
    }

    /**
     * Helper: Generate JSON response for _get_schema
     *
     * @param operation_name Name of operation to get schema for
     * @return JSON object with schema, or error if not found
     */
    json generate_schema_response(const std::string& operation_name) const {
        auto schema = get_schema(operation_name);

        if (schema) {
            return schema->to_json();
        } else {
            return json{
                {"error", "Operation not found"},
                {"operation", operation_name}
            };
        }
    }

    /**
     * Helper: Generate JSON response for _server_info
     *
     * @return JSON object with complete server information
     */
    json generate_server_info_response() const {
        return server_info().to_json();
    }
};

} // namespace umicp

#endif // UMICP_TOOL_DISCOVERY_H

