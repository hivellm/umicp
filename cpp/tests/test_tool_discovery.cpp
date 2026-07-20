/**
 * UMICP Tool Discovery Test
 * Tests for v1.1.0 tool discovery interfaces
 */

#include "../include/tool_discovery.h"
#include <nlohmann/json.hpp>
#include <cassert>
#include <iostream>

using namespace umicp;
using json = nlohmann::json;

// Example service implementation
class TestService : public DiscoverableService {
public:
    std::vector<OperationSchema> list_operations() const override {
        return {
            OperationSchema("search_vectors", json{
                {"type", "object"},
                {"properties", {
                    {"collection", {{"type", "string"}, {"description", "Collection name"}}},
                    {"query", {{"type", "string"}, {"description", "Search query"}}},
                    {"limit", {{"type", "integer"}, {"default", 10}, {"minimum", 1}, {"maximum", 100}}}
                }},
                {"required", json::array({"collection", "query"})}
            })
            .with_title("Search Vectors")
            .with_description("Search for semantically similar content")
            .with_annotations(json{{"read_only", true}, {"idempotent", true}}),

            OperationSchema("create_collection", json{
                {"type", "object"},
                {"properties", {
                    {"name", {{"type", "string"}}},
                    {"dimension", {{"type", "integer"}}}
                }},
                {"required", json::array({"name", "dimension"})}
            })
            .with_title("Create Collection")
            .with_description("Create a new vector collection")
            .with_annotations(json{{"read_only", false}})
        };
    }

    ServerInfo server_info() const override {
        return ServerInfo("test-service", "1.0.0", "UMICP/1.1")
            .with_features({"discovery", "search", "collections"})
            .with_operations_count(2)
            .with_mcp_compatible(true)
            .with_metadata(json{{"author", "HiveLLM"}, {"license", "MIT"}});
    }
};

void test_operation_schema_creation() {
    std::cout << "Test: OperationSchema creation... ";

    auto schema = OperationSchema("test_op", json{{"type", "object"}})
        .with_title("Test Operation")
        .with_description("A test operation")
        .with_annotations(json{{"read_only", true}});

    assert(schema.name == "test_op");
    assert(schema.title == "Test Operation");
    assert(schema.description == "A test operation");
    assert(schema.input_schema["type"] == "object");
    assert(schema.annotations.value()["read_only"] == true);

    std::cout << "✅ PASSED\n";
}

void test_operation_schema_to_json() {
    std::cout << "Test: OperationSchema to JSON... ";

    auto schema = OperationSchema("search", json{
        {"type", "object"},
        {"properties", {{"query", {{"type", "string"}}}}}
    })
    .with_title("Search")
    .with_annotations(json{{"read_only", true}});

    auto json_output = schema.to_json();

    assert(json_output["name"] == "search");
    assert(json_output["title"] == "Search");
    assert(json_output.contains("input_schema"));
    assert(json_output["annotations"]["read_only"] == true);

    std::cout << "✅ PASSED\n";
}

void test_server_info_creation() {
    std::cout << "Test: ServerInfo creation... ";

    auto info = ServerInfo("my-service", "1.0.0", "UMICP/1.1")
        .with_features({"discovery", "streaming"})
        .with_operations_count(10)
        .with_mcp_compatible(true);

    assert(info.server == "my-service");
    assert(info.version == "1.0.0");
    assert(info.protocol == "UMICP/1.1");
    assert(info.features->size() == 2);
    assert(info.operations_count == 10);
    assert(info.mcp_compatible == true);

    std::cout << "✅ PASSED\n";
}

void test_server_info_to_json() {
    std::cout << "Test: ServerInfo to JSON... ";

    auto info = ServerInfo("vectorizer", "0.9.0", "UMICP/1.1")
        .with_features({"discovery", "search"})
        .with_operations_count(40)
        .with_mcp_compatible(true)
        .with_metadata(json{{"license", "MIT"}});

    auto json_output = info.to_json();

    assert(json_output["server"] == "vectorizer");
    assert(json_output["version"] == "0.9.0");
    assert(json_output["protocol"] == "UMICP/1.1");
    assert(json_output["features"].size() == 2);
    assert(json_output["operations_count"] == 40);
    assert(json_output["mcp_compatible"] == true);
    assert(json_output["metadata"]["license"] == "MIT");

    std::cout << "✅ PASSED\n";
}

void test_discoverable_service_list_operations() {
    std::cout << "Test: DiscoverableService::list_operations()... ";

    TestService service;
    auto operations = service.list_operations();

    assert(operations.size() == 2);
    assert(operations[0].name == "search_vectors");
    assert(operations[0].title == "Search Vectors");
    assert(operations[1].name == "create_collection");

    std::cout << "✅ PASSED\n";
}

void test_discoverable_service_get_schema() {
    std::cout << "Test: DiscoverableService::get_schema()... ";

    TestService service;

    // Get existing schema
    auto schema = service.get_schema("search_vectors");
    assert(schema.has_value());
    assert(schema->name == "search_vectors");
    assert(schema->annotations.value()["read_only"] == true);

    // Non-existent schema
    auto not_found = service.get_schema("non_existent");
    assert(!not_found.has_value());

    std::cout << "✅ PASSED\n";
}

void test_discoverable_service_server_info() {
    std::cout << "Test: DiscoverableService::server_info()... ";

    TestService service;
    auto info = service.server_info();

    assert(info.server == "test-service");
    assert(info.version == "1.0.0");
    assert(info.protocol == "UMICP/1.1");
    assert(info.features->size() == 3);
    assert(info.mcp_compatible == true);

    std::cout << "✅ PASSED\n";
}

void test_generate_operations_response() {
    std::cout << "Test: generate_operations_response()... ";

    TestService service;
    auto response = service.generate_operations_response();

    assert(response.contains("operations"));
    assert(response["operations"].is_array());
    assert(response["operations"].size() == 2);
    assert(response["count"] == 2);
    assert(response["protocol"] == "UMICP/1.1");
    assert(response["mcp_compatible"] == true);

    std::cout << "✅ PASSED\n";
}

void test_generate_schema_response() {
    std::cout << "Test: generate_schema_response()... ";

    TestService service;

    // Existing operation
    auto response = service.generate_schema_response("search_vectors");
    assert(response.contains("name"));
    assert(response["name"] == "search_vectors");
    assert(response.contains("input_schema"));

    // Non-existent operation
    auto error_response = service.generate_schema_response("invalid");
    assert(error_response.contains("error"));
    assert(error_response["error"] == "Operation not found");

    std::cout << "✅ PASSED\n";
}

void test_generate_server_info_response() {
    std::cout << "Test: generate_server_info_response()... ";

    TestService service;
    auto response = service.generate_server_info_response();

    assert(response["server"] == "test-service");
    assert(response["version"] == "1.0.0");
    assert(response.contains("features"));
    assert(response["mcp_compatible"] == true);

    std::cout << "✅ PASSED\n";
}

int main() {
    std::cout << "\n=== UMICP C++ Core v1.1.0 - Tool Discovery Tests ===\n\n";

    try {
        // Native types tests
        test_operation_schema_creation();
        test_operation_schema_to_json();
        test_server_info_creation();
        test_server_info_to_json();

        // Discovery interface tests
        test_discoverable_service_list_operations();
        test_discoverable_service_get_schema();
        test_discoverable_service_server_info();
        test_generate_operations_response();
        test_generate_schema_response();
        test_generate_server_info_response();

        std::cout << "\n✅ All tests PASSED! (10/10)\n";
        std::cout << "Tool Discovery interfaces are 100% functional.\n\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}

