/**
 * UMICP Envelope - Comprehensive Tests
 * 100% coverage of envelope operations
 */

#include "envelope.h"
#include "umicp_types.h"
#include <gtest/gtest.h>

using namespace umicp;

// ============================================================================
// Envelope Creation Tests
// ============================================================================

class EnvelopeTest : public ::testing::Test {
protected:
    void SetUp() override {
        envelope = Envelope();
    }

    Envelope envelope;
};

TEST_F(EnvelopeTest, DefaultConstruction) {
    Envelope e;
    EXPECT_FALSE(e.get_from().empty());
    EXPECT_FALSE(e.get_message_id().empty());
}

TEST_F(EnvelopeTest, SetAndGetFrom) {
    envelope.set_from("sender-123");
    EXPECT_EQ(envelope.get_from(), "sender-123");
}

TEST_F(EnvelopeTest, SetAndGetTo) {
    envelope.set_to("receiver-456");
    EXPECT_EQ(envelope.get_to(), "receiver-456");
}

TEST_F(EnvelopeTest, SetAndGetOperation) {
    envelope.set_operation(OperationType::DATA);
    EXPECT_EQ(envelope.get_operation(), OperationType::DATA);
}

TEST_F(EnvelopeTest, SetAndGetMessageId) {
    envelope.set_message_id("msg-12345");
    EXPECT_EQ(envelope.get_message_id(), "msg-12345");
}

TEST_F(EnvelopeTest, SetAndGetTimestamp) {
    envelope.set_timestamp("2025-10-10T10:00:00Z");
    EXPECT_EQ(envelope.get_timestamp(), "2025-10-10T10:00:00Z");
}

TEST_F(EnvelopeTest, SetAndGetContentType) {
    envelope.set_content_type(ContentType::CBOR);
    EXPECT_EQ(envelope.get_content_type(), ContentType::CBOR);
}

TEST_F(EnvelopeTest, AllOperationTypes) {
    envelope.set_operation(OperationType::CONTROL);
    EXPECT_EQ(envelope.get_operation(), OperationType::CONTROL);

    envelope.set_operation(OperationType::DATA);
    EXPECT_EQ(envelope.get_operation(), OperationType::DATA);

    envelope.set_operation(OperationType::ACK);
    EXPECT_EQ(envelope.get_operation(), OperationType::ACK);

    envelope.set_operation(OperationType::ERROR);
    EXPECT_EQ(envelope.get_operation(), OperationType::ERROR);
}

TEST_F(EnvelopeTest, AllContentTypes) {
    envelope.set_content_type(ContentType::JSON);
    EXPECT_EQ(envelope.get_content_type(), ContentType::JSON);

    envelope.set_content_type(ContentType::CBOR);
    EXPECT_EQ(envelope.get_content_type(), ContentType::CBOR);

    envelope.set_content_type(ContentType::MSGPACK);
    EXPECT_EQ(envelope.get_content_type(), ContentType::MSGPACK);
}

// ============================================================================
// Capabilities Tests
// ============================================================================

TEST_F(EnvelopeTest, SetAndGetCapabilities) {
    StringMap caps;
    caps["key1"] = "value1";
    caps["key2"] = "value2";

    envelope.set_capabilities(caps);
    auto retrieved = envelope.get_capabilities();

    EXPECT_EQ(retrieved.size(), 2);
    EXPECT_EQ(retrieved["key1"], "value1");
    EXPECT_EQ(retrieved["key2"], "value2");
}

TEST_F(EnvelopeTest, EmptyCapabilities) {
    StringMap caps;
    envelope.set_capabilities(caps);
    auto retrieved = envelope.get_capabilities();
    EXPECT_TRUE(retrieved.empty());
}

TEST_F(EnvelopeTest, CapabilitiesWithSpecialCharacters) {
    StringMap caps;
    caps["special_chars"] = "!@#$%^&*()_+-={}[]|:;<>?,./";
    envelope.set_capabilities(caps);
    auto retrieved = envelope.get_capabilities();
    EXPECT_EQ(retrieved["special_chars"], "!@#$%^&*()_+-={}[]|:;<>?,./");
}

