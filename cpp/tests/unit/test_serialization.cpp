/**
 * UMICP Serialization Unit Tests
 * Testing JSON and binary serialization/deserialization
 */

#include <gtest/gtest.h>
#include "serialization.h"
#include "../utils/test_helpers.h"

using namespace umicp;
using namespace umicp::testing;

class SerializationTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();

        // Create test envelope
        test_envelope_ = TestHelpers::create_test_envelope("test-sender", "test-receiver");
        test_envelope_.capabilities = StringMap{
            {"command", "ping"},
            {"version", "1.0"},
            {"binary_support", "true"}
        };

        // Create test frame
        ByteBuffer test_payload = TestHelpers::generate_random_data(100);
        test_frame_ = TestHelpers::create_test_frame(12345, 1, test_payload);

        // Test data for hash/base64
        test_data_ = TestHelpers::generate_random_data(256);
        test_string_ = "Hello, UMICP World! 🚀";
    }

    Envelope test_envelope_;
    Frame test_frame_;
    ByteBuffer test_data_;
    std::string test_string_;
};

// ===============================================
// JSON Envelope Serialization Tests
// ===============================================

TEST_F(SerializationTest, JsonEnvelope_BasicSerialization) {
    auto result = JsonSerializer::serialize_envelope(test_envelope_);

    ASSERT_TRUE(result.is_success());
    ASSERT_FALSE(result.value->empty());

    const std::string& json = *result.value;

    // Check that all required fields are present
    EXPECT_NE(json.find("\"v\":"), std::string::npos);
    EXPECT_NE(json.find("\"msg_id\":"), std::string::npos);
    EXPECT_NE(json.find("\"ts\":"), std::string::npos);
    EXPECT_NE(json.find("\"from\":"), std::string::npos);
    EXPECT_NE(json.find("\"to\":"), std::string::npos);
    EXPECT_NE(json.find("\"op\":"), std::string::npos);

    // Check capabilities
    EXPECT_NE(json.find("\"capabilities\""), std::string::npos);
    EXPECT_NE(json.find("\"command\""), std::string::npos);
    EXPECT_NE(json.find("\"ping\""), std::string::npos);

    std::cout << "  📄 Serialized JSON size: " << json.size() << " bytes" << std::endl;
}

TEST_F(SerializationTest, JsonEnvelope_Deserialization) {
    // First serialize
    auto serialize_result = JsonSerializer::serialize_envelope(test_envelope_);
    ASSERT_TRUE(serialize_result.is_success());

    // Then deserialize
    auto deserialize_result = JsonSerializer::deserialize_envelope(*serialize_result.value);

    ASSERT_TRUE(deserialize_result.is_success());

    const Envelope& deserialized = *deserialize_result.value;

    // Verify core fields
    EXPECT_EQ(deserialized.version, test_envelope_.version);
    EXPECT_EQ(deserialized.msg_id, test_envelope_.msg_id);
    EXPECT_EQ(deserialized.from, test_envelope_.from);
    EXPECT_EQ(deserialized.to, test_envelope_.to);
    EXPECT_EQ(deserialized.op, test_envelope_.op);
}

TEST_F(SerializationTest, JsonEnvelope_RoundTrip) {
    // Multiple round trips should be identical
    auto json1 = JsonSerializer::serialize_envelope(test_envelope_);
    auto env1 = JsonSerializer::deserialize_envelope(*json1.value);
    auto json2 = JsonSerializer::serialize_envelope(*env1.value);
    auto env2 = JsonSerializer::deserialize_envelope(*json2.value);

    ASSERT_TRUE(json1.is_success());
    ASSERT_TRUE(env1.is_success());
    ASSERT_TRUE(json2.is_success());
    ASSERT_TRUE(env2.is_success());

    ASSERT_ENVELOPES_EQUAL(*env1.value, *env2.value);
}

TEST_F(SerializationTest, JsonEnvelope_MinimalEnvelope) {
    // Test with minimal envelope (no optional fields)
    Envelope minimal;
    minimal.version = "1.0";
    minimal.msg_id = "minimal-msg";
    minimal.ts = "2025-09-10T03:00:00.000Z";
    minimal.from = "minimal-sender";
    minimal.to = "minimal-receiver";
    minimal.op = OperationType::CONTROL;

    auto result = JsonSerializer::serialize_envelope(minimal);
    ASSERT_TRUE(result.is_success());

    auto deserialize_result = JsonSerializer::deserialize_envelope(*result.value);
    ASSERT_TRUE(deserialize_result.is_success());

    ASSERT_ENVELOPES_EQUAL(minimal, *deserialize_result.value);
}

