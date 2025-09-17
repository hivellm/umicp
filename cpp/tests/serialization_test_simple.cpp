/**
 * Simple Serialization Test
 */

#include <gtest/gtest.h>
#include <iostream>
#include <vector>

// Include UMICP headers
#include "umicp_types.h"
#include "serialization.h"

using namespace umicp;

TEST(SerializationSimple, SHA256Hash) {
    std::string test_string = "Hello, UMICP!";

    auto hash_result = HashUtils::sha256_hex(test_string);

    ASSERT_TRUE(hash_result.is_success());
    EXPECT_EQ(hash_result.value->length(), 64); // SHA-256 = 64 hex chars

    // Hash should be consistent
    auto hash_result2 = HashUtils::sha256_hex(test_string);
    ASSERT_TRUE(hash_result2.is_success());
    EXPECT_EQ(*hash_result.value, *hash_result2.value);

    std::cout << "✅ SHA-256 hash test passed!" << std::endl;
    std::cout << "   Hash: " << hash_result.value->substr(0, 16) << "..." << std::endl;
}

TEST(SerializationSimple, Base64Encoding) {
    std::string test_string = "Hello, UMICP World!";
    ByteBuffer test_data(test_string.begin(), test_string.end());

    auto encode_result = Base64Utils::encode(test_data);
    ASSERT_TRUE(encode_result.is_success());

    auto decode_result = Base64Utils::decode(*encode_result.value);
    ASSERT_TRUE(decode_result.is_success());

    std::string decoded_string(decode_result.value->begin(), decode_result.value->end());
    EXPECT_EQ(decoded_string, test_string);

    std::cout << "✅ Base64 encoding/decoding test passed!" << std::endl;
    std::cout << "   Original: " << test_string << std::endl;
    std::cout << "   Base64:   " << *encode_result.value << std::endl;
}

TEST(SerializationSimple, EnvelopeSerialization) {
    // Create test envelope
    Envelope test_envelope;
    test_envelope.version = "1.0";
    test_envelope.msg_id = "test-123";
    test_envelope.from = "test-sender";
    test_envelope.to = "test-receiver";
    test_envelope.op = OperationType::CONTROL;
    test_envelope.ts = "2025-09-10T00:00:00.000Z";

    auto serialize_result = JsonSerializer::serialize_envelope(test_envelope);
    ASSERT_TRUE(serialize_result.is_success());
    EXPECT_FALSE(serialize_result.value->empty());

    auto deserialize_result = JsonSerializer::deserialize_envelope(*serialize_result.value);
    ASSERT_TRUE(deserialize_result.is_success());

    EXPECT_EQ(deserialize_result.value->version, test_envelope.version);
    EXPECT_EQ(deserialize_result.value->msg_id, test_envelope.msg_id);
    EXPECT_EQ(deserialize_result.value->from, test_envelope.from);
    EXPECT_EQ(deserialize_result.value->to, test_envelope.to);
    EXPECT_EQ(deserialize_result.value->op, test_envelope.op);

    std::cout << "✅ Envelope serialization test passed!" << std::endl;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    std::cout << "📄 Starting Serialization Test" << std::endl;
    int result = RUN_ALL_TESTS();
    std::cout << "🏁 Serialization Tests Complete" << std::endl;
    return result;
}