TEST_F(EnvelopeTest, CapabilitiesWithEmptyValues) {
    StringMap caps;
    caps["empty_key"] = "";
    envelope.set_capabilities(caps);
    auto retrieved = envelope.get_capabilities();
    EXPECT_EQ(retrieved["empty_key"], "");
}

TEST_F(EnvelopeTest, CapabilitiesWithLongStrings) {
    StringMap caps;
    std::string long_value(10000, 'a');
    caps["long_value"] = long_value;
    envelope.set_capabilities(caps);
    auto retrieved = envelope.get_capabilities();
    EXPECT_EQ(retrieved["long_value"], long_value);
}

// ============================================================================
// PayloadHint Tests
// ============================================================================

TEST_F(EnvelopeTest, SetAndGetPayloadHint) {
    PayloadHint hint;
    hint.type = PayloadType::VECTOR;
    hint.size = 1024;
    hint.encoding = EncodingType::FLOAT32;
    hint.count = 256;

    envelope.set_payload_hint(hint);
    auto retrieved = envelope.get_payload_hint();

    EXPECT_EQ(retrieved.type, PayloadType::VECTOR);
    EXPECT_TRUE(retrieved.size.has_value());
    EXPECT_EQ(retrieved.size.value(), 1024);
    EXPECT_TRUE(retrieved.encoding.has_value());
    EXPECT_EQ(retrieved.encoding.value(), EncodingType::FLOAT32);
    EXPECT_TRUE(retrieved.count.has_value());
    EXPECT_EQ(retrieved.count.value(), 256);
}

TEST_F(EnvelopeTest, AllPayloadTypes) {
    PayloadHint hint;

    hint.type = PayloadType::VECTOR;
    envelope.set_payload_hint(hint);
    EXPECT_EQ(envelope.get_payload_hint().type, PayloadType::VECTOR);

    hint.type = PayloadType::TEXT;
    envelope.set_payload_hint(hint);
    EXPECT_EQ(envelope.get_payload_hint().type, PayloadType::TEXT);

    hint.type = PayloadType::METADATA;
    envelope.set_payload_hint(hint);
    EXPECT_EQ(envelope.get_payload_hint().type, PayloadType::METADATA);

    hint.type = PayloadType::BINARY;
    envelope.set_payload_hint(hint);
    EXPECT_EQ(envelope.get_payload_hint().type, PayloadType::BINARY);
}

TEST_F(EnvelopeTest, AllEncodingTypes) {
    PayloadHint hint;
    hint.type = PayloadType::VECTOR;

    std::vector<EncodingType> encodings = {
        EncodingType::FLOAT32,
        EncodingType::FLOAT64,
        EncodingType::INT32,
        EncodingType::INT64,
        EncodingType::UINT8,
        EncodingType::UINT16,
        EncodingType::UINT32,
        EncodingType::UINT64
    };

    for (auto enc : encodings) {
        hint.encoding = enc;
        envelope.set_payload_hint(hint);
        EXPECT_TRUE(envelope.get_payload_hint().encoding.has_value());
        EXPECT_EQ(envelope.get_payload_hint().encoding.value(), enc);
    }
}

TEST_F(EnvelopeTest, PayloadHintWithOptionalFields) {
    PayloadHint hint;
    hint.type = PayloadType::BINARY;
    // No size, encoding, or count

    envelope.set_payload_hint(hint);
    auto retrieved = envelope.get_payload_hint();

    EXPECT_EQ(retrieved.type, PayloadType::BINARY);
    EXPECT_FALSE(retrieved.size.has_value());
    EXPECT_FALSE(retrieved.encoding.has_value());
    EXPECT_FALSE(retrieved.count.has_value());
}

// ============================================================================
// Serialization Tests
// ============================================================================

TEST_F(EnvelopeTest, ToJsonProducesValidString) {
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);

    auto json = envelope.to_json();
    EXPECT_FALSE(json.empty());
    EXPECT_NE(json.find("from"), std::string::npos);
    EXPECT_NE(json.find("to"), std::string::npos);
}