TEST_F(SerializationTest, JsonEnvelope_ErrorOperationType) {
    Envelope error_envelope = test_envelope_;
    error_envelope.op = OperationType::ERROR;

    auto result = JsonSerializer::serialize_envelope(error_envelope);
    ASSERT_TRUE(result.is_success());

    const std::string& json = *result.value;
    EXPECT_NE(json.find("\"op\":3"), std::string::npos); // ERROR = 3
}

// ===============================================
// JSON Canonicalization Tests
// ===============================================

TEST_F(SerializationTest, JsonCanonicalization_WhitespaceRemoval) {
    std::string messy_json = "{\n  \"key1\" : \"value1\" ,\n  \"key2\" : 123  \n}";

    auto result = JsonSerializer::canonicalize_json(messy_json);

    ASSERT_TRUE(result.is_success());

    const std::string& canonical = *result.value;
    EXPECT_EQ(canonical.find('\n'), std::string::npos);
    EXPECT_EQ(canonical.find("  "), std::string::npos);

    std::cout << "  📄 Original: " << messy_json.size() << " bytes" << std::endl;
    std::cout << "  📄 Canonical: " << canonical.size() << " bytes" << std::endl;
}

TEST_F(SerializationTest, JsonCanonicalization_StringPreservation) {
    std::string json_with_strings = R"({"text":"This has   spaces\nand\tnewlines"})";

    auto result = JsonSerializer::canonicalize_json(json_with_strings);

    ASSERT_TRUE(result.is_success());

    // Whitespace inside strings should be preserved
    EXPECT_NE(result.value->find("   spaces"), std::string::npos);
    EXPECT_NE(result.value->find("\\n"), std::string::npos);
    EXPECT_NE(result.value->find("\\t"), std::string::npos);
}

// ===============================================
// Binary Frame Serialization Tests
// ===============================================

TEST_F(SerializationTest, BinaryFrame_Serialization) {
    auto result = BinarySerializer::serialize_frame(test_frame_);

    ASSERT_TRUE(result.is_success());

    const ByteBuffer& data = *result.value;

    // Check frame size
    size_t expected_size = UMICP_FRAME_HEADER_SIZE + test_frame_.payload.size();
    EXPECT_EQ(data.size(), expected_size);

    // Check header fields
    EXPECT_EQ(data[0], test_frame_.header.version);
    EXPECT_EQ(data[1], test_frame_.header.type);

    std::cout << "  📦 Frame size: " << TestHelpers::format_bytes(data.size()) << std::endl;
}

TEST_F(SerializationTest, BinaryFrame_Deserialization) {
    // Serialize first
    auto serialize_result = BinarySerializer::serialize_frame(test_frame_);
    ASSERT_TRUE(serialize_result.is_success());

    // Then deserialize
    auto deserialize_result = BinarySerializer::deserialize_frame(*serialize_result.value);

    ASSERT_TRUE(deserialize_result.is_success());

    const Frame& deserialized = *deserialize_result.value;

    // Verify header
    EXPECT_EQ(deserialized.header.version, test_frame_.header.version);
    EXPECT_EQ(deserialized.header.type, test_frame_.header.type);
    EXPECT_EQ(deserialized.header.flags, test_frame_.header.flags);
    EXPECT_EQ(deserialized.header.stream_id, test_frame_.header.stream_id);
    EXPECT_EQ(deserialized.header.sequence, test_frame_.header.sequence);

    // Verify payload
    ASSERT_BUFFERS_EQUAL(deserialized.payload, test_frame_.payload);
}

TEST_F(SerializationTest, BinaryFrame_RoundTrip) {
    auto data1 = BinarySerializer::serialize_frame(test_frame_);
    auto frame1 = BinarySerializer::deserialize_frame(*data1.value);
    auto data2 = BinarySerializer::serialize_frame(*frame1.value);
    auto frame2 = BinarySerializer::deserialize_frame(*data2.value);

    ASSERT_TRUE(data1.is_success());
    ASSERT_TRUE(frame1.is_success());
    ASSERT_TRUE(data2.is_success());
    ASSERT_TRUE(frame2.is_success());

    ASSERT_FRAMES_EQUAL(*frame1.value, *frame2.value);
    ASSERT_BUFFERS_EQUAL(*data1.value, *data2.value);
}

TEST_F(SerializationTest, BinaryFrame_LargePayload) {
    // Test with 1MB payload
    ByteBuffer large_payload = TestHelpers::generate_random_data(1024 * 1024);
    Frame large_frame = TestHelpers::create_test_frame(99999, 42, large_payload);

    auto serialize_result = BinarySerializer::serialize_frame(large_frame);
    ASSERT_TRUE(serialize_result.is_success());

    auto deserialize_result = BinarySerializer::deserialize_frame(*serialize_result.value);
    ASSERT_TRUE(deserialize_result.is_success());

    ASSERT_FRAMES_EQUAL(large_frame, *deserialize_result.value);

    std::cout << "  📦 Large frame: " << TestHelpers::format_bytes(serialize_result.value->size()) << std::endl;
}

