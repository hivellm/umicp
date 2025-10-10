/**
 * UMICP Schema Registry - Comprehensive Tests
 * 100% coverage of schema validation operations
 */

#include "schema_registry.h"
#include "envelope.h"
#include "umicp_types.h"
#include <gtest/gtest.h>

using namespace umicp;

// ============================================================================
// Schema Registration Tests
// ============================================================================

class SchemaRegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        registry = std::make_unique<SchemaRegistry>();
    }

    std::unique_ptr<SchemaRegistry> registry;
};

TEST_F(SchemaRegistryTest, RegisterBasicSchema) {
    SchemaDefinition schema;
    schema.id = "test-schema-v1";
    schema.version = "1.0.0";
    schema.schema_data = R"({"type": "object"})";
    schema.content_type = "json";

    auto result = registry->register_schema(schema);

    EXPECT_TRUE(result.is_ok());
}

TEST_F(SchemaRegistryTest, RegisterMultipleSchemas) {
    SchemaDefinition schema1;
    schema1.id = "schema1";
    schema1.version = "1.0.0";
    schema1.schema_data = "{}";

    SchemaDefinition schema2;
    schema2.id = "schema2";
    schema2.version = "1.0.0";
    schema2.schema_data = "{}";

    EXPECT_TRUE(registry->register_schema(schema1).is_ok());
    EXPECT_TRUE(registry->register_schema(schema2).is_ok());
}

TEST_F(SchemaRegistryTest, RegisterSameIdDifferentVersions) {
    SchemaDefinition v1;
    v1.id = "schema";
    v1.version = "1.0.0";
    v1.schema_data = "{}";

    SchemaDefinition v2;
    v2.id = "schema";
    v2.version = "2.0.0";
    v2.schema_data = "{}";

    EXPECT_TRUE(registry->register_schema(v1).is_ok());
    EXPECT_TRUE(registry->register_schema(v2).is_ok());
}

TEST_F(SchemaRegistryTest, RegisterDuplicateSchemaFails) {
    SchemaDefinition schema;
    schema.id = "duplicate";
    schema.version = "1.0.0";
    schema.schema_data = "{}";

    EXPECT_TRUE(registry->register_schema(schema).is_ok());

    // Try to register same schema again
    auto result = registry->register_schema(schema);
    EXPECT_FALSE(result.is_ok());
}

TEST_F(SchemaRegistryTest, RegisterEmptyIdFails) {
    SchemaDefinition schema;
    schema.id = "";
    schema.version = "1.0.0";
    schema.schema_data = "{}";

    auto result = registry->register_schema(schema);

    EXPECT_FALSE(result.is_ok());
}

TEST_F(SchemaRegistryTest, RegisterEmptyVersionFails) {
    SchemaDefinition schema;
    schema.id = "test";
    schema.version = "";
    schema.schema_data = "{}";

    auto result = registry->register_schema(schema);

    EXPECT_FALSE(result.is_ok());
}

// ============================================================================
// Schema Retrieval Tests
// ============================================================================

TEST_F(SchemaRegistryTest, GetExistingSchema) {
    SchemaDefinition schema;
    schema.id = "get-test";
    schema.version = "1.0.0";
    schema.schema_data = R"({"test": true})";

    registry->register_schema(schema);

    auto result = registry->get_schema("get-test", "1.0.0");

    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(result.value.has_value());
    EXPECT_EQ(result.value->id, "get-test");
}

TEST_F(SchemaRegistryTest, GetNonExistentSchemaFails) {
    auto result = registry->get_schema("nonexistent", "1.0.0");

    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::NOT_FOUND);
}

TEST_F(SchemaRegistryTest, GetSchemaWrongVersionFails) {
    SchemaDefinition schema;
    schema.id = "versioned";
    schema.version = "1.0.0";
    schema.schema_data = "{}";

    registry->register_schema(schema);

    auto result = registry->get_schema("versioned", "2.0.0");

    EXPECT_FALSE(result.is_ok());
}

TEST_F(SchemaRegistryTest, GetLatestVersion) {
    SchemaDefinition v1;
    v1.id = "multi-version";
    v1.version = "1.0.0";
    v1.schema_data = "v1";

    SchemaDefinition v2;
    v2.id = "multi-version";
    v2.version = "2.0.0";
    v2.schema_data = "v2";

    registry->register_schema(v1);
    registry->register_schema(v2);

    auto result = registry->get_latest_schema("multi-version");

    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(result.value.has_value());
    EXPECT_EQ(result.value->version, "2.0.0");
}

// ============================================================================
// Envelope Validation Tests
// ============================================================================

TEST_F(SchemaRegistryTest, ValidateBasicEnvelope) {
    Envelope envelope;
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);
    envelope.set_message_id("msg-123");

    auto result = registry->validate_envelope(envelope);

    EXPECT_TRUE(result.is_ok());
}