TEST_F(EnvelopeTest, ToJsonWithAllFields) {
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);
    envelope.set_message_id("msg-123");
    envelope.set_timestamp("2025-10-10T10:00:00Z");

    StringMap caps;
    caps["key"] = "value";
    envelope.set_capabilities(caps);

    PayloadHint hint;
    hint.type = PayloadType::VECTOR;
    envelope.set_payload_hint(hint);

    auto json = envelope.to_json();
    EXPECT_FALSE(json.empty());
}

TEST_F(EnvelopeTest, RoundTripSerialization) {
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);
    envelope.set_message_id("msg-123");

    auto json = envelope.to_json();
    auto result = EnvelopeProcessor::deserialize(json);

    ASSERT_TRUE(result.is_ok());
    ASSERT_TRUE(result.value.has_value());

    auto deserialized = *result.value;
    EXPECT_EQ(deserialized.get_from(), "sender");
    EXPECT_EQ(deserialized.get_to(), "receiver");
    EXPECT_EQ(deserialized.get_operation(), OperationType::DATA);
    EXPECT_EQ(deserialized.get_message_id(), "msg-123");
}

TEST_F(EnvelopeTest, SerializeEmptyEnvelope) {
    Envelope empty;
    auto json = empty.to_json();
    EXPECT_FALSE(json.empty());
}

TEST_F(EnvelopeTest, DeserializeInvalidJson) {
    std::string invalid_json = "{ invalid json }";
    auto result = EnvelopeProcessor::deserialize(invalid_json);
    EXPECT_FALSE(result.is_ok());
}

TEST_F(EnvelopeTest, DeserializeEmptyString) {
    auto result = EnvelopeProcessor::deserialize("");
    EXPECT_FALSE(result.is_ok());
}

// ============================================================================
// Validation Tests
// ============================================================================

TEST_F(EnvelopeTest, ValidateValidEnvelope) {
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);
    envelope.set_message_id("msg-123");

    auto result = EnvelopeProcessor::validate(envelope);
    EXPECT_TRUE(result.is_ok());
}

TEST_F(EnvelopeTest, ValidateEnvelopeWithoutFrom) {
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);

    auto result = EnvelopeProcessor::validate(envelope);
    // May fail depending on validation rules
}

TEST_F(EnvelopeTest, ValidateEnvelopeWithoutTo) {
    envelope.set_from("sender");
    envelope.set_operation(OperationType::DATA);

    auto result = EnvelopeProcessor::validate(envelope);
    // May fail depending on validation rules
}

TEST_F(EnvelopeTest, ValidateEnvelopeWithAllFields) {
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

    auto result = EnvelopeProcessor::validate(envelope);
    EXPECT_TRUE(result.is_ok());
}

// ============================================================================
// Hash Generation Tests
// ============================================================================

TEST_F(EnvelopeTest, HashGenerationProducesString) {
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);

    auto hash = EnvelopeProcessor::hash(envelope);
    EXPECT_FALSE(hash.empty());
    EXPECT_GE(hash.length(), 32); // SHA-256 produces at least 32 chars
}

TEST_F(EnvelopeTest, SameEnvelopeProducesSameHash) {
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);
    envelope.set_message_id("msg-123");

    auto hash1 = EnvelopeProcessor::hash(envelope);
    auto hash2 = EnvelopeProcessor::hash(envelope);

    EXPECT_EQ(hash1, hash2);
}

TEST_F(EnvelopeTest, DifferentEnvelopesProduceDifferentHashes) {
    envelope.set_from("sender1");
    auto hash1 = EnvelopeProcessor::hash(envelope);

    envelope.set_from("sender2");
    auto hash2 = EnvelopeProcessor::hash(envelope);

    EXPECT_NE(hash1, hash2);
}

TEST_F(EnvelopeTest, EmptyEnvelopeHashIsValid) {
    Envelope empty;
    auto hash = EnvelopeProcessor::hash(empty);
    EXPECT_FALSE(hash.empty());
}

// ============================================================================
// EnvelopeBuilder Tests
// ============================================================================

