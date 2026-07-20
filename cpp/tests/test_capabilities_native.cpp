#include <gtest/gtest.h>
#include "envelope.h"
#include "tool_discovery.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace umicp;

// ============================================================================
// CAPABILITIES NATIVE TYPES - COMPREHENSIVE TESTS
// ============================================================================

class CapabilitiesNativeTest : public ::testing::Test {
protected:
    Envelope envelope;

    void SetUp() override {
        envelope.from = "test-sender";
        envelope.to = "test-receiver";
        envelope.operation = OperationType::DATA;
    }
};

// Integer tests
TEST_F(CapabilitiesNativeTest, IntegerPositive) {
    envelope.capabilities = {{"max_tokens", 100}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_EQ(deserialized.capabilities.value().at("max_tokens").get<int>(), 100);
}

TEST_F(CapabilitiesNativeTest, IntegerNegative) {
    envelope.capabilities = {{"offset", -50}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_EQ(deserialized.capabilities.value().at("offset").get<int>(), -50);
}

TEST_F(CapabilitiesNativeTest, IntegerZero) {
    envelope.capabilities = {{"count", 0}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_EQ(deserialized.capabilities.value().at("count").get<int>(), 0);
}

TEST_F(CapabilitiesNativeTest, IntegerLarge) {
    envelope.capabilities = {{"max_value", 2147483647}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_EQ(deserialized.capabilities.value().at("max_value").get<int>(), 2147483647);
}

// Float/Double tests
TEST_F(CapabilitiesNativeTest, FloatDecimal) {
    envelope.capabilities = {{"temperature", 0.7}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_NEAR(deserialized.capabilities.value().at("temperature").get<double>(), 0.7, 0.001);
}

TEST_F(CapabilitiesNativeTest, FloatScientific) {
    envelope.capabilities = {{"learning_rate", 1.5e-4}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_NEAR(deserialized.capabilities.value().at("learning_rate").get<double>(), 1.5e-4, 1e-10);
}

TEST_F(CapabilitiesNativeTest, FloatZero) {
    envelope.capabilities = {{"threshold", 0.0}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_DOUBLE_EQ(deserialized.capabilities.value().at("threshold").get<double>(), 0.0);
}

// Boolean tests
TEST_F(CapabilitiesNativeTest, BooleanTrue) {
    envelope.capabilities = {{"enabled", true}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_TRUE(deserialized.capabilities.value().at("enabled").get<bool>());
}

TEST_F(CapabilitiesNativeTest, BooleanFalse) {
    envelope.capabilities = {{"disabled", false}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_FALSE(deserialized.capabilities.value().at("disabled").get<bool>());
}

// String tests
TEST_F(CapabilitiesNativeTest, StringSimple) {
    envelope.capabilities = {{"model", json("gpt-4")}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_EQ(deserialized.capabilities.value().at("model").get<std::string>(), "gpt-4");
}

TEST_F(CapabilitiesNativeTest, StringEmpty) {
    envelope.capabilities = {{"empty", json("")}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_EQ(deserialized.capabilities.value().at("empty").get<std::string>(), "");
}

TEST_F(CapabilitiesNativeTest, StringUnicode) {
    envelope.capabilities = {{"message", json("Hello 世界 🌍")}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_EQ(deserialized.capabilities.value().at("message").get<std::string>(), "Hello 世界 🌍");
}

// Array tests
TEST_F(CapabilitiesNativeTest, ArrayStrings) {
    envelope.capabilities = {{"models", json::array({"gpt-4", "claude-3", "llama-3"})}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto arr = deserialized.capabilities.value().at("models");
    EXPECT_TRUE(arr.is_array());
    EXPECT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0].get<std::string>(), "gpt-4");
}

TEST_F(CapabilitiesNativeTest, ArrayIntegers) {
    envelope.capabilities = {{"limits", json::array({10, 20, 30, 40})}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto arr = deserialized.capabilities.value().at("limits");
    EXPECT_EQ(arr.size(), 4);
    EXPECT_EQ(arr[2].get<int>(), 30);
}

TEST_F(CapabilitiesNativeTest, ArrayMixed) {
    envelope.capabilities = {{"mixed", json::array({1, "two", true, 4.5})}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto arr = deserialized.capabilities.value().at("mixed");
    EXPECT_EQ(arr[0].get<int>(), 1);
    EXPECT_EQ(arr[1].get<std::string>(), "two");
    EXPECT_TRUE(arr[2].get<bool>());
    EXPECT_NEAR(arr[3].get<double>(), 4.5, 0.001);
}

TEST_F(CapabilitiesNativeTest, ArrayEmpty) {
    envelope.capabilities = {{"empty_array", json::array()}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto arr = deserialized.capabilities.value().at("empty_array");
    EXPECT_TRUE(arr.is_array());
    EXPECT_EQ(arr.size(), 0);
}

TEST_F(CapabilitiesNativeTest, ArrayNested) {
    envelope.capabilities = {{"nested", json::array({json::array({1, 2}), json::array({3, 4})})}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto arr = deserialized.capabilities.value().at("nested");
    EXPECT_EQ(arr[0][1].get<int>(), 2);
    EXPECT_EQ(arr[1][0].get<int>(), 3);
}

// Object tests
TEST_F(CapabilitiesNativeTest, ObjectSimple) {
    json config = {{"timeout", 30}, {"retries", 3}};
    envelope.capabilities = {{"config", config}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto obj = deserialized.capabilities.value().at("config");
    EXPECT_EQ(obj["timeout"].get<int>(), 30);
    EXPECT_EQ(obj["retries"].get<int>(), 3);
}

TEST_F(CapabilitiesNativeTest, ObjectNested) {
    json auth = {
        {"type", "oauth"},
        {"credentials", {
            {"client_id", "abc123"},
            {"scope", json::array({"read", "write"})}
        }}
    };
    envelope.capabilities = {{"auth", auth}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto obj = deserialized.capabilities.value().at("auth");
    EXPECT_EQ(obj["type"].get<std::string>(), "oauth");
    EXPECT_EQ(obj["credentials"]["client_id"].get<std::string>(), "abc123");
}

TEST_F(CapabilitiesNativeTest, ObjectEmpty) {
    envelope.capabilities = {{"empty_obj", json::object()}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto obj = deserialized.capabilities.value().at("empty_obj");
    EXPECT_TRUE(obj.is_object());
    EXPECT_EQ(obj.size(), 0);
}

// Null tests
TEST_F(CapabilitiesNativeTest, NullValue) {
    envelope.capabilities = {{"optional", nullptr}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    EXPECT_TRUE(deserialized.capabilities.value().at("optional").is_null());
}

// Complex mixed types
TEST_F(CapabilitiesNativeTest, ComplexMixedTypes) {
    envelope.capabilities = {
        {"string_val", json("test")},
        {"int_val", 42},
        {"float_val", 3.14},
        {"bool_val", true},
        {"null_val", nullptr},
        {"array_val", json::array({1, 2, 3})},
        {"object_val", json::object({{"key", "value"}})}
    };

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto caps = deserialized.capabilities.value();
    EXPECT_EQ(caps["string_val"].get<std::string>(), "test");
    EXPECT_EQ(caps["int_val"].get<int>(), 42);
    EXPECT_NEAR(caps["float_val"].get<double>(), 3.14, 0.001);
    EXPECT_TRUE(caps["bool_val"].get<bool>());
    EXPECT_TRUE(caps["null_val"].is_null());
    EXPECT_EQ(caps["array_val"].size(), 3);
    EXPECT_EQ(caps["object_val"]["key"].get<std::string>(), "value");
}

// Edge cases
TEST_F(CapabilitiesNativeTest, SpecialCharactersInKeys) {
    envelope.capabilities = {
        {"key-with-dash", 1},
        {"key_with_underscore", 2},
        {"key.with.dot", 3},
        {"key:with:colon", 4}
    };

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto caps = deserialized.capabilities.value();
    EXPECT_EQ(caps["key-with-dash"].get<int>(), 1);
    EXPECT_EQ(caps["key_with_underscore"].get<int>(), 2);
    EXPECT_EQ(caps["key.with.dot"].get<int>(), 3);
    EXPECT_EQ(caps["key:with:colon"].get<int>(), 4);
}

TEST_F(CapabilitiesNativeTest, VeryLargeObject) {
    CapabilitiesMap large_caps;
    for (int i = 0; i < 100; i++) {
        large_caps["key_" + std::to_string(i)] = i;
    }
    envelope.capabilities = large_caps;

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto caps = deserialized.capabilities.value();
    EXPECT_EQ(caps.size(), 100);
    EXPECT_EQ(caps["key_50"].get<int>(), 50);
    EXPECT_EQ(caps["key_99"].get<int>(), 99);
}

TEST_F(CapabilitiesNativeTest, DeeplyNestedObject) {
    json deep = {
        {"level1", {
            {"level2", {
                {"level3", {
                    {"level4", {
                        {"level5", "deep_value"}
                    }}
                }}
            }}
        }}
    };
    envelope.capabilities = {{"deep", deep}};

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto caps = deserialized.capabilities.value();
    EXPECT_EQ(
        caps["deep"]["level1"]["level2"]["level3"]["level4"]["level5"].get<std::string>(),
        "deep_value"
    );
}

// Type checking
TEST_F(CapabilitiesNativeTest, TypePreservation) {
    envelope.capabilities = {
        {"int", 42},
        {"float", 3.14},
        {"bool", true},
        {"string", json("text")},
        {"array", json::array({1, 2})},
        {"object", json::object({{"a", 1}})}
    };

    auto serialized = envelope.serialize();
    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));

    auto caps = deserialized.capabilities.value();
    EXPECT_TRUE(caps["int"].is_number_integer());
    EXPECT_TRUE(caps["float"].is_number_float());
    EXPECT_TRUE(caps["bool"].is_boolean());
    EXPECT_TRUE(caps["string"].is_string());
    EXPECT_TRUE(caps["array"].is_array());
    EXPECT_TRUE(caps["object"].is_object());
}

// Error handling
TEST_F(CapabilitiesNativeTest, InvalidJSONDeserialization) {
    std::string invalid_json = R"({"from":"test","to":"test","operation":1,"capabilities":{invalid}})";

    Envelope env;
    EXPECT_FALSE(env.deserialize(invalid_json));
}

TEST_F(CapabilitiesNativeTest, MissingCapabilitiesField) {
    envelope.capabilities = std::nullopt;

    auto serialized = envelope.serialize();
    EXPECT_TRUE(serialized.find("capabilities") == std::string::npos);

    Envelope deserialized;
    ASSERT_TRUE(deserialized.deserialize(serialized));
    EXPECT_FALSE(deserialized.capabilities.has_value());
}

// Builder pattern tests
TEST_F(CapabilitiesNativeTest, BuilderWithNativeTypes) {
    auto env = EnvelopeBuilder()
        .from("sender")
        .to("receiver")
        .operation(OperationType::DATA)
        .capability("count", 100)
        .capability("enabled", true)
        .capability("ratio", 0.75)
        .build();

    ASSERT_TRUE(env.capabilities.has_value());
    auto caps = env.capabilities.value();
    EXPECT_EQ(caps["count"].get<int>(), 100);
    EXPECT_TRUE(caps["enabled"].get<bool>());
    EXPECT_NEAR(caps["ratio"].get<double>(), 0.75, 0.001);
}

TEST_F(CapabilitiesNativeTest, BuilderCapabilityStringHelper) {
    auto env = EnvelopeBuilder()
        .from("sender")
        .to("receiver")
        .capability_str("model", "gpt-4")
        .build();

    ASSERT_TRUE(env.capabilities.has_value());
    EXPECT_EQ(env.capabilities.value()["model"].get<std::string>(), "gpt-4");
}

TEST_F(CapabilitiesNativeTest, BuilderCapabilityIntHelper) {
    auto env = EnvelopeBuilder()
        .from("sender")
        .to("receiver")
        .capability_int("tokens", 1000)
        .build();

    ASSERT_TRUE(env.capabilities.has_value());
    EXPECT_EQ(env.capabilities.value()["tokens"].get<int>(), 1000);
}

TEST_F(CapabilitiesNativeTest, BuilderCapabilityBoolHelper) {
    auto env = EnvelopeBuilder()
        .from("sender")
        .to("receiver")
        .capability_bool("streaming", true)
        .build();

    ASSERT_TRUE(env.capabilities.has_value());
    EXPECT_TRUE(env.capabilities.value()["streaming"].get<bool>());
}

TEST_F(CapabilitiesNativeTest, BuilderCapabilityDoubleHelper) {
    auto env = EnvelopeBuilder()
        .from("sender")
        .to("receiver")
        .capability_double("temperature", 0.8)
        .build();

    ASSERT_TRUE(env.capabilities.has_value());
    EXPECT_NEAR(env.capabilities.value()["temperature"].get<double>(), 0.8, 0.001);
}

// Backward compatibility
TEST_F(CapabilitiesNativeTest, BackwardCompatibilityWithStringValues) {
    // Old format: all values as strings
    std::string old_format = R"({
        "from": "sender",
        "to": "receiver",
        "operation": 1,
        "capabilities": {
            "model": "gpt-4",
            "count": "100"
        }
    })";

    Envelope env;
    ASSERT_TRUE(env.deserialize(old_format));

    ASSERT_TRUE(env.capabilities.has_value());
    auto caps = env.capabilities.value();
    EXPECT_EQ(caps["model"].get<std::string>(), "gpt-4");
    // String "100" is preserved as string if that's what was in JSON
    EXPECT_EQ(caps["count"].get<std::string>(), "100");
}