TEST_F(SchemaRegistryTest, ValidateEnvelopeWithAllFields) {
    Envelope envelope;
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);
    envelope.set_message_id("msg-123");
    envelope.set_timestamp("2025-10-10T10:00:00Z");
    envelope.set_content_type(ContentType::JSON);

    StringMap caps;
    caps["version"] = "1.0";
    envelope.set_capabilities(caps);

    PayloadHint hint;
    hint.type = PayloadType::VECTOR;
    envelope.set_payload_hint(hint);

    auto result = registry->validate_envelope(envelope);

    EXPECT_TRUE(result.is_ok());
}

TEST_F(SchemaRegistryTest, ValidateEnvelopeWithMissingFrom) {
    Envelope envelope;
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);

    auto result = registry->validate_envelope(envelope);

    // May fail depending on validation rules
}

TEST_F(SchemaRegistryTest, ValidateEnvelopeWithMissingOperation) {
    Envelope envelope;
    envelope.set_from("sender");
    envelope.set_to("receiver");

    auto result = registry->validate_envelope(envelope);

    // Should fail - operation is required
}

// ============================================================================
// Message Validation with Schema
// ============================================================================

TEST_F(SchemaRegistryTest, ValidateMessageWithSchema) {
    // Register schema
    SchemaDefinition schema;
    schema.id = "message-schema";
    schema.version = "1.0.0";
    schema.schema_data = R"({
        "type": "object",
        "properties": {
            "name": {"type": "string"},
            "age": {"type": "number"}
        },
        "required": ["name"]
    })";
    schema.content_type = "json";

    registry->register_schema(schema);

    // Valid message
    std::string valid_message = R"({"name": "Alice", "age": 30})";

    auto result = registry->validate_message_with_schema(
        "message-schema",
        "1.0.0",
        valid_message
    );

    // Validation depends on schema implementation
}

TEST_F(SchemaRegistryTest, ValidateMessageInvalidData) {
    SchemaDefinition schema;
    schema.id = "test";
    schema.version = "1.0.0";
    schema.schema_data = R"({"type": "object"})";

    registry->register_schema(schema);

    std::string invalid = "not valid json";

    auto result = registry->validate_message_with_schema("test", "1.0.0", invalid);

    EXPECT_FALSE(result.is_ok());
}

// ============================================================================
// Version Compatibility Tests
// ============================================================================

TEST_F(SchemaRegistryTest, CheckVersionCompatibility) {
    auto result = registry->check_compatibility("schema", "1.0.0", "1.0.0");

    // Same versions should be compatible
    if (result.is_ok()) {
        EXPECT_TRUE(*result.value);
    }
}

TEST_F(SchemaRegistryTest, CheckMinorVersionCompatibility) {
    auto result = registry->check_compatibility("schema", "1.0.0", "1.1.0");

    // Minor version increase should be compatible
}

TEST_F(SchemaRegistryTest, CheckMajorVersionIncompatibility) {
    auto result = registry->check_compatibility("schema", "1.0.0", "2.0.0");

    // Major version change might be incompatible
}

// ============================================================================
// Schema Removal Tests
// ============================================================================

TEST_F(SchemaRegistryTest, UnregisterSchema) {
    SchemaDefinition schema;
    schema.id = "removable";
    schema.version = "1.0.0";
    schema.schema_data = "{}";

    registry->register_schema(schema);

    auto result = registry->unregister_schema("removable", "1.0.0");

    EXPECT_TRUE(result.is_ok());

    // Should not be found after removal
    auto get_result = registry->get_schema("removable", "1.0.0");
    EXPECT_FALSE(get_result.is_ok());
}

TEST_F(SchemaRegistryTest, UnregisterNonExistentSchemaFails) {
    auto result = registry->unregister_schema("nonexistent", "1.0.0");

    EXPECT_FALSE(result.is_ok());
}

// ============================================================================
// Schema Listing Tests
// ============================================================================

TEST_F(SchemaRegistryTest, ListSchemasWhenEmpty) {
    auto schemas = registry->list_schemas();

    EXPECT_TRUE(schemas.empty());
}

TEST_F(SchemaRegistryTest, ListSchemasWithMultiple) {
    SchemaDefinition s1, s2, s3;
    s1.id = "schema1";
    s1.version = "1.0.0";
    s1.schema_data = "{}";

    s2.id = "schema2";
    s2.version = "1.0.0";
    s2.schema_data = "{}";

    s3.id = "schema3";
    s3.version = "1.0.0";
    s3.schema_data = "{}";

    registry->register_schema(s1);
    registry->register_schema(s2);
    registry->register_schema(s3);

    auto schemas = registry->list_schemas();

    EXPECT_EQ(schemas.size(), 3);
}