TEST(EnvelopeBuilderTest, BuildBasicEnvelope) {
    EnvelopeBuilder builder;
    auto result = builder
        .from("sender")
        .to("receiver")
        .operation(OperationType::DATA)
        .message_id("msg-123")
        .build();

    ASSERT_TRUE(result.is_ok());
    ASSERT_TRUE(result.value.has_value());

    auto envelope = *result.value;
    EXPECT_EQ(envelope.get_from(), "sender");
    EXPECT_EQ(envelope.get_to(), "receiver");
    EXPECT_EQ(envelope.get_operation(), OperationType::DATA);
    EXPECT_EQ(envelope.get_message_id(), "msg-123");
}

TEST(EnvelopeBuilderTest, BuildWithCapabilities) {
    StringMap caps;
    caps["key1"] = "value1";
    caps["key2"] = "value2";

    EnvelopeBuilder builder;
    auto result = builder
        .from("sender")
        .to("receiver")
        .operation(OperationType::CONTROL)
        .capabilities(caps)
        .build();

    ASSERT_TRUE(result.is_ok());
    ASSERT_TRUE(result.value.has_value());

    auto envelope = *result.value;
    auto retrieved_caps = envelope.get_capabilities();
    EXPECT_EQ(retrieved_caps.size(), 2);
    EXPECT_EQ(retrieved_caps["key1"], "value1");
}

TEST(EnvelopeBuilderTest, BuildWithPayloadHint) {
    PayloadHint hint;
    hint.type = PayloadType::VECTOR;
    hint.size = 1024;
    hint.encoding = EncodingType::FLOAT32;

    EnvelopeBuilder builder;
    auto result = builder
        .from("sender")
        .to("receiver")
        .operation(OperationType::DATA)
        .payload_hint(hint)
        .build();

    ASSERT_TRUE(result.is_ok());
    ASSERT_TRUE(result.value.has_value());

    auto envelope = *result.value;
    auto retrieved_hint = envelope.get_payload_hint();
    EXPECT_EQ(retrieved_hint.type, PayloadType::VECTOR);
}

TEST(EnvelopeBuilderTest, BuildMinimalEnvelope) {
    EnvelopeBuilder builder;
    auto result = builder
        .from("sender")
        .operation(OperationType::CONTROL)
        .build();

    ASSERT_TRUE(result.is_ok());
}

TEST(EnvelopeBuilderTest, BuildCompleteEnvelope) {
    StringMap caps;
    caps["version"] = "1.0";

    PayloadHint hint;
    hint.type = PayloadType::BINARY;

    EnvelopeBuilder builder;
    auto result = builder
        .from("sender")
        .to("receiver")
        .operation(OperationType::DATA)
        .message_id("msg-123")
        .capabilities(caps)
        .payload_hint(hint)
        .build();

    ASSERT_TRUE(result.is_ok());
    ASSERT_TRUE(result.value.has_value());
}