TEST_F(SerializationTest, BinaryFrame_EmptyPayload) {
    Frame empty_frame = TestHelpers::create_test_frame(1, 1, ByteBuffer{});

    auto serialize_result = BinarySerializer::serialize_frame(empty_frame);
    ASSERT_TRUE(serialize_result.is_success());

    // Should be exactly header size
    EXPECT_EQ(serialize_result.value->size(), UMICP_FRAME_HEADER_SIZE);

    auto deserialize_result = BinarySerializer::deserialize_frame(*serialize_result.value);
    ASSERT_TRUE(deserialize_result.is_success());

    EXPECT_TRUE(deserialize_result.value->payload.empty());
}

TEST_F(SerializationTest, BinaryFrame_InvalidData) {
    // Test with invalid frame data
    ByteBuffer invalid_data = {0x01, 0x02, 0x03}; // Too small

    auto result = BinarySerializer::deserialize_frame(invalid_data);

    EXPECT_FALSE(result.is_success());
    EXPECT_EQ(result.code, ErrorCode::INVALID_FRAME);
}

// ===============================================
// Hash Utilities Tests
// ===============================================

TEST_F(SerializationTest, HashUtils_SHA256_ConsistentHashing) {
    auto hash1 = HashUtils::sha256_hex(test_data_);
    auto hash2 = HashUtils::sha256_hex(test_data_);

    ASSERT_TRUE(hash1.is_success());
    ASSERT_TRUE(hash2.is_success());

    EXPECT_EQ(*hash1.value, *hash2.value);
    EXPECT_EQ(hash1.value->length(), 64); // SHA-256 = 256 bits = 64 hex chars

    std::cout << "  🔒 SHA-256: " << hash1.value->substr(0, 16) << "..." << std::endl;
}

TEST_F(SerializationTest, HashUtils_SHA256_StringVsBuffer) {
    ByteBuffer string_as_bytes(test_string_.begin(), test_string_.end());

    auto string_hash = HashUtils::sha256_hex(test_string_);
    auto buffer_hash = HashUtils::sha256_hex(string_as_bytes);

    ASSERT_TRUE(string_hash.is_success());
    ASSERT_TRUE(buffer_hash.is_success());

    EXPECT_EQ(*string_hash.value, *buffer_hash.value);
}

TEST_F(SerializationTest, HashUtils_SHA256_EmptyData) {
    ByteBuffer empty_data;

    auto result = HashUtils::sha256_hex(empty_data);

    ASSERT_TRUE(result.is_success());
    EXPECT_EQ(result.value->length(), 64);

    // SHA-256 of empty string is known
    EXPECT_EQ(*result.value, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST_F(SerializationTest, HashUtils_SHA256_RawFormat) {
    auto raw_result = HashUtils::sha256_raw(test_data_);

    ASSERT_TRUE(raw_result.is_success());
    EXPECT_EQ(raw_result.value->size(), 32); // 256 bits = 32 bytes

    // Convert to hex and compare with hex function
    auto hex_result = HashUtils::sha256_hex(test_data_);
    ASSERT_TRUE(hex_result.is_success());

    // Manual hex conversion for verification
    std::stringstream hex_stream;
    hex_stream << std::hex << std::setfill('0');
    for (uint8_t byte : *raw_result.value) {
        hex_stream << std::setw(2) << static_cast<int>(byte);
    }

    EXPECT_EQ(hex_stream.str(), *hex_result.value);
}

// ===============================================
// Base64 Utilities Tests
// ===============================================

TEST_F(SerializationTest, Base64Utils_BasicEncoding) {
    ByteBuffer simple_data = {'H', 'e', 'l', 'l', 'o'};

    auto result = Base64Utils::encode(simple_data);

    ASSERT_TRUE(result.is_success());
    EXPECT_EQ(*result.value, "SGVsbG8=");
}

TEST_F(SerializationTest, Base64Utils_BasicDecoding) {
    std::string encoded = "SGVsbG8=";

    auto result = Base64Utils::decode(encoded);

    ASSERT_TRUE(result.is_success());

    std::string decoded(result.value->begin(), result.value->end());
    EXPECT_EQ(decoded, "Hello");
}

TEST_F(SerializationTest, Base64Utils_RoundTrip) {
    auto encoded = Base64Utils::encode(test_data_);
    ASSERT_TRUE(encoded.is_success());

    auto decoded = Base64Utils::decode(*encoded.value);
    ASSERT_TRUE(decoded.is_success());

    ASSERT_BUFFERS_EQUAL(test_data_, *decoded.value);

    std::cout << "  📄 Original: " << TestHelpers::format_bytes(test_data_.size()) << std::endl;
    std::cout << "  📄 Base64: " << TestHelpers::format_bytes(encoded.value->size()) << std::endl;
}

TEST_F(SerializationTest, Base64Utils_EmptyData) {
    ByteBuffer empty_data;

    auto encoded = Base64Utils::encode(empty_data);
    ASSERT_TRUE(encoded.is_success());
    EXPECT_TRUE(encoded.value->empty());

    auto decoded = Base64Utils::decode(*encoded.value);
    ASSERT_TRUE(decoded.is_success());
    EXPECT_TRUE(decoded.value->empty());
}

TEST_F(SerializationTest, Base64Utils_PaddingCases) {
    // Test different padding scenarios
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"A", "QQ=="}, // 1 byte -> 2 padding chars
        {"AB", "QUI="}, // 2 bytes -> 1 padding char
        {"ABC", "QUJD"} // 3 bytes -> no padding
    };

    for (const auto& [input, expected] : test_cases) {
        ByteBuffer data(input.begin(), input.end());
        auto encoded = Base64Utils::encode(data);

        ASSERT_TRUE(encoded.is_success());
        EXPECT_EQ(*encoded.value, expected);

        auto decoded = Base64Utils::decode(*encoded.value);
        ASSERT_TRUE(decoded.is_success());

        std::string result(decoded.value->begin(), decoded.value->end());
        EXPECT_EQ(result, input);
    }
}