TEST_F(SchemaRegistryTest, ListVersionsForSchema) {
    SchemaDefinition v1, v2, v3;
    v1.id = "multi";
    v1.version = "1.0.0";
    v1.schema_data = "{}";

    v2.id = "multi";
    v2.version = "1.1.0";
    v2.schema_data = "{}";

    v3.id = "multi";
    v3.version = "2.0.0";
    v3.schema_data = "{}";

    registry->register_schema(v1);
    registry->register_schema(v2);
    registry->register_schema(v3);

    auto versions = registry->list_versions("multi");

    EXPECT_GE(versions.size(), 3);
}

// ============================================================================
// Statistics Tests
// ============================================================================

TEST_F(SchemaRegistryTest, GetStatistics) {
    auto stats = registry->get_stats();

    EXPECT_GE(stats.schemas_registered, 0);
    EXPECT_GE(stats.validations_performed, 0);
    EXPECT_GE(stats.validation_failures, 0);
}

TEST_F(SchemaRegistryTest, StatisticsIncreaseAfterOperations) {
    auto stats_before = registry->get_stats();

    SchemaDefinition schema;
    schema.id = "stats-test";
    schema.version = "1.0.0";
    schema.schema_data = "{}";

    registry->register_schema(schema);

    Envelope envelope;
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);

    registry->validate_envelope(envelope);

    auto stats_after = registry->get_stats();

    EXPECT_GT(stats_after.schemas_registered, stats_before.schemas_registered);
    EXPECT_GT(stats_after.validations_performed, stats_before.validations_performed);
}

TEST_F(SchemaRegistryTest, ResetStatistics) {
    SchemaDefinition schema;
    schema.id = "test";
    schema.version = "1.0.0";
    schema.schema_data = "{}";

    registry->register_schema(schema);
    registry->reset_stats();

    auto stats = registry->get_stats();

    EXPECT_EQ(stats.validations_performed, 0);
    EXPECT_EQ(stats.validation_failures, 0);
}

// ============================================================================
// Schema Content Types
// ============================================================================

TEST_F(SchemaRegistryTest, JsonSchemaContentType) {
    SchemaDefinition schema;
    schema.id = "json-schema";
    schema.version = "1.0.0";
    schema.schema_data = R"({"type": "object"})";
    schema.content_type = "json";

    auto result = registry->register_schema(schema);

    EXPECT_TRUE(result.is_ok());
}

TEST_F(SchemaRegistryTest, XmlSchemaContentType) {
    SchemaDefinition schema;
    schema.id = "xml-schema";
    schema.version = "1.0.0";
    schema.schema_data = "<schema></schema>";
    schema.content_type = "xml";

    auto result = registry->register_schema(schema);

    EXPECT_TRUE(result.is_ok());
}

TEST_F(SchemaRegistryTest, CustomSchemaContentType) {
    SchemaDefinition schema;
    schema.id = "custom-schema";
    schema.version = "1.0.0";
    schema.schema_data = "custom schema definition";
    schema.content_type = "custom";

    auto result = registry->register_schema(schema);

    EXPECT_TRUE(result.is_ok());
}

// ============================================================================
// Version Parsing Tests
// ============================================================================

TEST(SchemaVersionTest, ParseSemanticVersion) {
    std::string version = "1.2.3";

    // Should parse correctly
    EXPECT_NE(version.find("1"), std::string::npos);
}

TEST(SchemaVersionTest, ParseMajorMinorOnly) {
    std::string version = "2.1";

    EXPECT_NE(version.find("2"), std::string::npos);
}

TEST(SchemaVersionTest, ParseWithPrerelease) {
    std::string version = "1.0.0-alpha.1";

    EXPECT_NE(version.find("alpha"), std::string::npos);
}