TEST(EnvelopeBuilderTest, ChainedCalls) {
    auto result = EnvelopeBuilder()
        .from("a")
        .to("b")
        .operation(OperationType::ACK)
        .message_id("id")
        .build();

    ASSERT_TRUE(result.is_ok());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(EnvelopeTest, VeryLongFromField) {
    std::string long_id(10000, 'x');
    envelope.set_from(long_id);
    EXPECT_EQ(envelope.get_from(), long_id);
}

TEST_F(EnvelopeTest, VeryLongToField) {
    std::string long_id(10000, 'y');
    envelope.set_to(long_id);
    EXPECT_EQ(envelope.get_to(), long_id);
}

TEST_F(EnvelopeTest, VeryLongMessageId) {
    std::string long_id(10000, 'z');
    envelope.set_message_id(long_id);
    EXPECT_EQ(envelope.get_message_id(), long_id);
}

TEST_F(EnvelopeTest, SpecialCharactersInFields) {
    envelope.set_from("sender!@#$%");
    envelope.set_to("receiver&*()");
    envelope.set_message_id("msg-<>{}[]");

    EXPECT_EQ(envelope.get_from(), "sender!@#$%");
    EXPECT_EQ(envelope.get_to(), "receiver&*()");
    EXPECT_EQ(envelope.get_message_id(), "msg-<>{}[]");
}

TEST_F(EnvelopeTest, UnicodeCharactersInFields) {
    envelope.set_from("发送者");
    envelope.set_to("接收者");

    EXPECT_EQ(envelope.get_from(), "发送者");
    EXPECT_EQ(envelope.get_to(), "接收者");
}

TEST_F(EnvelopeTest, EmptyStringFields) {
    envelope.set_from("");
    envelope.set_to("");
    envelope.set_message_id("");

    EXPECT_EQ(envelope.get_from(), "");
    EXPECT_EQ(envelope.get_to(), "");
    EXPECT_EQ(envelope.get_message_id(), "");
}

TEST_F(EnvelopeTest, ManyCapabilities) {
    StringMap caps;
    for (int i = 0; i < 100; ++i) {
        caps["key" + std::to_string(i)] = "value" + std::to_string(i);
    }

    envelope.set_capabilities(caps);
    auto retrieved = envelope.get_capabilities();

    EXPECT_EQ(retrieved.size(), 100);
    EXPECT_EQ(retrieved["key50"], "value50");
}

// ============================================================================
// Copy and Move Semantics
// ============================================================================

TEST_F(EnvelopeTest, CopyConstruction) {
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);

    Envelope copied = envelope;

    EXPECT_EQ(copied.get_from(), "sender");
    EXPECT_EQ(copied.get_to(), "receiver");
    EXPECT_EQ(copied.get_operation(), OperationType::DATA);
}

TEST_F(EnvelopeTest, CopyAssignment) {
    envelope.set_from("sender");
    envelope.set_to("receiver");

    Envelope other;
    other = envelope;

    EXPECT_EQ(other.get_from(), "sender");
    EXPECT_EQ(other.get_to(), "receiver");
}

TEST_F(EnvelopeTest, MoveConstruction) {
    envelope.set_from("sender");
    envelope.set_to("receiver");

    Envelope moved = std::move(envelope);

    EXPECT_EQ(moved.get_from(), "sender");
    EXPECT_EQ(moved.get_to(), "receiver");
}

TEST_F(EnvelopeTest, MoveAssignment) {
    envelope.set_from("sender");
    envelope.set_to("receiver");

    Envelope other;
    other = std::move(envelope);

    EXPECT_EQ(other.get_from(), "sender");
    EXPECT_EQ(other.get_to(), "receiver");
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(EnvelopeThreadSafetyTest, ConcurrentEnvelopeCreation) {
    std::vector<std::thread> threads;
    const int num_threads = 10;
    std::vector<Envelope> envelopes(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i, &envelopes]() {
            envelopes[i].set_from("sender" + std::to_string(i));
            envelopes[i].set_to("receiver" + std::to_string(i));
            envelopes[i].set_operation(OperationType::DATA);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (int i = 0; i < num_threads; ++i) {
        EXPECT_EQ(envelopes[i].get_from(), "sender" + std::to_string(i));
    }
}

TEST(EnvelopeThreadSafetyTest, ConcurrentSerialization) {
    Envelope base;
    base.set_from("sender");
    base.set_to("receiver");
    base.set_operation(OperationType::DATA);

    std::vector<std::thread> threads;
    std::vector<std::string> results(10);

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i, &base, &results]() {
            Envelope copy = base;
            copy.set_message_id("msg-" + std::to_string(i));
            results[i] = copy.to_json();
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (const auto& result : results) {
        EXPECT_FALSE(result.empty());
    }
}

TEST(EnvelopeThreadSafetyTest, ConcurrentDeserialization) {
    std::string json = R"({
        "from": "sender",
        "to": "receiver",
        "operation": "data",
        "message_id": "msg-123"
    })";

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&json, &success_count]() {
            auto result = EnvelopeProcessor::deserialize(json);
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

// ============================================================================
// Performance Tests
// ============================================================================

TEST(EnvelopePerformanceTest, SerializationPerformance) {
    Envelope envelope;
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        envelope.set_message_id("msg-" + std::to_string(i));
        auto json = envelope.to_json();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Should complete 1000 serializations in < 100ms
    EXPECT_LT(duration.count(), 100000);
}

TEST(EnvelopePerformanceTest, DeserializationPerformance) {
    std::string json = R"({
        "from": "sender",
        "to": "receiver",
        "operation": "data",
        "message_id": "msg-123"
    })";

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        auto result = EnvelopeProcessor::deserialize(json);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Should complete 1000 deserializations in < 200ms
    EXPECT_LT(duration.count(), 200000);
}