TEST_F(SerializationTest, Base64Utils_InvalidBase64) {
    std::string invalid_base64 = "Invalid@Base64!";

    auto result = Base64Utils::decode(invalid_base64);

    EXPECT_FALSE(result.is_success());
    EXPECT_EQ(result.code, ErrorCode::SERIALIZATION_FAILED);
}

// ===============================================
// CBOR/MessagePack Placeholder Tests
// ===============================================

TEST_F(SerializationTest, CBOR_NotImplementedYet) {
    JsonObject test_obj = {{"key", "value"}, {"number", "42"}};

    auto encode_result = BinarySerializer::encode_cbor(test_obj);
    EXPECT_TRUE(encode_result.is_success());
    EXPECT_GT(encode_result.value->size(), 0);

    // Test round-trip
    auto decode_result = BinarySerializer::decode_cbor(*encode_result.value);
    EXPECT_TRUE(decode_result.is_success());
    EXPECT_EQ(decode_result.value->size(), test_obj.size());
    EXPECT_EQ((*decode_result.value)["key"], "value");
    EXPECT_EQ((*decode_result.value)["number"], "42");
}

TEST_F(SerializationTest, MessagePack_NotImplementedYet) {
    JsonObject test_obj = {{"key", "value"}, {"number", "42"}};

    auto encode_result = BinarySerializer::encode_msgpack(test_obj);
    EXPECT_TRUE(encode_result.is_success());
    EXPECT_GT(encode_result.value->size(), 0);

    // Test round-trip
    auto decode_result = BinarySerializer::decode_msgpack(*encode_result.value);
    EXPECT_TRUE(decode_result.is_success());
    EXPECT_EQ(decode_result.value->size(), test_obj.size());
    EXPECT_EQ((*decode_result.value)["key"], "value");
    EXPECT_EQ((*decode_result.value)["number"], "42");
}

// ===============================================
// Performance Tests
// ===============================================

TEST_F(SerializationTest, Performance_JsonSerialization) {
    auto time_serialize = TestHelpers::benchmark_function([&]() {
        JsonSerializer::serialize_envelope(test_envelope_);
    }, 1000);

    auto json_result = JsonSerializer::serialize_envelope(test_envelope_);
    auto time_deserialize = TestHelpers::benchmark_function([&]() {
        JsonSerializer::deserialize_envelope(*json_result.value);
    }, 1000);

    std::cout << "  📊 JSON Serialize: " << TestHelpers::format_duration(time_serialize) << std::endl;
    std::cout << "  📊 JSON Deserialize: " << TestHelpers::format_duration(time_deserialize) << std::endl;
}

TEST_F(SerializationTest, Performance_BinaryFrames) {
    auto time_serialize = TestHelpers::benchmark_function([&]() {
        BinarySerializer::serialize_frame(test_frame_);
    }, 1000);

    auto frame_result = BinarySerializer::serialize_frame(test_frame_);
    auto time_deserialize = TestHelpers::benchmark_function([&]() {
        BinarySerializer::deserialize_frame(*frame_result.value);
    }, 1000);

    std::cout << "  📊 Frame Serialize: " << TestHelpers::format_duration(time_serialize) << std::endl;
    std::cout << "  📊 Frame Deserialize: " << TestHelpers::format_duration(time_deserialize) << std::endl;
}