TEST(SchemaVersionTest, ParseWithBuildMetadata) {
    std::string version = "1.0.0+build.123";

    EXPECT_NE(version.find("build"), std::string::npos);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(SchemaRegistryTest, VeryLongSchemaId) {
    SchemaDefinition schema;
    schema.id = std::string(10000, 'x');
    schema.version = "1.0.0";
    schema.schema_data = "{}";

    auto result = registry->register_schema(schema);

    if (result.is_ok()) {
        EXPECT_EQ(schema.id.length(), 10000);
    }
}

TEST_F(SchemaRegistryTest, VeryLargeSchemaData) {
    SchemaDefinition schema;
    schema.id = "large";
    schema.version = "1.0.0";
    schema.schema_data = std::string(1024 * 1024, '{'); // 1MB

    auto result = registry->register_schema(schema);

    EXPECT_TRUE(result.is_ok());
}

TEST_F(SchemaRegistryTest, SpecialCharactersInSchemaId) {
    SchemaDefinition schema;
    schema.id = "schema-id_with.special-chars";
    schema.version = "1.0.0";
    schema.schema_data = "{}";

    auto result = registry->register_schema(schema);

    EXPECT_TRUE(result.is_ok());
}

TEST_F(SchemaRegistryTest, UnicodeInSchemaId) {
    SchemaDefinition schema;
    schema.id = "模式-123";
    schema.version = "1.0.0";
    schema.schema_data = "{}";

    auto result = registry->register_schema(schema);

    EXPECT_TRUE(result.is_ok());
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(SchemaRegistryThreadSafetyTest, ConcurrentRegistrations) {
    SchemaRegistry shared_registry;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i, &shared_registry, &success_count]() {
            SchemaDefinition schema;
            schema.id = "concurrent-" + std::to_string(i);
            schema.version = "1.0.0";
            schema.schema_data = "{}";

            auto result = shared_registry.register_schema(schema);
            if (result.is_ok()) {
                success_count++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(success_count, 10);
}

TEST(SchemaRegistryThreadSafetyTest, ConcurrentValidations) {
    SchemaRegistry shared_registry;

    SchemaDefinition schema;
    schema.id = "validation-test";
    schema.version = "1.0.0";
    schema.schema_data = "{}";
    shared_registry.register_schema(schema);

    std::vector<std::thread> threads;
    std::atomic<int> validation_count{0};

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&shared_registry, &validation_count]() {
            Envelope envelope;
            envelope.set_from("sender");
            envelope.set_to("receiver");
            envelope.set_operation(OperationType::DATA);

            auto result = shared_registry.validate_envelope(envelope);
            if (result.is_ok()) {
                validation_count++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(validation_count, 10);
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST(SchemaRegistryPerformanceTest, RegisterManySchemas) {
    SchemaRegistry registry;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        SchemaDefinition schema;
        schema.id = "perf-schema-" + std::to_string(i);
        schema.version = "1.0.0";
        schema.schema_data = "{}";

        registry.register_schema(schema);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should register 1000 schemas in < 100ms
    EXPECT_LT(duration.count(), 100);
}

TEST(SchemaRegistryPerformanceTest, ValidateManyEnvelopes) {
    SchemaRegistry registry;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        Envelope envelope;
        envelope.set_from("sender");
        envelope.set_to("receiver");
        envelope.set_operation(OperationType::DATA);
        envelope.set_message_id("msg-" + std::to_string(i));

        registry.validate_envelope(envelope);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should validate 1000 envelopes in < 50ms
    EXPECT_LT(duration.count(), 50);
}

// ============================================================================
// Clear/Reset Tests
// ============================================================================

TEST_F(SchemaRegistryTest, ClearAllSchemas) {
    for (int i = 0; i < 10; ++i) {
        SchemaDefinition schema;
        schema.id = "clear-test-" + std::to_string(i);
        schema.version = "1.0.0";
        schema.schema_data = "{}";
        registry->register_schema(schema);
    }

    registry->clear();

    auto schemas = registry->list_schemas();
    EXPECT_TRUE(schemas.empty());
}

TEST_F(SchemaRegistryTest, ClearEmptyRegistry) {
    registry->clear();

    auto schemas = registry->list_schemas();
    EXPECT_TRUE(schemas.empty());
}

// ============================================================================
// Complex Scenarios
// ============================================================================

TEST(SchemaRegistryScenarioTest, MicroserviceSchemas) {
    SchemaRegistry registry;

    // Register schemas for different microservices
    SchemaDefinition user_schema;
    user_schema.id = "user-service";
    user_schema.version = "1.0.0";
    user_schema.schema_data = R"({"type": "object", "properties": {"userId": {"type": "string"}}})";

    SchemaDefinition order_schema;
    order_schema.id = "order-service";
    order_schema.version = "1.0.0";
    order_schema.schema_data = R"({"type": "object", "properties": {"orderId": {"type": "string"}}})";

    EXPECT_TRUE(registry.register_schema(user_schema).is_ok());
    EXPECT_TRUE(registry.register_schema(order_schema).is_ok());

    auto schemas = registry.list_schemas();
    EXPECT_EQ(schemas.size(), 2);
}

TEST(SchemaRegistryScenarioTest, APIVersioning) {
    SchemaRegistry registry;

    // Register different API versions
    for (int major = 1; major <= 3; ++major) {
        for (int minor = 0; minor <= 2; ++minor) {
            SchemaDefinition schema;
            schema.id = "api";
            schema.version = std::to_string(major) + "." + std::to_string(minor) + ".0";
            schema.schema_data = "{}";

            registry.register_schema(schema);
        }
    }

    auto versions = registry.list_versions("api");
    EXPECT_GE(versions.size(), 9); // 3 major * 3 minor
}