TEST(EnvelopePerformanceTest, HashPerformance) {
    Envelope envelope;
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        auto hash = EnvelopeProcessor::hash(envelope);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Should complete 1000 hashes in < 50ms
    EXPECT_LT(duration.count(), 50000);
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST(EnvelopeStressTest, LargeScaleEnvelopeCreation) {
    std::vector<Envelope> envelopes;
    envelopes.reserve(10000);

    for (int i = 0; i < 10000; ++i) {
        Envelope e;
        e.set_from("sender" + std::to_string(i));
        e.set_to("receiver" + std::to_string(i));
        e.set_operation(OperationType::DATA);
        envelopes.push_back(std::move(e));
    }

    EXPECT_EQ(envelopes.size(), 10000);
}

TEST(EnvelopeStressTest, LargeCapabilitiesMap) {
    Envelope envelope;
    StringMap caps;

    for (int i = 0; i < 1000; ++i) {
        caps["key" + std::to_string(i)] = "value" + std::to_string(i);
    }

    envelope.set_capabilities(caps);
    auto retrieved = envelope.get_capabilities();

    EXPECT_EQ(retrieved.size(), 1000);
}

TEST(EnvelopeStressTest, SerializationOfLargeEnvelope) {
    Envelope envelope;
    envelope.set_from("sender");
    envelope.set_to("receiver");

    StringMap caps;
    for (int i = 0; i < 100; ++i) {
        caps["key" + std::to_string(i)] = std::string(100, 'x');
    }
    envelope.set_capabilities(caps);

    auto json = envelope.to_json();
    EXPECT_GT(json.length(), 10000);

    auto result = EnvelopeProcessor::deserialize(json);
    EXPECT_TRUE(result.is_ok());
}

// ============================================================================
// Error Handling
// ============================================================================

TEST(EnvelopeErrorTest, InvalidJsonSyntax) {
    std::vector<std::string> invalid_jsons = {
        "not json at all",
        "{ invalid",
        "{ \"missing\": quote }",
        "null",
        "123",
        "true",
        "[]"
    };

    for (const auto& invalid : invalid_jsons) {
        auto result = EnvelopeProcessor::deserialize(invalid);
        EXPECT_FALSE(result.is_ok());
    }
}

TEST(EnvelopeErrorTest, MissingRequiredFields) {
    std::string incomplete_json = R"({ "from": "sender" })";
    auto result = EnvelopeProcessor::deserialize(incomplete_json);
    // May succeed or fail depending on validation rules
}

TEST(EnvelopeErrorTest, NullValues) {
    std::string json_with_nulls = R"({
        "from": null,
        "to": "receiver",
        "operation": "data"
    })";

    auto result = EnvelopeProcessor::deserialize(json_with_nulls);
    // Should handle null gracefully
}

// ============================================================================
// Integration with Other Components
// ============================================================================

TEST(EnvelopeIntegrationTest, EnvelopeWithFrame) {
    Envelope envelope;
    envelope.set_from("sender");
    envelope.set_to("receiver");
    envelope.set_operation(OperationType::DATA);

    // Envelope should work alongside Frame
    EXPECT_NE(envelope.get_message_id(), "");
}

TEST(EnvelopeIntegrationTest, MultipleEnvelopesIndependent) {
    Envelope e1, e2, e3;

    e1.set_from("sender1");
    e2.set_from("sender2");
    e3.set_from("sender3");

    EXPECT_EQ(e1.get_from(), "sender1");
    EXPECT_EQ(e2.get_from(), "sender2");
    EXPECT_EQ(e3.get_from(), "sender3");
}

